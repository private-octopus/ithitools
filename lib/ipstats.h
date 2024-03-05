/*
* Author: Christian Huitema
* Copyright (c) 2023, Private Octopus, Inc.
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

#ifndef IPSTAT_H
#define IPSTAT_H

#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <HashBinGeneric.h>
#include <TldAsKey.h>
#include <DnsStats.h>
#include <cdns.h>


/*
* The class IPStats manages the extraction of per address records,
* as a first step towards the study of DNS resolvers. The goal is
* to transform the PCAP or CBOR file into a much smaller CSV file
* containing one entry per IP address and per hour.
* 
* We are interested into queries only. We want to characterize
* resolvers, and thus the following:
* 
* - Source IP Address
* - Hour at which the qeury was issued, so we can aggregate per hour
* - Protocol (UDP or TCP)
* - TLD
* - SLD
* - more data, as we think of it...
* 
* Should we compact the TLD and SLD?
* Option1: char*. Pointer is 8 bytes, char field is 2 to 6 bytes + null, more or less 16 bytes. One malloc/field
* Option2: pointer to hash table entry, 8 bytes. Rare malloc.
* Option3: index, 4 bytes, plus shared value in list. 
* Option3 requires:
* 1) Indexed list (retrieve by index)
* 2) Hash table: key by name, provides index.
 */

class HyperLogLog
{
public:
    HyperLogLog();
    ~HyperLogLog();

    void AddLogs(const HyperLogLog* hll);
    static uint64_t Fnv64(const uint8_t* x, size_t l);
    static int BucketID(uint64_t fnv64);
    static int TrailingZeroes(uint64_t fnv64);
    void AddKey(const uint8_t* x, size_t l);
    size_t Serialize(uint8_t* buffer, size_t buffer_size);
    size_t Deserialize(const uint8_t* buffer, size_t buffer_size);
    double Assess();
    bool WriteHyperLogLog(FILE* F);
    void ParseHyperLogLog(char const* line, size_t* index);

    uint8_t hllv[16];
};

extern const uint32_t RegisteredTldNameNb;
extern char const* RegisteredTldName[];

class IPStats;

class IPStatsRecord
{
public:
    IPStatsRecord();
    ~IPStatsRecord();


    size_t ipaddr_length;
    uint8_t ip_addr[16];
    uint64_t query_volume;
    uint64_t hourly_volume[24];
    uint64_t daily_volume[31];
    uint64_t arpa_count;
    uint64_t no_such_domain_queries;
    uint64_t no_such_domain_reserved;
    uint64_t no_such_domain_frequent;
    uint64_t no_such_domain_chromioids;
    uint64_t tld_counts[8];
    HyperLogLog tld_hyperlog;
    /* TODO: do we really need the SLD count? If this is for "good" records,
     * we have no statistical basis for expecting specific values */
    uint64_t sld_counts[8];
    HyperLogLog sld_hyperlog;
    uint64_t name_parts[8];
    uint64_t rr_types[8];
    /* TODO: how will we derive "locale" from the name in the file? */
    uint64_t locales[8];

    bool IsSameKey(IPStatsRecord* key);
    uint32_t Hash();
    IPStatsRecord* CreateCopy();
    void Add(IPStatsRecord* key);

    IPStatsRecord * HashNext;

    void add_vec(uint64_t* x, uint64_t* y, size_t l);

    bool Serialize(uint8_t buffer, size_t buffer_size, size_t * length);
    bool Deserialize(uint8_t buffer, size_t buffer_size, size_t * length);

    /* The following methods are used to populate a record describing a single query */
    bool SetIP(size_t ipaddr_length, uint8_t* ipaddr_v);
    bool SetTime(int64_t qr_time);
    bool SetQName(uint8_t* q_name, uint32_t q_name_length, IPStats * ip_stats);
    bool SetRR(int rr_type, int rr_class);

    bool WriteRecord(FILE* F);
    static IPStatsRecord* ParseLine(char const* line);
private:
    bool WriteIP(FILE* F);
    static void SetXLD(size_t xld_length, uint8_t * xld, const char ** XLD_subset, size_t nb_XLD_subset, uint64_t * xld_counts, HyperLogLog * xld_hyperlog);
    void SetTLD(size_t tld_length, uint8_t* tld);
    void SetSLD(size_t sld_length, uint8_t* sld);
};

class IPStats
{
public:
    IPStats();
    ~IPStats();

    /* For the command line tools */
    bool LoadCborFiles(size_t nb_files, char const** fileNames);
    bool LoadCborFile(char const* fileNames);

    bool SaveToCsv(char const* file_name);

    uint32_t GetCount();

#if 0
    bool LoadPcapFiles(size_t nb_files, char const ** fileNames);
    bool LoadPcapFile(char const * fileName);
#endif

    bool IsRegisteredTLD(uint8_t* x, size_t l);

private:
    BinHash<IPStatsRecord> ip_records;
    void SubmitCborPacket(cdns* cdns_ctx, size_t packet_id);
    static bool IPAddressIsLower(IPStatsRecord * x, IPStatsRecord * y);
    BinHash<TldAsKey> registeredTld;
    DnsStats dnsstats;
};

#endif /* IPSTATS_H */