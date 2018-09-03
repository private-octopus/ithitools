/*
* Author: Christian Huitema
* Copyright (c) 2017, Private Octopus, Inc.
* All rights reserved.
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Private Octopus, Inc. BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AddressFilter.h"
#include <string.h>

#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <WinSock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>

#ifndef SOCKET_TYPE 
#define SOCKET_TYPE SOCKET
#endif
#ifndef SOCKET_CLOSE
#define SOCKET_CLOSE(x) closesocket(x)
#endif
#ifndef WSA_START_DATA
#define WSA_START_DATA WSADATA
#endif
#ifndef WSA_START
#define WSA_START(x, y) WSAStartup((x), (y))
#endif
#ifndef WSA_LAST_ERROR
#define WSA_LAST_ERROR(x)  WSAGetLastError()
#endif
#ifndef socklen_t
#define socklen_t int
#endif

#else  /* Linux */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/* #include <unistd.h> */
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef __USE_XOPEN2K
#define __USE_XOPEN2K
#endif
#ifndef __USE_POSIX
#define __USE_POSIX
#endif

#endif /* _WINDOWS or Linux*/

IPAsKey::IPAsKey(uint8_t * addr, size_t addr_len)
    :
    HashNext(NULL),
    count(1),
    hash(0)
{
    if (addr_len > 16)
    {
        addr_len = 16;
    }

    memcpy(this->addr, addr, addr_len);
    this->addr_len = addr_len;
}

IPAsKey::~IPAsKey()
{}

bool IPAsKey::IsSameKey(IPAsKey * key)
{
    bool ret = (key->addr_len == this->addr_len &&
        memcmp(key->addr, this->addr, this->addr_len) == 0);
    return ret;
}

uint32_t IPAsKey::Hash()
{
    if (hash == 0)
    {
        hash = 0xDEADBEEF;
        for (size_t i = 0; i < addr_len; i++)
        {
            hash = hash * 101 + addr[i];
        }
    }
    return hash;
}

IPAsKey * IPAsKey::CreateCopy()
{
    IPAsKey * x = new IPAsKey(addr, addr_len);

    if (x != NULL)
    {
        x->count = count;
        x->hash = hash;
    }

    return x;
}

void IPAsKey::Add(IPAsKey * key)
{
    count += key->count;
}

AddressFilter::AddressFilter()
{
}

AddressFilter::~AddressFilter()
{
}

bool AddressFilter::SetList(char const * fname)
{
    bool ret;
    FILE * F = NULL;
    char line[256];

#ifdef _WINDOWS
    errno_t err = fopen_s(&F, fname, "w");
    ret = (err == 0 && F != NULL);
#else
    F = fopen(fname, "r");
    ret = (F != NULL);
#endif

    while (ret && fgets(line, sizeof(line), F) != NULL)
    {
        if (line[0] != '#')
        {
            AddToList(line);
        }
    }


    if (F != NULL)
    {
        fclose(F);
    }

    return ret;
}

void AddressFilter::SetList(char const ** addr_list, size_t nb_names)
{
    for (size_t i = 0; i < nb_names; i++)
    {
        AddToList(addr_list[i]);
    }
}

void AddressFilter::AddToList(char const * addrText)
{
    uint8_t * addr_bin = NULL;
    size_t addr_len = 0;
    struct in_addr ipv4_addr;
    struct in6_addr ipv6_addr;

    if (inet_pton(AF_INET, addrText, &ipv4_addr) == 1)
    {
        /* Valid IPv4 address */
        addr_bin = (uint8_t *)&ipv4_addr;
        addr_len = sizeof(struct in_addr);
    }
    else  if (inet_pton(AF_INET6, addrText, &ipv6_addr) == 1)
    {
        /* Valid IPv6 address */
        addr_bin = (uint8_t *)&ipv6_addr;
        addr_len = sizeof(struct in6_addr);
    }

    if (addr_len > 0)
    {
        AddToList(addr_bin, addr_len);
    }
}

void AddressFilter::AddToList(uint8_t * addr, size_t len)
{
    IPAsKey * x = new IPAsKey(addr, len);

    if (x != NULL)
    {
        bool stored = false;

        (void)table.InsertOrAdd(x, false, &stored);

        if (!stored)
        {
            delete x;
        }
    }
}

bool AddressFilter::IsInList(uint8_t * addr, size_t len)
{
    IPAsKey key(addr, len);
    IPAsKey * ret = table.Retrieve(&key);

    return (ret != NULL);
}

bool AddressFilter::AddressText(uint8_t * addr, size_t len, char * text, size_t text_max)
{
    bool ret = true;

    if (len == 4)
    {
        if (inet_ntop(AF_INET, addr, text, text_max) == NULL)
        {
            ret = false;
        }
    }
    else if (len == 16)
    {
        if (inet_ntop(AF_INET6, addr, text, text_max) == NULL)
        {
            ret = false;
        }
    }
    else
    {
        ret = false;
    }

    return ret;
}

AddressUseTracker::AddressUseTracker()
    :
    table_lru_max(0x8000)
{
}

AddressUseTracker::~AddressUseTracker()
{
}

uint32_t AddressUseTracker::Check(uint8_t * addr, size_t len)
{
    IPAsKeyLRU * x = new IPAsKeyLRU(addr, len);
    uint32_t count = 1;

    while (table.GetCount() >= table_lru_max)
    {
        IPAsKeyLRU * removed = table.RemoveLRU();

        if (removed != NULL)
        {
            delete removed;
        }
    }

    if (x != NULL)
    {
        bool stored = false;

        IPAsKeyLRU * y = table.InsertOrAdd(x, false, &stored);

        if (!stored)
        {
            delete x;
        }

        if (y != NULL)
        {
            count = y->count;
        }
    }

    return count;
}

IPAsKeyLRU::IPAsKeyLRU(uint8_t * addr, size_t addr_len)
    :
    IPAsKey(addr, addr_len),
    MoreRecentKey(NULL),
    LessRecentKey(NULL)
{
}

IPAsKeyLRU::~IPAsKeyLRU()
{
}

IPAsKeyLRU * IPAsKeyLRU::CreateCopy()
{
    IPAsKeyLRU * x = new IPAsKeyLRU(addr, addr_len);

    if (x != NULL)
    {
        x->count = count;
        x->hash = hash;
    }

    return x;
}


StatsByIP::StatsByIP(uint8_t * addr, size_t addr_len, bool has_do, bool has_edns, bool mini_qname) :
    HashNext(NULL),
    count(1),
    hash(0),
    nb_do((has_do)?1:0),
    nb_edns((has_edns)?1:0),
    nb_mini_qname((mini_qname)?1:0),
    query_seen(false),
    response_seen(false),
    option_mask(0)
{
    if (addr_len > 16)
    {
        addr_len = 16;
    }

    memcpy(this->addr, addr, addr_len);
    this->addr_len = addr_len;
}

StatsByIP::~StatsByIP()
{
}

bool StatsByIP::IsSameKey(StatsByIP * key)
{
    bool ret = (key->addr_len == this->addr_len &&
        memcmp(key->addr, this->addr, this->addr_len) == 0);
    return ret;
}

uint32_t StatsByIP::Hash()
{
    if (hash == 0)
    {
        hash = 0xDEADBEEF;
        for (size_t i = 0; i < addr_len; i++)
        {
            hash = hash * 101 + addr[i];
        }
    }
    return hash;
}


StatsByIP * StatsByIP::CreateCopy()
{
    StatsByIP * x = new StatsByIP(addr, addr_len, false, false, false);

    if (x != NULL)
    {
        x->hash = hash;
        x->nb_do = nb_do;
        x->nb_edns = nb_edns;
        x->nb_mini_qname = nb_mini_qname;
        x->query_seen = query_seen;
        x->response_seen = response_seen;
        x->option_mask = option_mask;
    }

    return x;
}

void StatsByIP::Add(StatsByIP * key)
{
    count += key->count;
    nb_do += key->nb_do;
    nb_edns += key->nb_edns;
    nb_mini_qname += key->nb_mini_qname;
    query_seen |= key->query_seen;
    response_seen |= key->response_seen;
    option_mask |= key->option_mask;
}

bool StatsByIP::IsDoUsed()
{
    return (nb_do > 0);
}

bool StatsByIP::IsEdnsSupported()
{
    return (nb_edns > 0);
}

bool StatsByIP::IsQnameMinimized()
{
    uint32_t ref_count = count;

    if (query_seen) {
        ref_count--;
    }

    return(nb_mini_qname == ref_count);
}

/* Options should be counted at most once per resolver. To
 * verify that, we use a simple bit mask, much like a simplified
 * Bloome filter */

bool StatsByIP::RegisterNewOption(uint16_t option_code)
{
    uint32_t option_hash = option_code;

    /* Compute a minimal 16 bit hash */
    option_hash ^= (option_code * 102);
    option_hash ^= (option_hash >> 11);
    option_hash *= 31;
    option_hash ^= (option_hash >> 6);

    /* Retain only the last 6 bits */
    option_hash &= 0x3F;
    uint64_t bit_mask = (1ull << option_hash);

    /* check the mask */
    bool ret = (option_mask&bit_mask) == 0;

    if (ret) {
        option_mask |= bit_mask;
    }

    return ret;
}

