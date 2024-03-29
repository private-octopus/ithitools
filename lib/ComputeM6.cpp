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

#include <string.h>
#include <algorithm>
#include "DnsStats.h"
#include "ComputeM6.h"
/* Initial definition of the registry tables */

static metric6_registered_t r_classes[] = {
    { 1, "Internet (IN)" },
    { 2, "Unassigned" },
    { 3, "Chaos (CH)" },
    { 4, "Hesiod (HS)" },
    { 254, "QCLASS NONE" },
    { 255, "QCLASS * (ANY)" }
};

static metric6_registered_t r_rrtypes[] = {
    { 1,"A" },
    { 2,"NS" },
    { 3,"MD" },
    { 4,"MF" },
    { 5,"CNAME" },
    { 6,"SOA" },
    { 7,"MB" },
    { 8,"MG" },
    { 9,"MR" },
    { 10,"NULL" },
    { 11,"WKS" },
    { 12,"PTR" },
    { 13,"HINFO" },
    { 14,"MINFO" },
    { 15,"MX" },
    { 16,"TXT" },
    { 17,"RP" },
    { 18,"AFSDB" },
    { 19,"X25" },
    { 20,"ISDN" },
    { 21,"RT" },
    { 22,"NSAP" },
    { 23,"NSAP-PTR" },
    { 24,"SIG" },
    { 25,"KEY" },
    { 26,"PX" },
    { 27,"GPOS" },
    { 28,"AAAA" },
    { 29,"LOC" },
    { 30,"NXT" },
    { 31,"EID" },
    { 32,"NIMLOC" },
    { 33,"SRV" },
    { 34,"ATMA" },
    { 35,"NAPTR" },
    { 36,"KX" },
    { 37,"CERT" },
    { 38,"A6" },
    { 39,"DNAME" },
    { 40,"SINK" },
    { 41,"OPT" },
    { 42,"APL" },
    { 43,"DS" },
    { 44,"SSHFP" },
    { 45,"IPSECKEY" },
    { 46,"RRSIG" },
    { 47,"NSEC" },
    { 48,"DNSKEY" },
    { 49,"DHCID" },
    { 50,"NSEC3" },
    { 51,"NSEC3PARAM" },
    { 52,"TLSA" },
    { 53,"SMIMEA" },
    { 54,"Unassigned" },
    { 55,"HIP" },
    { 56,"NINFO" },
    { 57,"RKEY" },
    { 58,"TALINK" },
    { 59,"CDS" },
    { 60,"CDNSKEY" },
    { 61,"OPENPGPKEY" },
    { 62,"CSYNC" },
    { 99,"SPF" },
    { 100,"UINFO" },
    { 101,"UID" },
    { 102,"GID" },
    { 103,"UNSPEC" },
    { 104,"NID" },
    { 105,"L32" },
    { 106,"L64" },
    { 107,"LP" },
    { 108,"EUI48" },
    { 109,"EUI64" },
    { 249,"TKEY" },
    { 250,"TSIG" },
    { 251,"IXFR" },
    { 252,"AXFR" },
    { 253,"MAILB" },
    { 254,"MAILA" },
    { 255,"*" },
    { 256,"URI" },
    { 257,"CAA" },
    { 258,"AVC" },
    { 259,"DOA" },
    { 32768,"TA" },
    { 32769,"DLV" }
};

static metric6_registered_t r_opcodes[] = {
    { 0,"Query" },
    { 1,"IQuery" },
    { 2,"Status" },
    { 4,"Notify" },
    { 5,"Update" }
};

static metric6_registered_t r_rcodes[] = {
    { 0, "NoError" },
    { 1, "FormErr" },
    { 2, "ServFail" },
    { 3, "NXDomain" },
    { 4, "NotImp" },
    { 5, "Refused" },
    { 6, "YXDomain" },
    { 7, "YXRRSet" },
    { 8, "NXRRSet" },
    { 9, "NotAuth" },
    { 9, "NotAuth" },
    { 10, "NotZone" },
    { 16, "BADVERS" },
    { 16, "BADSIG" },
    { 17, "BADKEY" },
    { 18, "BADTIME" },
    { 19, "BADMODE" },
    { 20, "BADNAME" },
    { 21, "BADALG" },
    { 22, "BADTRUNC" },
    { 23, "BADCOOKIE" }
};

static metric6_registered_t r_AFSDB_RRSubtype[] = {
    { 1, "Andrews File Service v3.0 Location Service" },
    { 2, "DCE / NCA root cell directory node" },
};

static metric6_registered_t r_DHCID_RRIdType[] = {
    { 0x0000, "htype" },
    { 0x0001, "client-id" }
};

static metric6_registered_t r_label_type[] = {
    { 0, "normal" },
    { 0xC0, "compressed" },
    { 0x40, "binary" },
    { 0x7C, "expansion" }
};

static metric6_registered_t r_opt_code[] = {
    { 1,"LLQ" },
    { 2,"UL" },
    { 3,"NSID" },
    { 4,"Reserved" },
    { 5,"DAU" },
    { 6,"DHU" },
    { 7,"N3U" },
    { 8,"edns-client-subnet" },
    { 9,"EDNS EXPIRE" },
    { 10,"COOKIE" },
    { 11,"edns-tcp-keepalive" },
    { 12,"Padding" },
    { 13,"CHAIN" },
    { 14,"edns-key-tag" },
    { 26946,"DeviceID" }
};

/*
 * Header flags are defined by IANA as bit positions, from 5 to 11, but the capture
 * program defines them as "shift positions", starting at 0. The number used
 * here are the shift positions, off by one from the IANA numbers.
 */

static metric6_registered_t r_header_flags[] = {
    { 4, "AA" },
    { 5, "TC" },
    { 6, "RD" },
    { 7, "RA" },
    { 9, "AD" },
    { 10, "CD" }
};

static metric6_registered_t r_edns_header_flags[] = {
    { 0, "DO" }
};

static metric6_registered_t r_edns_versions[] = {
    { 0, "EDNS version 0" }
};

static metric6_registered_t r_csync_flags[] = {
    { 0, "immediate" },
    { 1, "soaminimum" }
};

static metric6_registered_t r_dnssec_algos[] = {
    { 0, "Delete DS" },
    { 1, "RSA/MD5" },
    { 2, "Diffie-Hellman" },
    { 3, "DSA/SHA1" },
    { 5, "RSA/SHA-1" },
    { 6, "DSA-NSEC3-SHA1" },
    { 7, "RSASHA1-NSEC3-SHA1" },
    { 8, "RSA/SHA-256" },
    { 10, "RSA/SHA-512" },
    { 12, "GOST R 34.10-2001" },
    { 13, "ECDSA Curve P-256 with SHA-256" },
    { 14, "ECDSA Curve P-384 with SHA-384" },
    { 15, "Ed25519" },
    { 16, "Ed448" },
    { 252, "Reserved for Indirect Keys" },
    { 253, "private algorithm" },
    { 254, "private algorithm OID" }
};

static metric6_registered_t r_dnssec_prime_lengths[] = {
    { 1, "index 1" },
    { 2, "index 2" }
};

static metric6_registered_t r_dnssec_well_known_primes[] = {
    { 0x0001, "Well - Known Group 1: A 768 bit prime" },
    { 0x0002, "Well - Known Group 2 : A 1024 bit prime" }
};

static metric6_registered_t r_dane_cert_usage[] = {
    { 0, "PKIX-TA" },
    { 1, "PKIX-EE" },
    { 2, "DANE-TA" },
    { 3, "DANE-EE" }
};

static metric6_registered_t r_dane_selector[] = {
    { 0, "Cert" },
    { 1, "SPKI" },
    { 255, "PrivSel" }
};

static metric6_registered_t r_dane_matching_type[] = {
    { 0, "Full" },
    { 1, "SHA2-256" },
    { 2, "SHA2-512" },
    { 255, "PrivMatch" }
};

/* Initial table of m6 metrics */
metric6_def_t m6_metrics_list[] = {
    { "M6.DNS.01", REGISTRY_DNS_CLASSES,
    sizeof(r_classes) / sizeof(metric6_registered_t), r_classes,
    "dns-parameters-2.csv" },
    { "M6.DNS.02", REGISTRY_DNS_RRType,
    sizeof(r_rrtypes) / sizeof(metric6_registered_t), r_rrtypes,
    "dns-parameters-4.csv" },
    { "M6.DNS.03", REGISTRY_DNS_OpCodes,
    sizeof(r_opcodes) / sizeof(metric6_registered_t), r_opcodes,
    "dns-parameters-5.csv" },
    { "M6.DNS.04", REGISTRY_DNS_RCODES,
    sizeof(r_rcodes) / sizeof(metric6_registered_t), r_rcodes,
    "dns-parameters-6.csv" },
    { "M6.DNS.05", REGISTRY_DNS_AFSDB_RRSubtype,
    sizeof(r_AFSDB_RRSubtype) / sizeof(metric6_registered_t), r_AFSDB_RRSubtype,
    "dns-parameters-8.csv" },
    { "M6.DNS.06", REGISTRY_DNS_DHCID_RRIdType,
    sizeof(r_DHCID_RRIdType) / sizeof(metric6_registered_t), r_DHCID_RRIdType,
    "dns-parameters-9.csv" },
    { "M6.DNS.07", REGISTRY_DNS_LabelType,
    sizeof(r_label_type) / sizeof(metric6_registered_t), r_label_type,
    "dns-parameters-10.csv" },
    { "M6.DNS.08", REGISTRY_EDNS_OPT_CODE,
    sizeof(r_opt_code) / sizeof(metric6_registered_t), r_opt_code,
    "dns-parameters-11.csv" },
    { "M6.DNS.09", REGISTRY_DNS_Header_Flags,
    sizeof(r_header_flags) / sizeof(metric6_registered_t), r_header_flags,
    "dns-parameters-12.csv" },
    { "M6.DNS.10", REGISTRY_EDNS_Header_Flags,
    sizeof(r_edns_header_flags) / sizeof(metric6_registered_t), r_edns_header_flags,
    "dns-parameters-13.csv" },
    { "M6.DNS.11", REGISTRY_EDNS_Version_number,
    sizeof(r_edns_versions) / sizeof(metric6_registered_t), r_edns_versions,
    "dns-parameters-14.csv" },
    { "M6.DNS.12", REGISTRY_DNS_CSYNC_Flags,
    sizeof(r_csync_flags) / sizeof(metric6_registered_t), r_csync_flags,
    "csync-flags.csv" },
    { "M6.DNSSEC.1", REGISTRY_DNSSEC_KEY_Prime_Lengths,
    sizeof(r_dnssec_prime_lengths) / sizeof(metric6_registered_t), r_dnssec_prime_lengths,
    "prime-lengths.csv" },
    { "M6.DNSSEC.2", REGISTRY_DNSSEC_KEY_Well_Known_Primes,
    sizeof(r_dnssec_well_known_primes) / sizeof(metric6_registered_t), r_dnssec_well_known_primes,
    "prime-generator-pairs.csv" },
    { "M6.DNSSEC.3", REGISTRY_DNSSEC_Algorithm_Numbers,
    sizeof(r_dnssec_algos) / sizeof(metric6_registered_t), r_dnssec_algos,
    "dns-sec-alg-numbers-1.csv" },
    { "M6.DANE.1", REGISTRY_DANE_CertUsage,
    sizeof(r_dane_cert_usage) / sizeof(metric6_registered_t), r_dane_cert_usage,
    "certificate-usages.csv" },
    { "M6.DANE.2", REGISTRY_DANE_TlsaSelector,
    sizeof(r_dane_selector) / sizeof(metric6_registered_t), r_dane_selector,
    "selectors.csv" },
    { "M6.DANE.3", REGISTRY_DANE_TlsaMatchingType,
    sizeof(r_dane_matching_type) / sizeof(metric6_registered_t), r_dane_matching_type,
    "matching-types.csv" }
};

ComputeM6::ComputeM6()
{
}

ComputeM6::~ComputeM6()
{
}

bool ComputeM6::Load(char const * single_file_name)
{
    return cs.Load(single_file_name);
}

bool ComputeM6::LoadMultipleFiles(char const ** in_files, int nb_files)
{
    return (nb_files == 1) ? Load(in_files[0]) : cs.Merge(nb_files, in_files);
}

bool ComputeM6::Compute()
{
    size_t m6_size = sizeof(m6_metrics_list) / sizeof(metric6_def_t);
    bool found_at_least_one = false;

    m6.reserve(m6_size);

    for (size_t i = 0; i < m6_size; i++)
    {
        std::vector<CaptureLine *> extract;
        metric6_line_t line;

        line.m6_prefix = m6_metrics_list[i].m6_prefix;

        cs.Extract(
            DnsStats::GetTableName(m6_metrics_list[i].table_id), &extract);

        if (extract.size() == 0)
        {
            line.m6_x_1 = 0;
            line.m6_x_2 = 0;
        }
        else
        {
            size_t count_present = 0;
            uint64_t count_instances = 0;
            uint64_t count_unregistered = 0;
            found_at_least_one = true;

            for (size_t j = 0; j < extract.size(); j++)
            {
                bool registered = false;
                metric6_parameter_t parameter_data;

                for (size_t k = 0; k < m6_metrics_list[i].nb_registered; k++)
                {
                    if ((int)m6_metrics_list[i].registry[k].key == extract[j]->key_number)
                    {
                        registered = true;
                        break;
                    }
                }

                count_instances += extract[j]->count;
                parameter_data.parameter_count = extract[j]->count;
                parameter_data.parameter_value = extract[j]->key_number;
                line.m6_x_3.push_back(parameter_data);

                if (registered)
                {
                    count_present++;
                }
                else
                {
                    count_unregistered += extract[j]->count;
                }
            }

            line.m6_x_1 = ((double)count_present) / ((double)m6_metrics_list[i].nb_registered);
            line.m6_x_2 = ((double)count_unregistered) / ((double)count_instances);
        }
        m6.push_back(line);
    }

    return found_at_least_one;
}

bool ComputeM6::Write(FILE * F_out, char const* date, char const* version)
{
    bool ret = true;

    for (size_t i = 0; i < m6.size(); i++)
    {
        ret &= fprintf(F_out, "%s.1,%s,%s, , %6f,\n", m6[i].m6_prefix,date, version, m6[i].m6_x_1) > 0;
        ret &= fprintf(F_out, "%s.2,%s,%s, , %6f,\n", m6[i].m6_prefix, date, version, m6[i].m6_x_2) > 0;
        for (size_t j = 0; j < m6[i].m6_x_3.size(); j++)
        {
            ret &= fprintf(F_out, "%s.3,%s,%s, %d, %llu,\n", m6[i].m6_prefix, date, version,
                m6[i].m6_x_3[j].parameter_value,
                (unsigned long long)m6[i].m6_x_3[j].parameter_count) > 0;
        }
    }

    return ret;
}

metric6_def_t const * ComputeM6::GetTable(char const * m6_prefix)
{
    size_t m6_size = sizeof(m6_metrics_list) / sizeof(metric6_def_t);
    metric6_def_t const * ret = NULL;

    for (size_t i = 0; i < m6_size; i++) {
        if (strcmp(m6_prefix, m6_metrics_list[i].m6_prefix) == 0) {
            ret = &m6_metrics_list[i];
        }
    }

    return ret;
}
