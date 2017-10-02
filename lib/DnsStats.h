#ifndef DNSSTAT_H
#define DNSSTAT_H

#include <stdint.h>
#include <stdio.h>
#include "DnsStatHash.h"

/*
 * List of registry definitions 
 */

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
#define	REGISTRY_TLD_error_class 25
#define	REGISTRY_DNS_txt_underline 26

#define DNS_REGISTRY_ERROR_RRTYPE (1<<0)
#define DNS_REGISTRY_ERROR_RRCLASS (1<<1)
#define DNS_REGISTRY_ERROR_OPCODE (1<<2)
#define DNS_REGISTRY_ERROR_RCODE (1<<3)
#define DNS_REGISTRY_ERROR_KALGO (1<<4)
#define DNS_REGISTRY_ERROR_OPTO (1<<5)
#define DNS_REGISTRY_ERROR_TLD (1<<6)
#define DNS_REGISTRY_ERROR_LABEL (1<<7)
#define DNS_REGISTRY_ERROR_FORMAT (1<<8)

class DnsStats
{
public:
    DnsStats();
    ~DnsStats();

    DnsStatHash hashTable;

    void SubmitPacket(uint8_t * packet, uint32_t length);

    bool ExportToCsv(char* fileName);

    int record_count; 
    int query_count;
    int response_count;
    uint32_t error_flags;

private:
    int SubmitQuery(uint8_t * packet, uint32_t length, uint32_t start, bool is_response);
    int SubmitRecord(uint8_t * packet, uint32_t length, uint32_t start, 
        uint32_t * e_rcode, uint32_t * e_length, bool is_response);
    int SubmitName(uint8_t * packet, uint32_t length, uint32_t start, uint32_t registryId);

    void SubmitOPTRecord(uint32_t flags, uint8_t * content, uint32_t length, uint32_t * e_rcode);
    void SubmitKeyRecord(uint8_t * content, uint32_t length);
    void SubmitRRSIGRecord(uint8_t * content, uint32_t length);
    void SubmitDSRecord(uint8_t * content, uint32_t length);

    void SubmitRegistryNumber(uint32_t registry_id, uint32_t number);
    void SubmitRegistryString(uint32_t registry_id, uint32_t length, uint8_t * value);

    void PrintRRType(FILE* F, uint32_t rrtype);
    void PrintRRClass(FILE* F, uint32_t rrclass);
    void PrintOpCode(FILE* F, uint32_t opcode);
    void PrintRCode(FILE* F, uint32_t rcode);
    void PrintDnsFlags(FILE* F, uint32_t flag);
    void PrintEDnsFlags(FILE* F, uint32_t flag);
    void PrintKeyAlgorithm(FILE* F, uint32_t algo);
    void PrintOptOption(FILE* F, uint32_t option);
    void PrintErrorFlags(FILE* F, uint32_t flags);
    void PrintTldErrorClass(FILE* F, uint32_t tld_error_class);

    void CheckRRType(uint32_t rrtype);
    void CheckRRClass(uint32_t rrclass);
    void CheckOpCode(uint32_t opcode);
    void CheckRCode(uint32_t rcode);
    void CheckKeyAlgorithm(uint32_t algo);
    void CheckOptOption(uint32_t option);
    bool CheckTld(uint32_t length, uint8_t * lower_case_tld);
    int CheckForUnderline(uint8_t * packet, uint32_t length, uint32_t start);

    void NormalizeNamePart(uint32_t length, uint8_t * value, uint8_t * normalized, uint32_t * flags);
};

#endif /* DNSTAT_H */
