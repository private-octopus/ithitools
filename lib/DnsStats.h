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

#ifndef DNSSTAT_H
#define DNSSTAT_H

#include <stdint.h>
#include <stdio.h>
#if 0
#include "DnsStatHash.h"
#endif
#include "AddressFilter.h"
#include "HashBinGeneric.h"
#include "CaptureSummary.h"
#include "TldAsKey.h"

/*
 * List of registry definitions 
 */
#define	REGISTRY_ITHITOOLS_VERSION	0
#define	REGISTRY_DNS_CLASSES	1
#define	REGISTRY_DNS_RRType	2
#define	REGISTRY_DNS_OpCodes	3
#define	REGISTRY_DNS_RCODES	4
#define	REGISTRY_DNS_AFSDB_RRSubtype	5
#define	REGISTRY_DNS_DHCID_RRIdType	6
#define	REGISTRY_DNS_LabelType	7
#define	REGISTRY_EDNS_OPT_CODE	8
#define	REGISTRY_DNS_Header_Flags	9
#define	REGISTRY_EDNS_Header_Flags	10
#define	REGISTRY_EDNS_Version_number	11
#define	REGISTRY_DNS_CSYNC_Flags	12
#define	REGISTRY_DNSSEC_Algorithm_Numbers	13
#define	REGISTRY_DNSSEC_KEY_Prime_Lengths	14
#define	REGISTRY_DNS_Q_CLASSES	15
#define	REGISTRY_DNS_Q_RRType	16
#define	REGISTRY_DNSSEC_KEY_Well_Known_Primes	17
#define	REGISTRY_EDNS_Packet_Size 18
#define	REGISTRY_DNS_Query_Size 19
#define	REGISTRY_DNS_Response_Size 20
#define	REGISTRY_DNS_TC_length 21
#define	REGISTRY_TLD_query 22
#define	REGISTRY_TLD_response 23
#define	REGISTRY_DNS_error_flag 24
// #define	REGISTRY_TLD_error_class 25
#define	REGISTRY_DNS_txt_underline 26
#define REGISTRY_DNS_root_QR 27
#define REGISTRY_DNS_LeakByLength 28
#define REGISTRY_DNS_LeakedTLD 29
#define REGISTRY_DNS_RFC6761TLD 30
#define REGISTRY_DNS_UsefulQueries 31
#define REGISTRY_DANE_CertUsage 32
#define REGISTRY_DANE_TlsaSelector 33
#define REGISTRY_DANE_TlsaMatchingType 34
#define REGISTRY_TOOL_FrequentAddress 35
#define REGISTRY_DNS_Tld_Usage 36
#define REGISTRY_DNS_RFC6761_Usage 37
#define REGISTRY_DNS_Frequent_TLD_Usage 38
#define REGISTRY_DNS_TLD_Usage_Count 39
#define REGISTRY_DNS_Local_TLD_Usage_Count 40


#define DNS_REGISTRY_ERROR_RRTYPE (1<<0)
#define DNS_REGISTRY_ERROR_RRCLASS (1<<1)
#define DNS_REGISTRY_ERROR_OPCODE (1<<2)
#define DNS_REGISTRY_ERROR_RCODE (1<<3)
#define DNS_REGISTRY_ERROR_KALGO (1<<4)
#define DNS_REGISTRY_ERROR_OPTO (1<<5)
#define DNS_REGISTRY_ERROR_TLD (1<<6)
#define DNS_REGISTRY_ERROR_LABEL (1<<7)
#define DNS_REGISTRY_ERROR_FORMAT (1<<8)

#define DNS_OPCODE_QUERY 0
#define DNS_RCODE_NOERROR 0
#define DNS_RCODE_NXDOMAIN 3


enum DnsStatsFlags
{
    dnsStateFlagFilterLargeUsers = 1,
    dnsStateFlagCountTld = 2,
    dnsStateFlagCountQueryParms = 4,
    dnsStateFlagCountUnderlinedNames = 8,
    dnsStateFlagCountPacketSizes = 16,
    dnsStateFlagListTldUsed = 32
};

class DnsHashEntry {
public:
    DnsHashEntry();
    ~DnsHashEntry();

    bool IsSameKey(DnsHashEntry* key);
    uint32_t Hash();
    DnsHashEntry* CreateCopy();
    void Add(DnsHashEntry* key);

    DnsHashEntry * HashNext;

    uint32_t hash;
    uint32_t registry_id;
    uint32_t count;
    uint32_t key_type;
    uint32_t key_length;
    union {
        uint32_t key_number;
        uint8_t key_value[64];
    };
};

class TldAddressAsKey
{
public:
    TldAddressAsKey(uint8_t * addr, size_t addr_len, uint8_t * tld, size_t tld_len);
    ~TldAddressAsKey();

    bool IsSameKey(TldAddressAsKey* key);
    uint32_t Hash();
    TldAddressAsKey* CreateCopy();
    void Add(TldAddressAsKey* key);

    TldAddressAsKey * HashNext; 

    size_t addr_len;
    uint8_t addr[16];
    size_t tld_len;
    uint8_t tld[65];
    uint32_t count;
    uint32_t hash;
};

class DnsStats
{
public:
    DnsStats();
    ~DnsStats();

    BinHash<DnsHashEntry> hashTable;
    AddressFilter rootAddresses;
    AddressFilter allowedAddresses;
    AddressFilter bannedAddresses;
    AddressUseTracker frequentAddresses;

    LruHash<TldAsKey> tldLeakage;
    BinHash<TldAddressAsKey> queryUsage;

    BinHash<TldAsKey> registeredTld;
    LruHash<TldAsKey> tldStringUsage;

    /* For the plug in */
    void SubmitPacket(uint8_t * packet, uint32_t length,
        uint8_t * source_addr, size_t source_addr_length,
        uint8_t * dest_addr, size_t dest_addr_length);

    /* For the command line tools */
    bool LoadPcapFiles(size_t nb_files, char const ** fileNames);
    bool ExportToCaptureSummary(CaptureSummary * cs);
    
    bool enable_frequent_address_filtering;
    uint32_t frequent_address_max_count;
    uint32_t max_tld_leakage_count; 
    uint32_t max_tld_leakage_table_count;
    uint32_t max_query_usage_count;
    uint32_t max_tld_string_usage_count;
    uint32_t dnsstat_flags;
    int record_count; 
    int query_count;
    int response_count;
    uint32_t error_flags;


    static bool IsRfc6761Tld(uint8_t * tld, size_t length);
    static void SetToUpperCase(uint8_t * domain, size_t length);
    static char const * GetTableName(uint32_t tableId);

private:
    bool LoadPcapFile(char const * fileName);
    void SubmitPacket(uint8_t * packet, uint32_t length, int ip_type, uint8_t* ip_header);

    int SubmitQuery(uint8_t * packet, uint32_t length, uint32_t start, bool is_response);
    int SubmitRecord(uint8_t * packet, uint32_t length, uint32_t start, 
        uint32_t * e_rcode, uint32_t * e_length, bool is_response);
    int SubmitName(uint8_t * packet, uint32_t length, uint32_t start, bool should_tabulate);

    void SubmitOPTRecord(uint32_t flags, uint8_t * content, uint32_t length, uint32_t * e_rcode);
    void SubmitKeyRecord(uint8_t * content, uint32_t length);
    void SubmitRRSIGRecord(uint8_t * content, uint32_t length);
    void SubmitDSRecord(uint8_t * content, uint32_t length);
    void SubmitTLSARecord(uint8_t * content, uint32_t length);

    void SubmitRegistryNumberAndCount(uint32_t registry_id, uint32_t number, uint32_t count);
    void SubmitRegistryNumber(uint32_t registry_id, uint32_t number);
    void SubmitRegistryStringAndCount(uint32_t registry_id, uint32_t length, uint8_t * value, uint32_t count);
    void SubmitRegistryString(uint32_t registry_id, uint32_t length, uint8_t * value);

    int CheckForUnderline(uint8_t * packet, uint32_t length, uint32_t start);

    int GetTLD(uint8_t * packet, uint32_t length, uint32_t start, uint32_t *offset);

    void NormalizeNamePart(uint32_t length, uint8_t * value, uint8_t * normalized, uint32_t * flags);

    void GetSourceAddress(int ip_type, uint8_t * ip_header, uint8_t ** addr, size_t * addr_length);
    void GetDestAddress(int ip_type, uint8_t * ip_header, uint8_t ** addr, size_t * addr_length);

    bool IsNumericDomain(uint8_t * tld, uint32_t length);

    void ExportDomains(LruHash<TldAsKey> * table, uint32_t registry_id, 
        bool do_accounting);
    void ExportLeakedDomains();
    void ExportStringUsage();

    void LoadRegisteredTLD_from_memory();

    bool CheckAddress(uint8_t* addr, size_t addr_len);
};

#endif /* DNSTAT_H */
