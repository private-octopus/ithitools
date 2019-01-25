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

#ifndef ADDRESS_FILTER_H
#define ADDRESS_FILTER_H

#include <stdint.h>
#include "HashBinGeneric.h"

#if __cplusplus < 199711L
#ifndef override
#define override 
#endif
#endif

class IPAsKey
{
public:
    IPAsKey(uint8_t * addr, size_t addr_len);
    virtual ~IPAsKey();

    bool IsSameKey(IPAsKey* key);
    uint32_t Hash();
    virtual IPAsKey* CreateCopy();
    virtual void Add(IPAsKey* key);
    IPAsKey * HashNext;

    size_t addr_len;
    uint8_t addr[16];
    uint32_t count;
    uint32_t hash;
};

class AddressFilter
{
public:
    AddressFilter();
    ~AddressFilter();

    bool SetList(char const * fname);
    void SetList(char const ** addr_list, size_t nb_names);

    void AddToList(char const * addrText);

    void AddToList(uint8_t * addr, size_t len);

    bool IsInList(uint8_t * addr, size_t len);

    static bool AddressText(uint8_t * addr, size_t len, char * text, size_t text_max);

    uint32_t GetCount()
    {
        return table.GetCount();
    }

    BinHash<IPAsKey> table;
};

class IPAsKeyLRU : public IPAsKey
{
public:
    IPAsKeyLRU(uint8_t * addr, size_t addr_len);
    ~IPAsKeyLRU();

    IPAsKeyLRU* CreateCopy() override;

    IPAsKeyLRU * MoreRecentKey;
    IPAsKeyLRU * LessRecentKey;
    IPAsKeyLRU * HashNext;
};

class AddressUseTracker
{
public:
    AddressUseTracker();
    ~AddressUseTracker();

    uint32_t Check(uint8_t * addr, size_t len);

    uint32_t table_lru_max;
    LruHash<IPAsKeyLRU> table;
};

/*
 * Keeping statistics per IP address:
 *
 * - Count: total number of queries from that address.
 * - nb_do: total number of queries with DO option.
 * - nb_edns: total number of queries with EDNS option.
 * - nb_min_qname: total number of queries with qname minimization.
 * - query_seen: if queries were observed.
 * - option_mask: one bit per hash(option code) for options used.
 *
 * The entries are created when a response is encountered to that specific IP,
 * see: DnsStats::RegisterStatsByIp(). They may also be created when an
 * option is found in a query, in DnsStats::RegisterOptionsByIp(), to update the 
 * option mask.
 *
 * The total number of entries maxes out at 0x8000, by default set up.
 *
 * When tallying the results, we look at whether queries from the IP address
 * use the DO bit, EDNS, or QName minimization. The first 2 are easy: just
 * count whether there is at least one DO bit query or at least one EDNS query.
 * The QName minimization is only true if no "not minimized" query was found.
 * 
 *
 * 
 */

class StatsByIP
{
public:
    StatsByIP(uint8_t * addr, size_t addr_len, bool has_do, bool has_edns, 
        bool not_qname_mini);
    virtual ~StatsByIP();

    bool IsSameKey(StatsByIP* key);
    uint32_t Hash();
    virtual StatsByIP* CreateCopy() ;
    virtual void Add(StatsByIP* key);
    StatsByIP * HashNext;

    bool IsDoUsed();
    bool IsEdnsSupported();
    bool IsQnameMinimized();

    bool RegisterNewOption(uint16_t option_code);

    size_t addr_len;
    uint8_t addr[16];
    uint32_t count;
    uint32_t hash;
    uint32_t nb_do;
    uint32_t nb_edns;
    uint32_t nb_not_qname_mini;
    bool query_seen;
    bool response_seen;
    uint64_t option_mask;
};

#endif /* ADDRESS_FILTER_H */