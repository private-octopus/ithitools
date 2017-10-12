#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include "DnsTypes.h"
#include "DnsStats.h"


DnsStats::DnsStats()
    :
    record_count(0),
    query_count(0),
    response_count(0)
{
}


DnsStats::~DnsStats()
{
}

static char const * RegistryNameById[] = {
    "0",
    "CLASS",
    "RR Type",
    "OpCode",
    "RCODE",
    "AFSDB RRSubtype",
    "DHCID RRIdType",
    "Label Type",
    "EDNS OPT CODE",
    "Header Flags",
    "EDNS Header_Flags",
    "EDNS Version number",
    "CSYNC Flags",
    "DNSSEC Algorithm Numbers",
    "DNSSEC KEY Prime Lengths",
    "Q-CLASS",
    "Q-RR Type",
    "DNSSEC Well Known Primes",
    "EDNS Packet Size",
    "Query Size",
    "Response Size",
    "TC Length",
    "Z-Q-TLD",
    "Z-R-TLD",
    "Z-Error Flags",
    "TLD Error Class",
    "Underlined part"
};

static uint32_t RegistryNameByIdNb = sizeof(RegistryNameById) / sizeof(char const*);

int DnsStats::SubmitQuery(uint8_t * packet, uint32_t length, uint32_t start, bool is_response)
{
    int rrclass = 0;
    int rrtype = 0;
    uint32_t name_start = start;

    start = SubmitName(packet, length, start,
        (!is_response && query_count < 10000)?REGISTRY_TLD_query:0);

    if (start + 4 <= length)
    {
        rrtype = (packet[start] << 8) | packet[start + 1];
        rrclass = (packet[start + 2] << 8) | packet[start + 3];
        start += 4;
        CheckRRClass(rrclass);
        CheckRRType(rrtype);
        SubmitRegistryNumber(REGISTRY_DNS_Q_CLASSES, rrclass);
        SubmitRegistryNumber(REGISTRY_DNS_Q_RRType, rrtype);

        if (rrtype == DnsRtype_TXT)
        {
            SubmitRegistryString(REGISTRY_DNS_txt_underline, 3, (uint8_t *) "TXT");
            CheckForUnderline(packet, length, name_start);
        }
    }
    else
    {
        error_flags |= DNS_REGISTRY_ERROR_FORMAT;
        start = length;
    }

    return start;
}

int DnsStats::SubmitRecord(uint8_t * packet, uint32_t length, uint32_t start, 
    uint32_t * e_rcode, uint32_t * e_length, bool is_response)
{
    int rrtype = 0;
    int rrclass = 0;
    unsigned int ttl = 0;
    int ldata = 0;
    int name_start = start;

    record_count++;

    start = SubmitName(packet, length, start, 0);

    if ((start + 10) > length)
    {
        error_flags |= DNS_REGISTRY_ERROR_FORMAT;
        start = length;
    }
    else
    {
        rrtype = (packet[start] << 8) | packet[start + 1];
        rrclass = (packet[start + 2] << 8) | packet[start + 3];
        ttl = (packet[start + 4] << 24) | (packet[start + 5] << 16)
            | (packet[start + 6] << 8) | packet[start + 7];
        ldata = (packet[start + 8] << 8) | packet[start + 9];

        if (start + ldata + 10 > length)
        {
            error_flags |= DNS_REGISTRY_ERROR_FORMAT;
            start = length;
        }
        else
        {
            if (ldata > 0 || rrtype == DnsRtype_OPT)
            {
                /* only record rrtypes and rrclass if valid response */
                if (rrtype != DnsRtype_OPT)
                {
                    CheckRRClass(rrclass);
                    CheckRRType(rrtype);

                    if (is_response)
                    {
                        SubmitRegistryNumber(REGISTRY_DNS_CLASSES, rrclass);
                        (void)SubmitName(packet, length, name_start, REGISTRY_TLD_response);
                    }
                    else
                    {
                        SubmitRegistryNumber(REGISTRY_DNS_Q_CLASSES, rrclass);
                    }
                }
                else
                {
                    /* document the extended length */
                    if (e_length != NULL)
                    {
                        *e_length = rrclass;
                    }
                }

                if (is_response)
                {
                    SubmitRegistryNumber(REGISTRY_DNS_RRType, rrtype);
                }
                else
                {
                    SubmitRegistryNumber(REGISTRY_DNS_Q_RRType, rrtype);
                }

                /* Further parsing for OPT, DNSKEY, RRSIG, DS,
                 * and maybe also AFSDB, NSEC3, DHCID, RSYNC types */
                switch (rrtype)
                {
                case (int)DnsRtype_OPT:
                    SubmitOPTRecord(ttl, &packet[start + 10], ldata, e_rcode);

                    break;
                case (int)DnsRtype_DNSKEY:
                    SubmitKeyRecord(&packet[start + 10], ldata);
                    break;
                case (int)DnsRtype_RRSIG:
                    SubmitRRSIGRecord(&packet[start + 10], ldata);
                    break;
                case (int)DnsRtype_DS:
                    SubmitDSRecord(&packet[start + 10], ldata);
                    break;
                default:
                    break;
                }
            }

            start += ldata + 10;
        }
    }

    return start;
}

static char const * common_bad_tld[] = {
    "local",
    "home",
    "ip",
    "localdomain",
    "dhcp",
    "localhost",
    "localnet",
    "lan",
    "telus",
    "internal",
    "belkin",
    "invalid",
    "workgroup",
    "domain",
    "corp",
    "comg",
    "homestation",
    "backnet",
    "router",
    "gateway",
    "nashr",
    "pk5001z",
    "pvt",
    "rbl",
    "toj",
    "_tcp",
    "actdsltmp",
    "dlinkrouter",
    "guest",
    "intra",
    "intranet",
    "reviewimages",
    "totolink",
    "airdream",
    "dlink",
    "station",
    "tendaap",
    "_nfsv4idmapdomain",
    "arris",
    "be",
    "blinkap",
    "enterprise",
    "mickeymouse",
    "netg",
    "openstacklocal",
    "private",
    "realtek",
    "site",
    "wimax",
    "domainname",
    "server",
    "yfserver",
    "oops",
    "fco",
    "public"
};

const uint32_t nb_common_bad_tld = sizeof(common_bad_tld) / sizeof(char const *);

int DnsStats::SubmitName(uint8_t * packet, uint32_t length, uint32_t start, uint32_t registryId)
{
    uint32_t l = 0;
    uint32_t offset = 0;
    uint32_t previous = 0;
    uint32_t name_start = start;
    uint8_t lower_case_tld[64];

    while (start < length)
    {
        l = packet[start];

        if (l == 0)
        {
            /* end of parsing*/

            SubmitRegistryNumber(REGISTRY_DNS_LabelType, 0);

            if (previous != 0)
            {
                uint32_t l_tld = packet[previous];
                uint32_t tld_flags = 0;

                if (l_tld > 0 && l_tld <= 63)
                {
                    NormalizeNamePart(l_tld, &packet[previous + 1], lower_case_tld, &tld_flags);

                    if (!CheckTld(l_tld, lower_case_tld))
                    {
                        /* Analyze of non expected TLD */
                        uint32_t tld_type = 0;
                        bool found = false;

                        for (uint32_t i = 0; i < nb_common_bad_tld; i++)
                        {
                            if (strcmp((const char *)lower_case_tld, common_bad_tld[i]) == 0)
                            {
                                found = true;
                                tld_type = i;
                                break;
                            }
                        }
                        if (!found)
                        {
                            if ((tld_flags&1) != 0)
                            {
                                tld_type = 62;
                            }
                            else if ((tld_flags & 2) != 0)
                            {
                                tld_type = 63;
                            }
                            else
                            {
                                tld_type = l_tld + tld_flags;
                            }
                        }
                        SubmitRegistryNumber(REGISTRY_TLD_error_class, tld_type);
                    }

                    if (registryId != 0)
                    {
                        SubmitRegistryString(registryId, l_tld, lower_case_tld);
                    }
                }
            }
            start++;
            break;
        }
        else if ((l & 0xC0) == 0xC0)
        {
            /* Name compression */
            SubmitRegistryNumber(REGISTRY_DNS_LabelType, 0xC0);

            if ((start + 2) > length)
            {
                error_flags |= DNS_REGISTRY_ERROR_FORMAT;
                start = length;
                break;
            }
            else
            {
                if (registryId != 0)
                {
                    uint32_t new_start = ((l & 63)<<8) + packet[start+1];

                    if (new_start < name_start)
                    {
                        (void) SubmitName(packet, length, new_start, registryId);
                    }
                }

                start += 2;
                break;
            }
        }
        else if (l > 0x3F)
        {
            /* found an extension. Don't know how to parse it! */
            error_flags |= DNS_REGISTRY_ERROR_LABEL;
            SubmitRegistryNumber(REGISTRY_DNS_LabelType, l);
            start = length;
            break;
        }
        else
        {
            /* regular name part. To do: tracking of underscore labels. */
            SubmitRegistryNumber(REGISTRY_DNS_LabelType, 0);
            if (start + l + 1 > length)
            {
                error_flags |= DNS_REGISTRY_ERROR_FORMAT;
                start = length;
                break;
            }
            else
            {
                previous = start;
                start += l + 1;
            }
        }
    }

    return start;
}

void DnsStats::SubmitOPTRecord(uint32_t flags, uint8_t * content, uint32_t length, uint32_t * e_rcode)
{
    uint32_t current_index = 0;

    /* Process the flags and rcodes */
    if (e_rcode != NULL)
    {
        *e_rcode = (flags >> 24) & 0xFF;
    }

    for (int i = 0; i < 16; i++)
    {
        if ((flags & (1 << i)) != 0)
        {
            SubmitRegistryNumber(REGISTRY_EDNS_Header_Flags, i);
        }
    }

    SubmitRegistryNumber(REGISTRY_EDNS_Version_number, (flags >> 16) & 0xFF);

    /* Find the options in the payload */
    while (current_index + 4 <= length)
    {
        uint32_t o_code = (content[current_index] << 8) | content[current_index + 1];
        uint32_t o_length = (content[current_index+2] << 8) | content[current_index + 3];
        current_index += 4 + o_length;
        CheckOptOption(o_code);
        SubmitRegistryNumber(REGISTRY_EDNS_OPT_CODE, o_code);
    }
}

void DnsStats::SubmitKeyRecord(uint8_t * content, uint32_t length)
{
    if (length > 8)
    {
        uint32_t algorithm = content[3];
        CheckKeyAlgorithm(algorithm);
        SubmitRegistryNumber(REGISTRY_DNSSEC_Algorithm_Numbers, algorithm);

        if (algorithm == 2)
        {
            uint32_t prime_length = (content[4] << 8) | content[5];
            if (prime_length < 16)
            {
                SubmitRegistryNumber(REGISTRY_DNSSEC_KEY_Prime_Lengths, prime_length);

                if (prime_length == 1 || prime_length == 2)
                {
                    uint32_t well_known_prime = (content[6] << 8) | content[7];
                    SubmitRegistryNumber(REGISTRY_DNSSEC_KEY_Well_Known_Primes, well_known_prime);
                }
            }
        }
    }
}

void DnsStats::SubmitRRSIGRecord(uint8_t * content, uint32_t length)
{
    if (length > 18)
    {
        uint32_t algorithm = content[2];
        CheckKeyAlgorithm(algorithm);
        SubmitRegistryNumber(REGISTRY_DNSSEC_Algorithm_Numbers, algorithm);
    }
}

void DnsStats::SubmitDSRecord(uint8_t * content, uint32_t length)
{
    if (length > 4)
    {
        uint32_t algorithm = content[2];
        CheckKeyAlgorithm(algorithm);
        SubmitRegistryNumber(REGISTRY_DNSSEC_Algorithm_Numbers, algorithm);
    }
}

void DnsStats::SubmitRegistryNumber(uint32_t registry_id, uint32_t number)
{
    dns_registry_entry_t key;

    key.count = 1;
    key.registry_id = registry_id;
    key.key_length = sizeof(uint32_t);
    key.key_type = 0; /* number */
    key.key_number = number;

    (void)hashTable.InsertOrAdd(&key);
}

void DnsStats::SubmitRegistryString(uint32_t registry_id, uint32_t length, uint8_t * value)
{
    dns_registry_entry_t key;

    if (length < 64)
    {
        key.count = 1;
        key.registry_id = registry_id;
        key.key_length = length;
        key.key_type = 1; /* string */
        memcpy(key.key_value, value, length);
        key.key_value[length] = 0;

        (void)hashTable.InsertOrAdd(&key);
    }
}

static char const *  rrtype_1_62[] = {
    "A",
    "NS",
    "MD",
    "MF",
    "CNAME",
    "SOA",
    "MB",
    "MG",
    "MR",
    "NULL",
    "WKS",
    "PTR",
    "HINFO",
    "MINFO",
    "MX",
    "TXT",
    "RP",
    "AFSDB",
    "X25",
    "ISDN",
    "RT",
    "NSAP",
    "NSAP-PTR",
    "SIG",
    "KEY",
    "PX",
    "GPOS",
    "AAAA",
    "LOC",
    "NXT",
    "EID",
    "NIMLOC",
    "SRV",
    "ATMA",
    "NAPTR",
    "KX",
    "CERT",
    "A6",
    "DNAME",
    "SINK",
    "OPT",
    "APL",
    "DS",
    "SSHFP",
    "IPSECKEY",
    "RRSIG",
    "NSEC",
    "DNSKEY",
    "DHCID",
    "NSEC3",
    "NSEC3PARAM",
    "TLSA",
    "SMIMEA",
    "Unassigned",
    "HIP",
    "NINFO",
    "RKEY",
    "TALINK",
    "CDS",
    "CDNSKEY",
    "OPENPGPKEY",
    "CSYNC"
};

static char const * rrtype_99_109[] = {
    "SPF",
    "UINFO",
    "UID",
    "GID",
    "UNSPEC",
    "NID",
    "L32",
    "L64",
    "LP",
    "EUI48",
    "EUI64"
};

static char const *  rrtype_249_258[] = {
    "TKEY",
    "TSIG",
    "IXFR",
    "AXFR",
    "MAILB",
    "MAILA",
    "RRCLASS * (ANY)",
    "URI",
    "CAA",
    "AVC"
};

static char const *  rrtype_32768_32769[] = {
    "TA",
    "DLV"
};

void DnsStats::PrintRRType(FILE * F, uint32_t rrtype)
{
    if (rrtype >= 1 && rrtype <= 62)
    {
        fprintf(F, """%s"",", rrtype_1_62[rrtype - 1]);
    }
    else if (rrtype >= 99 && rrtype <= 109)
    {
        fprintf(F, """%s"",", rrtype_99_109[rrtype - 99]);
    }
    else if (rrtype >= 249 && rrtype <= 258)
    {
        fprintf(F, """%s"",", rrtype_249_258[rrtype - 249]);
    }
    else if (rrtype >= 32768 && rrtype <= 32769)
    {
        fprintf(F, """%s"",", rrtype_32768_32769[rrtype - 32768]);
    }
    else if (rrtype >= 110 && rrtype <= 248)
    {
        fprintf(F, """Unassigned (%d)"",", rrtype);
    }
    else if (rrtype >= 110 && rrtype <= 248 ||
        rrtype >= 259 && rrtype <= 32767 ||
        rrtype >= 32770 && rrtype <= 65279)
    {
        fprintf(F, """Unassigned (%d)"",", rrtype);
    }
    else if (rrtype >= 65280 && rrtype <= 65534)
    {
        fprintf(F, """Private use (%d)"",", rrtype);
    }
    else
    {
        fprintf(F, """Reserved (%d)"",", rrtype);
    }

}

static char const *  rrclass_0_4[] = {
    "Reserved (0)",
    "Internet (IN)",
    "Unassigned (2)",
    "Chaos (CH)",
    "Hesiod (HS)"
};

static char const *  rrclass_254_255[] = {
    "QCLASS NONE",
    "QCLASS * (ANY)"
};

void DnsStats::PrintRRClass(FILE * F, uint32_t rrclass)
{
    if (rrclass <= 4)
    {
        fprintf(F, """%s"",", rrclass_0_4[rrclass]);
    }
    else if (rrclass >= 254 && rrclass <= 255)
    {
        fprintf(F, """%s"",", rrclass_254_255[rrclass - 254]);
    }
    else if (rrclass >= 5 && rrclass <= 253 ||
        rrclass >= 256 && rrclass <= 65279)
    {
        fprintf(F, """Unassigned (%d)"",", rrclass);
    }
    else if (rrclass >= 65280 && rrclass <= 65534)
    {
        fprintf(F, """Private use (%d)"",", rrclass);
    }
    else
    {
        fprintf(F, """Reserved (%d)"",", rrclass);
    }
}

static char const *  opcode_0_5[] = {
    "Query",
    "IQuery",
    "Status",
    "Unassigned (3)",
    "Notify",
    "Update"
};

void DnsStats::PrintOpCode(FILE * F, uint32_t opcode)
{
    if (opcode <= 5)
    {
        fprintf(F, """%s"",", opcode_0_5[opcode]);
    }
    else
    {
        fprintf(F, """Unassigned(%d)"",", opcode);
    }
}

static char const *  rcode_0_25[] = {
    "NoError",
    "FormErr",
    "ServFail",
    "NXDomain",
    "NotImp",
    "Refused",
    "YXDomain",
    "YXRRSet",
    "NXRRSet",
    "NotAuth",
    "NotZone",
    "Unassigned (11)",
    "Unassigned (12)",
    "Unassigned (13)",
    "Unassigned (14)",
    "Unassigned (15)",
    "BADVERS",
    "BADSIG",
    "BADKEY",
    "BADTIME",
    "BADMODE",
    "BADNAME",
    "BADALG",
    "BADTRUNC",
    "BADCOOKIE"
};

void DnsStats::PrintRCode(FILE * F, uint32_t rcode)
{
    if (rcode <= 25)
    {
        fprintf(F, """%s"",", rcode_0_25[rcode]);
    }
    else if (rcode >= 3841 && rcode <= 4095)
    {
        fprintf(F, """Private use (%d)"",", rcode);
    }
    else if (rcode == 65535)
    {
        fprintf(F, """Reserved (%d)"",", rcode);
    }
    else
    {
        fprintf(F, """Unassigned(%d)"",", rcode);
    }
}

static char const * dns_flags_id[] = {
    "CD",
    "AD",
    "bit 9",
    "RA",
    "RD",
    "TC",
    "AA"
};

void DnsStats::PrintDnsFlags(FILE * F, uint32_t flag)
{
    if (flag < 7)
    {
        fprintf(F, """%s"",", dns_flags_id[flag]);
    }
    else
    {
        fprintf(F, """ %d"",", flag);
    }
}

void DnsStats::PrintEDnsFlags(FILE * F, uint32_t flag)
{
    if (flag == 15)
    {
        fprintf(F, """DO"",");
    }
    else
    {
        fprintf(F, """ %d"",", 16 - flag);
    }
}

static char const * dnssec_algo_id_0_16[] = {
    "DELETE",
    "RSAMD5",
    "DH",
    "DSA",
    "Reserved (4)",
    "RSASHA1",
    "DSA-NSEC3-SHA1",
    "RSASHA1-NSEC3-SHA1",
    "RSASHA256",
    "Reserved (9)",
    "RSASHA512",
    "Reserved (11)",
    "ECC-GOST",
    "ECDSAP256SHA256",
    "ECDSAP384SHA384",
    "ED25519",
    "ED448"
};

static char const * dnssec_algo_id_252_254[] = {
    "INDIRECT",
    "PRIVATEDNS",
    "PRIVATEOID"
};

void DnsStats::PrintKeyAlgorithm(FILE * F, uint32_t algo)
{
    if (algo <= 16)
    {
        fprintf(F, """%s"",", dnssec_algo_id_0_16[algo]);
    }
    else if (algo >= 252 && algo <= 254)
    {
        fprintf(F, """%s"",", dnssec_algo_id_252_254[algo - 252]);
    }
    else if (algo >= 17 && algo <= 122)
    {
        fprintf(F, """unassigned (%d)"",", algo);
    }
    else if (algo >= 17 && algo <= 122)
    {
        fprintf(F, """reserved (%d)"",", algo);
    }
}

static char const * edns_option_0_14[] = {
    "Reserved (0)",
    "LLQ",
    "UL",
    "NSID",
    "Reserved",
    "DAU",
    "DHU",
    "N3U",
    "edns-client-subnet",
    "EDNS EXPIRE",
    "COOKIE",
    "edns-tcp-keepalive",
    "Padding",
    "CHAIN",
    "edns-key-tag"
};


void DnsStats::PrintOptOption(FILE * F, uint32_t option)
{
    if (option <= 14)
    {
        fprintf(F, """%s"",", edns_option_0_14[option]);
    }
    else if (option == 26946)
    {
        fprintf(F, """DeviceID"",");
    }
    else if (option >= 15 && option <= 26945 ||
        option >= 26947 && option <= 65000)
    {
        fprintf(F, """Unassigned (%d)"",", option);
    }
    else if (option >= 65001 && option <= 65534)
    {
        fprintf(F, """Experimental (%d)"",", option);
    }
    else
    {
        fprintf(F, """Reserved (%d)"",", option);
    }
}

static char const * error_flag_code = "TCORKPDLF";

void DnsStats::PrintErrorFlags(FILE * F, uint32_t flags)
{
    if (flags == 0)
    {
        fprintf(F, """No error"",");
    }
    else
    {
        char flags_name[10];
        int n = 0;

        for (int i = 0; i < 9; i++)
        {
            if ((flags&(1 << i)) != 0)
            {
                flags_name[n] = error_flag_code[i];
                n++;
            }
        }
        flags_name[n] = 0;
        fprintf(F, """%s"",", flags_name);
    }
}

static char const * tld_name_format[] = {
    "unknown format",
    "all letters",
    "all numbers",
    "alpha_num",
    "dash",
    "alpha_dash",
    "num_dash",
    "alpha_num_dash",
};

const uint32_t nb_tld_name_format = sizeof(tld_name_format) / sizeof(char const *);

void DnsStats::PrintTldErrorClass(FILE * F, uint32_t tld_error_class)
{
    if (tld_error_class < nb_common_bad_tld)
    {
        fprintf(F, """.%s"",", common_bad_tld[tld_error_class]);
    }
    else if (tld_error_class == 62)
    {
        fprintf(F, """Non ASCII"",");
    }
    else if (tld_error_class == 63)
    {
        fprintf(F, """Special chars"",");
    }
    else
    {
        uint32_t name_format = tld_error_class / 64;
        uint32_t name_length = tld_error_class % 64;

        if (name_format < nb_tld_name_format)
        {
            fprintf(F, """%s (%d)"",", tld_name_format[name_format], name_length);
        }
        else
        {
            fprintf(F, """Unknown(%d)"",", tld_error_class);
        }
    }
}

void DnsStats::CheckRRType(uint32_t rrtype)
{
    if (!((rrtype >= 1 && rrtype <= 62) || (rrtype >= 99 && rrtype <= 109) ||
        (rrtype >= 249 && rrtype <= 258) || (rrtype >= 32768 && rrtype <= 32769)))
    {
        error_flags |= DNS_REGISTRY_ERROR_RRTYPE;
    }
}

void DnsStats::CheckRRClass(uint32_t rrclass)
{
    if (rrclass > 4 && (rrclass < 254 || rrclass > 255))
    {
        error_flags |= DNS_REGISTRY_ERROR_RRCLASS;
    }
}

void DnsStats::CheckOpCode(uint32_t opcode)
{
    if (opcode > 5 || opcode == 3)
    {
        error_flags |= DNS_REGISTRY_ERROR_OPCODE;
    }
}

void DnsStats::CheckRCode(uint32_t rcode)
{
    if (rcode > 25 || (rcode  >= 11 && rcode <= 15))
    {
        error_flags |= DNS_REGISTRY_ERROR_RCODE;
    }
}

void DnsStats::CheckKeyAlgorithm(uint32_t algo)
{
    if (!(algo <= 16 || (algo >= 252 && algo <= 254)))
    {
        error_flags |= DNS_REGISTRY_ERROR_KALGO;
    }
}

void DnsStats::CheckOptOption(uint32_t option)
{
    if (!(option <= 14 || option == 26946))
    {
        error_flags |= DNS_REGISTRY_ERROR_OPTO;
    }
}

int DnsStats::CheckForUnderline(uint8_t * packet, uint32_t length, uint32_t start)
{
    uint32_t l = 0;
    uint32_t offset = 0;
    uint32_t previous = 0;
    uint32_t name_start = start;

    while (start < length)
    {
        l = packet[start];

        if (l == 0)
        {
            /* end of parsing*/
            start++;
            break;
        }
        else if ((l & 0xC0) == 0xC0)
        {
            if ((start + 2) > length)
            {
                /* error */
                start = length;
                break;
            }
            else
            {
                uint32_t new_start = ((l & 63) << 8) + packet[start + 1];

                if (new_start < name_start)
                {
                    (void)CheckForUnderline(packet, length, new_start);
                }

                start += 2;
                break;
            }
        }
        else if (l > 0x3F)
        {
            /* found an extension. Don't know how to parse it! */
            start = length;
            break;
        }
        else
        {
            /* Tracking of underscore labels. */
            if (start + l + 1 > length)
            {
                /* format error */
                start = length;
                break;
            }
            else
            {
                if (l > 0 && packet[start + 1] == '_')
                {
                    uint8_t underlined_name[64];
                    uint32_t flags;

                    NormalizeNamePart(l, &packet[start + 1], underlined_name, &flags);

                    if ((flags & 3) == 0)
                    {
                        SubmitRegistryString(REGISTRY_DNS_txt_underline, l, underlined_name);
                    }
                }
                previous = start;
                start += l + 1;
            }
        }
    }

    return start;
}

void DnsStats::NormalizeNamePart(uint32_t length, uint8_t * value,
    uint8_t * normalized, uint32_t * flags)
{
    bool has_letter = false;
    bool has_number = false;
    bool has_special = false;
    bool has_dash = false;
    bool has_non_ascii = false;

    for (uint32_t i = 0; i < length; i++)
    {
        uint8_t c = value[i];
        if (c >= 'A' && c <= 'Z')
        {
            c += 'a' - 'A';
            has_letter = true;
        }
        else if (c >= 'a' && c <= 'z')
        {
            has_letter = true;
        }
        else if (c >= '0' && c <= '9')
        {
            has_number = true;
        }
        else if (c == '-' || c == '_')
        {
            has_dash = true;
        }
        else if (c > 127)
        {
            has_non_ascii = true;
        }
        else if (c <= ' ' || c == 127 || c == '"' || c == ',')
        {
            c = '?';
            has_special = true;
        }
        else
        {
            has_special = true;
        }
        normalized[i] = c;
    }
    normalized[length] = 0;

    if (flags != NULL)
    {
        *flags = 0;

        if (has_non_ascii)
        {
            *flags += 1;
        }
        else if (has_special)
        {
            *flags += 2;
        }

        if (has_letter)
        {
            *flags += 64;
        }
        if (has_number)
        {
            *flags += 128;
        }
        if (has_dash)
        {
            *flags += 256;
        }
    }
}



static char const *  valid_tld[] = {
    "aaa",
    "aarp",
    "abarth",
    "abb",
    "abbott",
    "abbvie",
    "abc",
    "able",
    "abogado",
    "abudhabi",
    "ac",
    "academy",
    "accenture",
    "accountant",
    "accountants",
    "aco",
    "active",
    "actor",
    "ad",
    "adac",
    "ads",
    "adult",
    "ae",
    "aeg",
    "aero",
    "aetna",
    "af",
    "afamilycompany",
    "afl",
    "africa",
    "ag",
    "agakhan",
    "agency",
    "ai",
    "aig",
    "aigo",
    "airbus",
    "airforce",
    "airtel",
    "akdn",
    "al",
    "alfaromeo",
    "alibaba",
    "alipay",
    "allfinanz",
    "allstate",
    "ally",
    "alsace",
    "alstom",
    "am",
    "americanexpress",
    "americanfamily",
    "amex",
    "amfam",
    "amica",
    "amsterdam",
    "analytics",
    "android",
    "anquan",
    "anz",
    "ao",
    "aol",
    "apartments",
    "app",
    "apple",
    "aq",
    "aquarelle",
    "ar",
    "aramco",
    "archi",
    "army",
    "arpa",
    "art",
    "arte",
    "as",
    "asda",
    "asia",
    "associates",
    "at",
    "athleta",
    "attorney",
    "au",
    "auction",
    "audi",
    "audible",
    "audio",
    "auspost",
    "author",
    "auto",
    "autos",
    "avianca",
    "aw",
    "aws",
    "ax",
    "axa",
    "az",
    "azure",
    "ba",
    "baby",
    "baidu",
    "banamex",
    "bananarepublic",
    "band",
    "bank",
    "bar",
    "barcelona",
    "barclaycard",
    "barclays",
    "barefoot",
    "bargains",
    "baseball",
    "basketball",
    "bauhaus",
    "bayern",
    "bb",
    "bbc",
    "bbt",
    "bbva",
    "bcg",
    "bcn",
    "bd",
    "be",
    "beats",
    "beauty",
    "beer",
    "bentley",
    "berlin",
    "best",
    "bestbuy",
    "bet",
    "bf",
    "bg",
    "bh",
    "bharti",
    "bi",
    "bible",
    "bid",
    "bike",
    "bind",
    "bing",
    "bingo",
    "bio",
    "biz",
    "bj",
    "black",
    "blackfriday",
    "blanco",
    "blockbuster",
    "blog",
    "bloomberg",
    "blue",
    "bm",
    "bms",
    "bmw",
    "bn",
    "bnl",
    "bnpparibas",
    "bo",
    "boats",
    "boehringer",
    "bofa",
    "bom",
    "bond",
    "boo",
    "book",
    "booking",
    "boots",
    "bosch",
    "bostik",
    "boston",
    "bot",
    "boutique",
    "box",
    "br",
    "bradesco",
    "bridgestone",
    "broadway",
    "broker",
    "brother",
    "brussels",
    "bs",
    "bt",
    "budapest",
    "bugatti",
    "build",
    "builders",
    "business",
    "buy",
    "buzz",
    "bv",
    "bw",
    "by",
    "bz",
    "bzh",
    "ca",
    "cab",
    "cafe",
    "cal",
    "call",
    "calvinklein",
    "cam",
    "camera",
    "camp",
    "cancerresearch",
    "canon",
    "capetown",
    "capital",
    "capitalone",
    "car",
    "caravan",
    "cards",
    "care",
    "career",
    "careers",
    "cars",
    "cartier",
    "casa",
    "caseih",
    "cash",
    "casino",
    "cat",
    "catering",
    "catholic",
    "cba",
    "cbn",
    "cbre",
    "cbs",
    "cc",
    "cd",
    "ceb",
    "center",
    "ceo",
    "cern",
    "cern",
    "cf",
    "cfa",
    "cfd",
    "cg",
    "ch",
    "chanel",
    "channel",
    "chase",
    "chat",
    "cheap",
    "chintai",
    "chloe",
    "christmas",
    "chrome",
    "chrysler",
    "church",
    "ci",
    "cipriani",
    "circle",
    "cisco",
    "citadel",
    "citi",
    "citic",
    "city",
    "cityeats",
    "ck",
    "cl",
    "claims",
    "cleaning",
    "click",
    "clinic",
    "clinique",
    "clothing",
    "cloud",
    "club",
    "clubmed",
    "cm",
    "cn",
    "co",
    "coach",
    "codes",
    "coffee",
    "college",
    "cologne",
    "com",
    "comcast",
    "commbank",
    "community",
    "company",
    "compare",
    "computer",
    "comsec",
    "condos",
    "construction",
    "consulting",
    "contact",
    "contractors",
    "cooking",
    "cookingchannel",
    "cool",
    "coop",
    "corsica",
    "country",
    "coupon",
    "coupons",
    "courses",
    "cr",
    "credit",
    "creditcard",
    "creditunion",
    "cricket",
    "crown",
    "crs",
    "cruise",
    "cruises",
    "csc",
    "cu",
    "cuisinella",
    "cv",
    "cw",
    "cx",
    "cy",
    "cymru",
    "cyou",
    "cz",
    "dabur",
    "dad",
    "dance",
    "data",
    "date",
    "dating",
    "datsun",
    "day",
    "dclk",
    "dds",
    "de",
    "deal",
    "dealer",
    "deals",
    "degree",
    "delivery",
    "dell",
    "deloitte",
    "delta",
    "democrat",
    "dental",
    "dentist",
    "desi",
    "design",
    "dev",
    "dhl",
    "diamonds",
    "diet",
    "digital",
    "direct",
    "directory",
    "discount",
    "discover",
    "dish",
    "diy",
    "dj",
    "dk",
    "dm",
    "dnp",
    "do",
    "docs",
    "doctor",
    "dodge",
    "dog",
    "doha",
    "domains",
    "dot",
    "download",
    "drive",
    "dtv",
    "dubai",
    "duck",
    "dunlop",
    "duns",
    "dupont",
    "durban",
    "dvag",
    "dvr",
    "dz",
    "earth",
    "eat",
    "ec",
    "eco",
    "edeka",
    "edu",
    "education",
    "ee",
    "eg",
    "email",
    "emerck",
    "energy",
    "engineer",
    "engineering",
    "enterprises",
    "epost",
    "epson",
    "equipment",
    "er",
    "ericsson",
    "erni",
    "es",
    "esq",
    "estate",
    "esurance",
    "et",
    "eu",
    "eurovision",
    "eus",
    "events",
    "everbank",
    "exchange",
    "expert",
    "exposed",
    "express",
    "extraspace",
    "fage",
    "fail",
    "fairwinds",
    "faith",
    "family",
    "fan",
    "fans",
    "farm",
    "farmers",
    "fashion",
    "fast",
    "fedex",
    "feedback",
    "ferrari",
    "ferrero",
    "fi",
    "fiat",
    "fidelity",
    "fido",
    "film",
    "final",
    "finance",
    "financial",
    "fire",
    "firestone",
    "firmdale",
    "fish",
    "fishing",
    "fit",
    "fitness",
    "fj",
    "fk",
    "flickr",
    "flights",
    "flir",
    "florist",
    "flowers",
    "fly",
    "fm",
    "fo",
    "foo",
    "food",
    "foodnetwork",
    "football",
    "ford",
    "forex",
    "forsale",
    "forum",
    "foundation",
    "fox",
    "fr",
    "free",
    "fresenius",
    "frl",
    "frogans",
    "frontdoor",
    "frontier",
    "ftr",
    "fujitsu",
    "fujixerox",
    "fun",
    "fund",
    "furniture",
    "futbol",
    "fyi",
    "ga",
    "gal",
    "gallery",
    "gallo",
    "gallup",
    "game",
    "games",
    "gap",
    "garden",
    "gb",
    "gbiz",
    "gd",
    "gdn",
    "ge",
    "gea",
    "gent",
    "genting",
    "george",
    "gf",
    "gg",
    "ggee",
    "gh",
    "gi",
    "gift",
    "gifts",
    "gives",
    "giving",
    "gl",
    "glade",
    "glass",
    "gle",
    "global",
    "globo",
    "gm",
    "gmail",
    "gmbh",
    "gmo",
    "gmx",
    "gn",
    "godaddy",
    "gold",
    "goldpoint",
    "golf",
    "goo",
    "goodhands",
    "goodyear",
    "goog",
    "google",
    "gop",
    "got",
    "gov",
    "gp",
    "gq",
    "gr",
    "grainger",
    "graphics",
    "gratis",
    "green",
    "gripe",
    "group",
    "gs",
    "gt",
    "gu",
    "guardian",
    "gucci",
    "guge",
    "guide",
    "guitars",
    "guru",
    "gw",
    "gy",
    "hair",
    "hamburg",
    "hangout",
    "haus",
    "hbo",
    "hdfc",
    "hdfcbank",
    "health",
    "healthcare",
    "help",
    "helsinki",
    "here",
    "hermes",
    "hgtv",
    "hiphop",
    "hisamitsu",
    "hitachi",
    "hiv",
    "hk",
    "hkt",
    "hm",
    "hn",
    "hockey",
    "holdings",
    "holiday",
    "homedepot",
    "homegoods",
    "homes",
    "homesense",
    "honda",
    "honeywell",
    "horse",
    "hospital",
    "host",
    "hosting",
    "hot",
    "hoteles",
    "hotels",
    "hotmail",
    "house",
    "how",
    "hr",
    "hsbc",
    "ht",
    "htc",
    "hu",
    "hughes",
    "hyatt",
    "hyundai",
    "ibm",
    "icbc",
    "ice",
    "icn-2016",
    "icu",
    "id",
    "ie",
    "ieee",
    "ifm",
    "ikano",
    "il",
    "il",
    "im",
    "imamat",
    "imdb",
    "immo",
    "immobilien",
    "in",
    "industries",
    "infiniti",
    "info",
    "ing",
    "ink",
    "institute",
    "insure",
    "int",
    "intel",
    "international",
    "intuit",
    "investments",
    "io",
    "ipiranga",
    "iq",
    "ir",
    "irish",
    "is",
    "iselect",
    "ismaili",
    "ist",
    "istanbul",
    "it",
    "itau",
    "itv",
    "iveco",
    "iwc",
    "jaguar",
    "java",
    "jcb",
    "jcp",
    "je",
    "jeep",
    "jetzt",
    "jewelry",
    "jio",
    "jlc",
    "jll",
    "jm",
    "jmp",
    "jnj",
    "jo",
    "jobs",
    "joburg",
    "jot",
    "joy",
    "jp",
    "jpmorgan",
    "jprs",
    "juegos",
    "juniper",
    "kaufen",
    "kddi",
    "ke",
    "kerryproperties",
    "kfh",
    "kg",
    "kh",
    "ki",
    "kia",
    "kim",
    "kinder",
    "kindle",
    "kitchen",
    "kiwi",
    "km",
    "kn",
    "koeln",
    "komatsu",
    "kosher",
    "kp",
    "kpmg",
    "kpn",
    "kr",
    "krd",
    "kred",
    "kuokgroup",
    "kw",
    "ky",
    "kyoto",
    "kz",
    "la",
    "lacaixa",
    "ladbrokes",
    "lamborghini",
    "lamer",
    "lancaster",
    "lancia",
    "lancome",
    "land",
    "landrover",
    "lanxess",
    "lasalle",
    "lat",
    "latino",
    "latrobe",
    "law",
    "lawyer",
    "lb",
    "lc",
    "lds",
    "lease",
    "leclerc",
    "lefrak",
    "legal",
    "lego",
    "lexus",
    "lgbt",
    "li",
    "liaison",
    "lidl",
    "life",
    "lifeinsurance",
    "lifestyle",
    "lighting",
    "like",
    "lilly",
    "limited",
    "limo",
    "lincoln",
    "linde",
    "link",
    "lipsy",
    "live",
    "living",
    "lixil",
    "lk",
    "loan",
    "loans",
    "locker",
    "locus",
    "loft",
    "lol",
    "london",
    "lotte",
    "lotto",
    "love",
    "lpl",
    "lplfinancial",
    "lr",
    "ls",
    "lt",
    "ltd",
    "ltda",
    "lu",
    "lundbeck",
    "lupin",
    "luxe",
    "luxury",
    "lv",
    "ly",
    "ma",
    "macys",
    "madrid",
    "maif",
    "maison",
    "makeup",
    "man",
    "management",
    "mango",
    "market",
    "marketing",
    "markets",
    "marriott",
    "marshalls",
    "maserati",
    "mattel",
    "mba",
    "mc",
    "mcd",
    "mcdonalds",
    "mckinsey",
    "md",
    "me",
    "med",
    "media",
    "meet",
    "melbourne",
    "meme",
    "memorial",
    "men",
    "menu",
    "meo",
    "metlife",
    "mg",
    "mh",
    "miami",
    "microsoft",
    "mil",
    "mini",
    "mint",
    "mit",
    "mitsubishi",
    "mk",
    "ml",
    "mlb",
    "mls",
    "mm",
    "mma",
    "mn",
    "mo",
    "mobi",
    "mobile",
    "mobily",
    "moda",
    "moe",
    "moi",
    "mom",
    "monash",
    "money",
    "monster",
    "montblanc",
    "mopar",
    "mormon",
    "mortgage",
    "moscow",
    "moto",
    "motorcycles",
    "mov",
    "movie",
    "movistar",
    "mp",
    "mq",
    "mr",
    "ms",
    "msd",
    "mt",
    "mtn",
    "mtr",
    "mu",
    "museum",
    "mutual",
    "mv",
    "mw",
    "mx",
    "my",
    "mz",
    "na",
    "nab",
    "nadex",
    "nagoya",
    "name",
    "nationwide",
    "natura",
    "navy",
    "nba",
    "nc",
    "ne",
    "nec",
    "net",
    "netbank",
    "netflix",
    "network",
    "neustar",
    "new",
    "newholland",
    "news",
    "next",
    "nextdirect",
    "nexus",
    "nf",
    "nfl",
    "ng",
    "ngo",
    "nhk",
    "ni",
    "nico",
    "nike",
    "nikon",
    "ninja",
    "nissan",
    "nissay",
    "nl",
    "no",
    "nokia",
    "northwesternmutual",
    "norton",
    "now",
    "nowruz",
    "nowtv",
    "np",
    "nr",
    "nra",
    "nrw",
    "ntt",
    "nu",
    "nyc",
    "nz",
    "obi",
    "observer",
    "off",
    "office",
    "okinawa",
    "olayan",
    "olayangroup",
    "oldnavy",
    "ollo",
    "om",
    "omega",
    "one",
    "ong",
    "onl",
    "online",
    "onyourside",
    "ooo",
    "open",
    "oracle",
    "orange",
    "ord-2017",
    "org",
    "organic",
    "origins",
    "osaka",
    "otsuka",
    "ott",
    "ovh",
    "pa",
    "page",
    "pamperedchef",
    "panasonic",
    "panerai",
    "paris",
    "pars",
    "partners",
    "parts",
    "party",
    "passagens",
    "pay",
    "pccw",
    "pe",
    "pet",
    "pf",
    "pfizer",
    "pg",
    "ph",
    "pharmacy",
    "philips",
    "phone",
    "photo",
    "photography",
    "photos",
    "physio",
    "piaget",
    "pics",
    "pictet",
    "pictures",
    "pid",
    "pin",
    "ping",
    "pink",
    "pioneer",
    "pizza",
    "pk",
    "pl",
    "place",
    "play",
    "playstation",
    "plumbing",
    "plus",
    "pm",
    "pn",
    "pnc",
    "pohl",
    "poker",
    "politie",
    "porn",
    "post",
    "pr",
    "pramerica",
    "praxi",
    "press",
    "prime",
    "pro",
    "prod",
    "productions",
    "prof",
    "progressive",
    "promo",
    "properties",
    "property",
    "protection",
    "pru",
    "prudential",
    "ps",
    "pt",
    "pub",
    "pw",
    "pwc",
    "py",
    "qa",
    "qpon",
    "quebec",
    "quest",
    "qvc",
    "racing",
    "radio",
    "raid",
    "re",
    "read",
    "realestate",
    "realtor",
    "realty",
    "recipes",
    "red",
    "redstone",
    "redumbrella",
    "rehab",
    "reise",
    "reisen",
    "reit",
    "reliance",
    "ren",
    "rent",
    "rentals",
    "repair",
    "report",
    "republican",
    "rest",
    "restaurant",
    "review",
    "reviews",
    "rexroth",
    "rich",
    "richardli",
    "ricoh",
    "rightathome",
    "ril",
    "rio",
    "rip",
    "rmit",
    "ro",
    "rocher",
    "rocks",
    "rodeo",
    "rogers",
    "room",
    "rs",
    "rsvp",
    "ru",
    "rugby",
    "ruhr",
    "run",
    "rw",
    "rwe",
    "ryukyu",
    "sa",
    "saarland",
    "safe",
    "safety",
    "sakura",
    "sale",
    "salon",
    "samsclub",
    "samsung",
    "sandvik",
    "sandvikcoromant",
    "sanofi",
    "sap",
    "sapo",
    "sarl",
    "sas",
    "save",
    "saxo",
    "sb",
    "sbi",
    "sbs",
    "sc",
    "sca",
    "scb",
    "schaeffler",
    "schmidt",
    "scholarships",
    "school",
    "schule",
    "schwarz",
    "science",
    "scjohnson",
    "scor",
    "scot",
    "sd",
    "se",
    "seat",
    "secure",
    "security",
    "seek",
    "select",
    "sener",
    "server",
    "services",
    "ses",
    "seven",
    "sew",
    "sex",
    "sexy",
    "sfr",
    "sg",
    "sh",
    "shangrila",
    "sharp",
    "shaw",
    "shell",
    "shia",
    "shiksha",
    "shoes",
    "shop",
    "shopping",
    "shouji",
    "show",
    "showtime",
    "shriram",
    "si",
    "silk",
    "sina",
    "singles",
    "site",
    "sj",
    "sk",
    "ski",
    "skin",
    "sky",
    "skype",
    "sl",
    "sling",
    "sm",
    "smart",
    "smile",
    "sn",
    "sncf",
    "so",
    "soccer",
    "social",
    "softbank",
    "software",
    "sohu",
    "solar",
    "solutions",
    "song",
    "sony",
    "soy",
    "space",
    "spiegel",
    "spot",
    "spreadbetting",
    "sr",
    "srl",
    "srt",
    "st",
    "stada",
    "staples",
    "star",
    "starhub",
    "statebank",
    "statefarm",
    "statoil",
    "stc",
    "stcgroup",
    "stockholm",
    "storage",
    "store",
    "stream",
    "studio",
    "study",
    "style",
    "su",
    "sucks",
    "supplies",
    "supply",
    "support",
    "surf",
    "surgery",
    "suzuki",
    "sv",
    "swatch",
    "swiftcover",
    "swiss",
    "sx",
    "sy",
    "sydney",
    "symantec",
    "systems",
    "sz",
    "tab",
    "taipei",
    "talk",
    "taobao",
    "target",
    "tatamotors",
    "tatar",
    "tattoo",
    "tax",
    "taxi",
    "tc",
    "tci",
    "td",
    "tdk",
    "team",
    "tech",
    "technology",
    "tel",
    "telecity",
    "telefonica",
    "temasek",
    "tennis",
    "teva",
    "tf",
    "tg",
    "th",
    "thd",
    "theater",
    "theatre",
    "tiaa",
    "tickets",
    "tienda",
    "tiffany",
    "tips",
    "tires",
    "tirol",
    "tj",
    "tjmaxx",
    "tjx",
    "tk",
    "tkmaxx",
    "tl",
    "tm",
    "tmall",
    "tn",
    "to",
    "today",
    "tokyo",
    "tools",
    "top",
    "toray",
    "toshiba",
    "total",
    "tours",
    "town",
    "toyota",
    "toys",
    "tr",
    "trade",
    "trading",
    "training",
    "travel",
    "travelchannel",
    "travelers",
    "travelersinsurance",
    "trust",
    "trv",
    "tt",
    "tube",
    "tui",
    "tunes",
    "tushu",
    "tv",
    "tvs",
    "tw",
    "tz",
    "ua",
    "ubank",
    "ubs",
    "uconnect",
    "ug",
    "uk",
    "unicom",
    "university",
    "uno",
    "uol",
    "ups",
    "us",
    "uy",
    "uz",
    "va",
    "vacations",
    "vana",
    "vanguard",
    "vc",
    "ve",
    "vegas",
    "ventures",
    "verisign",
    "versicherung",
    "vet",
    "vg",
    "vi",
    "viajes",
    "video",
    "vig",
    "viking",
    "villas",
    "vin",
    "vip",
    "virgin",
    "visa",
    "vision",
    "vista",
    "vistaprint",
    "viva",
    "vivo",
    "vlaanderen",
    "vn",
    "vodka",
    "volkswagen",
    "volvo",
    "vote",
    "voting",
    "voto",
    "voyage",
    "vu",
    "vuelos",
    "wales",
    "walmart",
    "walter",
    "wang",
    "wanggou",
    "warman",
    "watch",
    "watches",
    "weather",
    "weatherchannel",
    "webcam",
    "weber",
    "website",
    "wed",
    "wedding",
    "weibo",
    "weir",
    "wf",
    "whoswho",
    "wien",
    "wiki",
    "williamhill",
    "win",
    "windows",
    "wine",
    "winners",
    "wme",
    "wolterskluwer",
    "woodside",
    "work",
    "works",
    "world",
    "wow",
    "ws",
    "wtc",
    "wtf",
    "xbox",
    "xerox",
    "xfinity",
    "xihuan",
    "xin",
    "xn--11b4c3d",
    "xn--1ck2e1b",
    "xn--1qqw23a",
    "xn--30rr7y",
    "xn--3bst00m",
    "xn--3ds443g",
    "xn--3e0b707e",
    "xn--3oq18vl8pn36a",
    "xn--3pxu8k",
    "xn--42c2d9a",
    "xn--45q11c",
    "xn--45brj9c",
    "xn--4gbrim",
    "xn--54b7fta0cc",
    "xn--55qx5d",
    "xn--5su34j936bgsg",
    "xn--5tzm5g",
    "xn--6frz82g",
    "xn--6qq986b3xl",
    "xn--80adxhks",
    "xn--80ao21a",
    "xn--80aqecdr1a",
    "xn--80asehdb",
    "xn--80aswg",
    "xn--8y0a063a",
    "xn--90a3ac",
    "xn--90ae",
    "xn--90ais",
    "xn--9dbq2a",
    "xn--9et52u",
    "xn--9krt00a",
    "xn--b4w605ferd",
    "xn--bck1b9a5dre4c",
    "xn--c1avg",
    "xn--c2br7g",
    "xn--cck2b3b",
    "xn--cg4bki",
    "xn--clchc0ea0b2g2a9gcd",
    "xn--czr694b",
    "xn--czrs0t",
    "xn--czru2d",
    "xn--d1acj3b",
    "xn--d1alf",
    "xn--e1a4c",
    "xn--efvy88h",
    "xn--estv75g",
    "xn--fiq228c5hs",
    "xn--fiqs8s",
    "xn--fiq228c5hs",
    "xn--fiq64b",
    "xn--fiqz9s",
    "xn--fjq720a",
    "xn--flw351e",
    "xn--flw351e",
    "xn--fpcrj9c3d",
    "xn--fpcrj9c3d",
    "xn--fzys8d69uvgm",
    "xn--g2xx48c",
    "xn--gckr3f0f",
    "xn--gecrj9c",
    "xn--gk3at1e",
    "xn--h2brj9c",
    "xn--hxt814e",
    "xn--i1b6b1a6a2e",
    "xn--imr513n",
    "xn--io0a7i",
    "xn--j1aef",
    "xn--j1amh",
    "xn--j6w193g",
    "xn--jlq61u9w7b",
    "xn--jvr189m",
    "xn--kcrx77d1x4a",
    "xn--kprw13d",
    "xn--kpu716f",
    "xn--kput3i",
    "xn--l1acc",
    "xn--lgbbat1ad8j",
    "xn--mgb9awbf",
    "xn--mgba3a4f16a",
    "xn--mgba7c0bbn0a",
    "xn--mgbab2bd",
    "xn--mgbai9azgqp6j",
    "xn--mgbai9azgqp6j",
    "xn--mgbayh7gpa",
    "xn--mgbb9fbpob",
    "xn--mgbbh1a71e",
    "xn--mgbca7dzdo",
    "xn--mgberp4a5d4ar",
    "xn--mgbi4ecexp",
    "xn--mgbpl2fh",
    "xn--mgbt3dhd",
    "xn--mgbx4cd0ab",
    "xn--mk1bu44c",
    "xn--mxtq1m",
    "xn--ngbc5azd",
    "xn--ngbe9e0a",
    "xn--ngbc5azd",
    "xn--node",
    "xn--node",
    "xn--nqv7f",
    "xn--nqv7fs00ema",
    "xn--nyqy26a",
    "xn--o3cw4h",
    "xn--ogbpf8fl",
    "xn--p1acf",
    "xn--p1ai",
    "xn--pbt977c",
    "xn--pgbs0dh",
    "xn--pssy2u",
    "xn--q9jyb4c",
    "xn--qcka1pmc",
    "xn--qxam",
    "xn--rhqv96g",
    "xn--rhqv96g",
    "xn--rovu88b",
    "xn--s9brj9c",
    "xn--ses554g",
    "xn--t60b56a",
    "xn--tckwe",
    "xn--tiq49xqyj",
    "xn--unup4y",
    "xn--vermgensberater-ctb",
    "xn--vhquv",
    "xn--vuq861b",
    "xn--w4r85el8fhu5dnra",
    "xn--w4rs40l",
    "xn--wgbl6a",
    "xn--xhq521b",
    "xn--xkc2al3hye2a",
    "xn--xkc2dl3a5ee0h",
    "xn--y9a3aq",
    "xn--yfro4i67o",
    "xn--ygbi2ammx",
    "xn--zfr164b",
    "xperia",
    "xxx",
    "xyz",
    "yachts",
    "yahoo",
    "yamaxun",
    "yandex",
    "ye",
    "yodobashi",
    "yoga",
    "yokohama",
    "you",
    "youtube",
    "yt",
    "yun",
    "za",
    "zappos",
    "zara",
    "zero",
    "zip",
    "zippo",
    "zm",
    "zone",
    "zuerich",
    "zw"
};

static const int nb_valid_tld = (int) (sizeof(valid_tld) / sizeof(const char *));

bool DnsStats::CheckTld(uint32_t length, uint8_t * lower_case_tld)
{
    int low = -1;
    int high = nb_valid_tld;
    int x = (high + low) / 2;
    bool is_valid = false;
    int cmp;
    bool ret = true;

    for (;;)
    {
        /* compare tld & valid_tld[x] */
        uint32_t i = 0;
        cmp = 0;

        for (i = 0; i < length; i++)
        {
            if (valid_tld[x][i] == 0)
            {
                cmp = 1;
                break;
            }
            else if (((uint8_t)valid_tld[x][i]) > lower_case_tld[i])
            {
                cmp = -1;
                break;
            }
            else if (((uint8_t)valid_tld[x][i]) < lower_case_tld[i])
            {
                cmp = 1;
                break;
            }
        }

        if (cmp == 0 && valid_tld[x][i] != 0)
        {
            cmp = -1;
        }

        if (cmp == 0)
        {
            break;
        }
        else if (cmp < 0)
        {
            high = x;
        }
        else
        {
            low = x;
        }

        x = (low + high) / 2;

        if (x <= low || x >= high)
        {
            break;
        }
    }

    if (cmp != 0)
    {
        error_flags |= DNS_REGISTRY_ERROR_TLD;
        ret = false;
    }

    return ret;
}



/*
* Examine the packet level information
*
* - DNS OpCodes
* - DNS RCodes
* - DNS Header Flags
*
* Analyze queries and responses.
* Special cases for TXT, KEY, CSYNC
*

1  1  1  1  1  1
0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                      ID                       |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                    QDCOUNT                    |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                    ANCOUNT                    |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                    NSCOUNT                    |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                    ARCOUNT                    |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*/

void DnsStats::SubmitPacket(uint8_t * packet, uint32_t length)
{
    bool is_response;
    bool has_header = true;
    uint32_t flags = 0;
    uint32_t opcode = 0;
    uint32_t rcode = 0;
    uint32_t e_rcode = 0;
    uint32_t qdcount = 0;
    uint32_t ancount = 0;
    uint32_t nscount = 0;
    uint32_t arcount = 0;
    uint32_t parse_index = 0;
    uint32_t e_length = 512;
    uint32_t tc_bit = 0;

    error_flags = 0;

    if (length < 12)
    {
        error_flags |= DNS_REGISTRY_ERROR_FORMAT;
        parse_index = length;
        has_header = false;
    }
    else
    {
        is_response = ((packet[2] & 128) != 0);

        if (is_response)
            response_count++;
        else
            query_count++;

        flags = ((packet[2] & 7) << 4) | ((packet[3] & 15) >> 4);
        opcode = (packet[2] >> 3) & 15;
        rcode = (packet[3] & 15);
        qdcount = (packet[4] << 8) | packet[5];
        ancount = (packet[6] << 8) | packet[7];
        nscount = (packet[8] << 8) | packet[9];
        arcount = (packet[10] << 8) | packet[11];

        SubmitRegistryNumber(REGISTRY_DNS_OpCodes, opcode);
        CheckOpCode(opcode);

        for (uint32_t i = 0; i < 7; i++)
        {
            if ((flags & (1 << i)) != 0)
            {
                SubmitRegistryNumber(REGISTRY_DNS_Header_Flags, i);
            }
        }

        parse_index = 12;
    }

    for (uint32_t i = 0; i < qdcount; i++)
    {
        if (parse_index >= length)
        {
            error_flags |= DNS_REGISTRY_ERROR_FORMAT;
        }
        parse_index = SubmitQuery(packet, length, parse_index, is_response);
    }

    for (uint32_t i = 0; i < ancount; i++)
    {
        if (parse_index >= length)
        {
            error_flags |= DNS_REGISTRY_ERROR_FORMAT;
        }
        parse_index = SubmitRecord(packet, length, parse_index, NULL, NULL, is_response);
    }

    for (uint32_t i = 0; i < nscount; i++)
    {
        if (parse_index >= length)
        {
            error_flags |= DNS_REGISTRY_ERROR_FORMAT;
        }
        parse_index = SubmitRecord(packet, length, parse_index, NULL, NULL, is_response);
    }

    for (uint32_t i = 0; i < arcount; i++)
    {
        if (parse_index >= length)
        {
            error_flags |= DNS_REGISTRY_ERROR_FORMAT;
        }
        parse_index = SubmitRecord(packet, length, parse_index, &e_rcode, &e_length, is_response);
    }

    if (has_header)
    {
        if (is_response)
        {
            SubmitRegistryNumber(REGISTRY_DNS_Response_Size, length);
            rcode |= (e_rcode << 4);
            CheckRCode(rcode);
            SubmitRegistryNumber(REGISTRY_DNS_RCODES, rcode);
            if ((flags & (1 << 5)) != 0)
            {
                SubmitRegistryNumber(REGISTRY_DNS_TC_length, e_length);
            }
        }
        else
        {
            SubmitRegistryNumber(REGISTRY_DNS_Query_Size, length);
            SubmitRegistryNumber(REGISTRY_EDNS_Packet_Size, e_length);
        }
    }

    SubmitRegistryNumber(REGISTRY_DNS_error_flag, error_flags);
}

bool CompareRegistryEntries(dns_registry_entry_t * x, dns_registry_entry_t * y)
{
    bool ret = false;

    if (x->registry_id < y->registry_id)
    {
        ret = true;
    }
    else if (x->registry_id == y->registry_id)
    {
        if (x->key_type < y->key_type)
        {
            ret = true;
        }
        else if (x->key_type == y->key_type)
        {
            if (x->key_type == 0)
            {
                ret = (x->key_number < y->key_number);
            }
            else
            {
                for (int i = 0; i < sizeof(x->key_value); i++)
                {
                    if (x->key_value[i] < y->key_value[i])
                    {
                        ret = true;
                        break;
                    }
                    else if (x->key_value[i] != y->key_value[i])
                    {
                        break;
                    }
                }
            }
        }
    }

    return ret;
}

bool DnsStats::ExportToCsv(char const * fileName)
{
    FILE* F;
    dns_registry_entry_t *entry;
    std::vector<dns_registry_entry_t *> lines(hashTable.GetCount());
#ifdef _WINDOWS
    errno_t err = fopen_s(&F, fileName, "w");
    bool ret = (err == 0);
#else
    bool ret;

    F = fopen(fileName, "w");
    ret = (F != NULL);
#endif

    if (ret)
    {
        int vector_index = 0;

        for (uint32_t i = 0; i < hashTable.GetSize(); i++)
        {
            entry = hashTable.GetEntry(i);
            if (entry != NULL)
            {
                lines[vector_index] = entry;
                vector_index++;
            }
        }
        std::sort(lines.begin(), lines.end(), CompareRegistryEntries);
    }

    if (ret)
    {
        for (dns_registry_entry_t * &entry : lines)
        {
            if (entry->registry_id < RegistryNameByIdNb)
            {
                fprintf(F, "%d, ""%s"",", entry->registry_id,
                    RegistryNameById[entry->registry_id]);
            }
            else
            {
                fprintf(F, "%d, %d,", entry->registry_id, entry->registry_id);
            }

            fprintf(F, "%d,", entry->key_type);

            if (entry->key_type == 0)
            {
                fprintf(F, "%d,", entry->key_number);
            }
            else
            {
                fprintf(F, """%s"",", entry->key_value);
            }

            if (entry->registry_id == REGISTRY_DNS_RRType ||
                entry->registry_id == REGISTRY_DNS_Q_RRType)
            {
                PrintRRType(F, entry->key_number);
            }
            else if (entry->registry_id == REGISTRY_DNS_CLASSES ||
                entry->registry_id == REGISTRY_DNS_Q_CLASSES)
            {
                PrintRRClass(F, entry->key_number);
            }
            else if (entry->registry_id == REGISTRY_DNS_OpCodes)
            {
                PrintOpCode(F, entry->key_number);
            }
            else if (entry->registry_id == REGISTRY_DNS_RCODES)
            {
                PrintRCode(F, entry->key_number);
            }
            else if (entry->registry_id == REGISTRY_DNS_Header_Flags)
            {
                PrintDnsFlags(F, entry->key_number);
            }
            else if (entry->registry_id == REGISTRY_EDNS_Header_Flags)
            {
                PrintEDnsFlags(F, entry->key_number);
            }
            else if (entry->registry_id == REGISTRY_DNSSEC_Algorithm_Numbers)
            {
                PrintKeyAlgorithm(F, entry->key_number);
            }
            else if (entry->registry_id == REGISTRY_EDNS_OPT_CODE)
            {
                PrintOptOption(F, entry->key_number);
            }
            else if (entry->registry_id == REGISTRY_DNS_error_flag)
            {
                PrintErrorFlags(F, entry->key_number);
            }
            else if (entry->registry_id == REGISTRY_TLD_error_class)
            {
                PrintTldErrorClass(F, entry->key_number);
            }
            else
            {
                fprintf(F, """ "",");
            }

            fprintf(F, "%d\n", entry->count);
        }

        fclose(F);
    }


    return ret;
}

