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
#include "dnscap_common.h"
#include "cdns.h"

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(x) (void)(x)
#endif

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
#define REGISTRY_DNSSEC_Client_Usage 41
#define REGISTRY_DNSSEC_Zone_Usage 42
#define REGISTRY_EDNS_Client_Usage 43
#define REGISTRY_QNAME_MINIMIZATION_Usage 44
#define REGISTRY_EDNS_OPT_USAGE 45
#define REGISTRY_EDNS_OPT_USAGE_REF 46
#define REGISTRY_VOLUME_PER_PROTO 47
#define REGISTRY_TCPSYN_PER_PROTO 48
#define REGISTRY_CAPTURE_DURATION 49
#define REGISTRY_VOLUME_53ONLY 50
#define REGISTRY_CAPTURE_DURATION53 51
#define REGISTRY_DNS_LEAK_BINARY 52
#define REGISTRY_DNS_LEAK_SYNTAX 53
#define REGISTRY_DNS_LEAK_IPV4 54
#define REGISTRY_DNS_LEAK_NUMERIC 55
#define REGISTRY_DNS_LEAK_2NDLEVEL 56
#define REGISTRY_DNS_ADDRESS_LIST 57
#define REGISTRY_DNS_ERRONEOUS_NAME_LIST 58
#define REGISTRY_DNS_TLD_MIN_DELAY_IP 59
#define REGISTRY_DNS_TLD_AVG_DELAY_IP 60
#define REGISTRY_DNS_TLD_MIN_DELAY_LOAD 61
#define REGISTRY_DNS_ADDRESS_DELAY 62

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
    dnsStateFlagListTldUsed = 32,
    dnsStateFlagReportResolverIPAddress = 64,
    dnsStateFlagListErroneousNames = 128,
    dnsStateFlagIncludeTcpRecords = 256
};


#ifdef PRIVACY_CONSCIOUS
enum DnsStatsLeakType
{
    dnsLeakNoLeak = 0,
    dnsLeakBinary,
    dnsLeakBadSyntax,
    dnsLeakNumeric,
    dnsLeakIpv4,
    dnsLeakRfc6771,
    dnsLeakFrequent,
    dnsLeakSinglePart,
    dnsLeakMultiPart,
    dnsLeakSinglePartDGA,
    dnsLeakMultiPartDGA
};
#endif

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
    uint64_t count;
    uint32_t key_type;
    uint32_t key_length;
    union {
        uint32_t key_number;
        uint8_t key_value[64];
    };
};

enum DnsPrefixClass {
    DnsPrefixStd = 0,
    DnsPrefixOneLevel,
    DnsPrefixException
};

class DnsPrefixEntry {
public:
    DnsPrefixEntry();
    ~DnsPrefixEntry();

    bool IsSameKey(DnsPrefixEntry* key);
    uint32_t Hash();
    DnsPrefixEntry* CreateCopy();
    void Add(DnsPrefixEntry* key);

    DnsPrefixEntry * HashNext;

    uint32_t hash;
    char * dnsPrefix;
    DnsPrefixClass dnsPrefixClass;
};

class DnssecPrefixEntry {
public:
    DnssecPrefixEntry();
    ~DnssecPrefixEntry();

    bool IsSameKey(DnssecPrefixEntry* key);
    uint32_t Hash();
    DnssecPrefixEntry* CreateCopy();
    void Add(DnssecPrefixEntry* key);

    DnssecPrefixEntry * HashNext;

    uint32_t hash;
    uint8_t * prefix;
    size_t prefix_len;
    bool is_dnssec;
private:
    uint8_t * prefix_data;
};

class DomainEntry {
public:
    DomainEntry();
    ~DomainEntry();

    bool IsSameKey(DomainEntry* key);
    uint32_t Hash();
    DomainEntry* CreateCopy();
    void Add(DomainEntry* key);

    DomainEntry * HashNext;

    uint32_t hash;
    uint32_t domain_length;
    char * domain;
    uint64_t count;
};

class TldAddressAsKey
{
public:
    TldAddressAsKey(uint8_t * addr, size_t addr_len, uint8_t * tld, size_t tld_len, my_bpftimeval ts);
    ~TldAddressAsKey();

    bool IsSameKey(TldAddressAsKey* key);
    uint32_t Hash();
    TldAddressAsKey* CreateCopy();
    void Add(TldAddressAsKey* key);

    TldAddressAsKey * HashNext;

    static bool CompareByAddressAndTld(TldAddressAsKey * x, TldAddressAsKey * y);

    size_t addr_len;
    uint8_t addr[16];
    size_t tld_len;
    uint8_t tld[65];
    uint32_t count;
    uint32_t hash;
    my_bpftimeval ts;
    my_bpftimeval ts_init;
    int64_t tld_min_delay;
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
    LruHash<TldAsKey> secondLdLeakage;
    BinHash<TldAddressAsKey> queryUsage;

    BinHash<TldAsKey> registeredTld;
    LruHash<TldAsKey> tldStringUsage;

    BinHash<DnsPrefixEntry> dnsPrefixTable;
    BinHash<DnssecPrefixEntry> dnssecPrefixTable;
    BinHash<StatsByIP> statsByIp;

    /* For the plug in */
    void SubmitPacket(uint8_t * packet, uint32_t length,
        uint8_t * source_addr, size_t source_addr_length,
        uint8_t * dest_addr, size_t dest_addr_length,
        my_bpftimeval ts);

    /* For the command line tools */
    bool LoadPcapFiles(size_t nb_files, char const ** fileNames);
    bool LoadCborFiles(size_t nb_files, char const** fileNames);
    bool LoadCborFile(char const* fileNames);
    bool ExportToCaptureSummary(CaptureSummary * cs);

    bool IsCaptureStopped() { return is_capture_stopped; };
    void StopCapture() { is_capture_stopped = true; }
    
    bool is_capture_dns_only;
    bool is_capture_stopped;
    uint32_t t_start_sec;
    uint32_t t_start_usec;
    int64_t duration_usec;
    uint64_t volume_53only;
    bool enable_frequent_address_filtering;
    uint32_t target_number_dns_packets;
    uint32_t frequent_address_max_count;
    uint32_t max_tld_leakage_count; 
    uint32_t max_tld_leakage_table_count;
    uint32_t max_query_usage_count;
    uint32_t max_tld_string_usage_count;
    uint32_t max_tld_string_leakage_count;
    uint32_t max_stats_by_ip_count;
    uint32_t dnsstat_flags;
    int record_count; 
    int query_count;
    int response_count;
    uint32_t error_flags;
    uint32_t dnssec_name_index;
    uint8_t * dnssec_packet;
    uint32_t dnssec_packet_length;
    bool is_do_flag_set;
    bool is_using_edns;
    uint8_t * edns_options;
    uint32_t edns_options_length;
    bool is_qname_minimized;

    static bool IsValidTldSyntax(uint8_t * tld, size_t length);
    static bool IsInSortedList(const char ** list, size_t nb_list, uint8_t * tld, size_t length);
    static bool IsRfc6761Tld(uint8_t * tld, size_t length);
    static bool IsFrequentLeakTld(uint8_t * tld, size_t length);
    static bool IsProbablyDgaTld(uint8_t * tld, size_t length);
    static void SetToUpperCase(uint8_t * domain, size_t length);
    static void TldCheck(uint8_t * domain, size_t length, bool * is_binary, bool * is_wrong_syntax, bool * is_numeric);

    static char const * GetTableName(uint32_t tableId);
    const char * GetZonePrefix(const char * dnsName);

    void RegisterDnssecUsageByName(uint8_t * packet, uint32_t length, uint32_t name_start,
        bool is_dnssec);
    void ExportDnssecUsage();

    void RegisterStatsByIp(uint8_t * dest_addr, size_t dest_addr_length);
    void RegisterOptionsByIp(uint8_t * source_addr, size_t source_addr_length);

    void RegisterTcpSynByIp(uint8_t * source_addr, size_t source_addr_length, bool tcp_port_583, bool tcp_port_443);

    void ExportStatsByIp();

    static size_t NormalizeNamePart(uint32_t length, uint8_t * value, uint8_t * normalized, size_t normalized_max, uint32_t * flags);

    static int GetDnsName(uint8_t * packet, uint32_t length, uint32_t start,
        uint8_t * name, size_t name_max, size_t * name_length);

    static int CompareDnsName(const uint8_t * packet, uint32_t length, uint32_t start1, uint32_t start2);
    static int Compare2DnsNames(const uint8_t* packet1, uint32_t length1, uint32_t start1, 
        const uint8_t* packet2, uint32_t length2, uint32_t start2);

    static bool IsIpv4Name(const uint8_t * name, size_t name_length);
    static bool IsIpv4Tld(uint8_t * packet, uint32_t length, uint32_t start);

    static bool IsQNameMinimized(uint32_t nb_queries, int q_rclass, int q_rtype,
        uint8_t* packet1, uint32_t length1, uint32_t qr_name_offset,
        uint8_t* packet2, uint32_t length2, uint32_t rr_name_offset);
    static void GetSourceAddress(int ip_type, uint8_t * ip_header, uint8_t ** addr, size_t * addr_length);
    static void GetDestAddress(int ip_type, uint8_t * ip_header, uint8_t ** addr, size_t * addr_length);

    void SubmitPacket(uint8_t * packet, uint32_t length, int ip_type, uint8_t* ip_header,
        my_bpftimeval ts);

    void UpdateDuration(my_bpftimeval ts);

    void SubmitCborPacket(cdns* cdns_ctx, size_t packet_id);
    void SubmitCborPacketQuery(cdns* cdns_ctx, cdns_query* query, cdns_query_signature* q_sig);
    void SubmitCborPacketResponse(cdns* cdns_ctx, cdns_query* query, cdns_query_signature* r_sig);

    /* Parallel construction for parsing CBOR and PCAP records. */
    void SubmitCborRecords(cdns* cdns_ctx, cdns_query* query, cdns_query_signature* q_sig,
        cdns_qr_extended* ext, bool is_response);
    void SubmitPcapRecords(uint8_t* packet, uint32_t length, uint32_t parse_index,
        bool is_response, bool has_header, uint32_t rcode, uint32_t flags,
        uint32_t qdcount, uint32_t ancount, uint32_t nscount, uint32_t arcount);

    static bool GetTLD(uint8_t * packet, uint32_t length, uint32_t start, uint32_t *offset, uint32_t * previous_offset, int * nb_name_parts);

    static int64_t DeltaUsec(long tv_sec, long tv_usec, long tv_sec_start, long tv_usec_start);
private:
    bool LoadPcapFile(char const * fileName);

    int SubmitQuery(uint8_t * packet, uint32_t length, uint32_t start, bool is_response, int * qclass, int * qtype);
    void SubmitQueryContent(int rrtype, int rrclass,
        uint8_t* packet, uint32_t packet_length, uint32_t name_offset);
    void SubmitQueryExtensions(
        uint8_t* packet, uint32_t length, uint32_t name_offset,
        uint8_t* client_addr, size_t client_addr_length);

    int SubmitRecord(uint8_t * packet, uint32_t length, uint32_t start, 
        uint32_t * e_rcode, uint32_t * e_length, bool is_response);
    void SubmitRecordContent(int rrtype, int rrclass, int ttl, int ldata,
        uint8_t* data, uint8_t* packet, uint32_t packet_length, uint32_t name_offset,
        uint32_t* e_rcode, uint32_t* e_length, bool is_response);

    void SubmitOpcodeAndFlags(uint32_t opcode, uint32_t flags);

    int SubmitName(uint8_t * packet, uint32_t length, uint32_t start, bool should_tabulate);

    void SubmitOPTRecord(uint32_t flags, uint8_t * content, uint32_t length, uint32_t * e_rcode);
    void RegisterEdnsUsage(uint32_t flags, uint32_t* e_rcode);
    void SubmitKeyRecord(uint8_t * content, uint32_t length);
    void SubmitRRSIGRecord(uint8_t * content, uint32_t length);
    void SubmitDSRecord(uint8_t * content, uint32_t length);
    void SubmitTLSARecord(uint8_t * content, uint32_t length);

    void SubmitRegistryNumberAndCount(uint32_t registry_id, uint32_t number, uint64_t count);
    void SubmitRegistryNumber(uint32_t registry_id, uint32_t number);
    void SubmitRegistryStringAndCount(uint32_t registry_id, uint32_t length, uint8_t * value, uint64_t count);
    void SubmitRegistryString(uint32_t registry_id, uint32_t length, uint8_t * value);

    void NameLeaksAnalysis(
        uint8_t* server_addr,
        size_t server_addr_length,
        uint8_t* client_addr,
        size_t client_addr_length,
        int rcode,
        uint8_t* packet,
        uint32_t packet_length,
        uint32_t name_offset,
        my_bpftimeval ts,
        bool is_not_empty_response
    );



    int CheckForUnderline(uint8_t * packet, uint32_t length, uint32_t start);

    bool IsNumericDomain(uint8_t * tld, uint32_t length);

    void ExportDomains(LruHash<TldAsKey> * table, uint32_t registry_id, uint32_t max_leak_count);
    void ExportLeakedDomains();
    void ExportStringUsage();
    void ExportSecondLeaked();
    void ExportQueryUsage();

    void LoadRegisteredTLD_from_memory();

    bool CheckAddress(uint8_t* addr, size_t addr_len);

    void LoadPrefixTable_from_memory();

    void RegisterDnssecUsageByPrefix(
        BinHash<DnssecPrefixEntry> * dnssecTable,
        uint8_t * prefix, size_t prefix_length, bool is_dnssec);

    void ExportDnssecUsageByTable(BinHash<DnssecPrefixEntry> * dnssecTable, uint32_t registry_id);
};

#endif /* DNSTAT_H */
