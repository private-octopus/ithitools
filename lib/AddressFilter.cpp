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

IPAsKey::IPAsKey(uint8_t * addr, size_t addr_len)
    :
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
    if (hash != 0)
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

void AddressFilter::AddToList(uint8_t * addr, size_t len)
{
    IPAsKey * x = new IPAsKey(addr, len);

    if (x != NULL)
    {
        if (!table.InsertOrAdd(x, false))
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
