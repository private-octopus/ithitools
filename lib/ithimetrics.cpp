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

#include <stdio.h>
#include <string.h>
#include <algorithm>
#include "DnsStats.h"
#include "CaptureSummary.h"
#include "M7Getter.h"
#include "ithimetrics.h"

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

static metric6_registered_t r_header_flags[] = {
    { 5, "AA" },
    { 6, "TC" },
    { 7, "RD" },
    { 8, "RA" },
    { 10, "AD" },
    { 11, "CD" }
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
    "dns-parameters-4.csv"},
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

/* Metric computation */
ithimetrics::ithimetrics()
    :
    nb_rootqueries(0),
    nb_userqueries(0),
    nb_nondelegated(0),
    nb_delegated(0),
    m3_1(0),
    m3_2(0),
    m33_4(0),
    m4_1(0),
    m7(0)
{
}


ithimetrics::~ithimetrics()
{
}

bool ithimetrics::GetMetrics(CaptureSummary * cs)
{
    bool ret = true;
    
    GetM3_1(cs);
    GetM3_2(cs);
    GetM33_1(cs);
    GetM33_2(cs);
    GetM33_3(cs);
    GetM4_1(cs);
    GetM4_2(cs);
    GetM4_3(cs);
    GetM6(cs);

    return ret;
}

bool ithimetrics::GetM7(char const * zone_file_name)
{
    M7Getter getter;

    bool ret = getter.GetM7(zone_file_name);

    if (ret)
    {
        if (getter.nb_tld_queried > 0)
        {
            m7 = ((double)getter.nb_ds_present) / ((double)getter.nb_tld_queried);
        }
        else
        {
            ret = false;
        }
    }

    return ret;
}

bool ithimetrics::Save(char const * file_name)
{
    FILE* F;

#ifdef _WINDOWS
    errno_t err = fopen_s(&F, file_name, "w");
    bool ret = (err == 0);
#else
    bool ret;
    F = fopen(file_name, "w");
    ret = (F != NULL);
#endif

    if (ret)
    {
        fprintf(F, "M3.1, , %6f,\n", m3_1);
        fprintf(F, "M3.2, , %6f,\n", m3_2);

        m33_4 = m3_1;

        for (size_t i = 0; i < m33_1.size(); i++)
        {
            fprintf(F, "M3.3.1, %s, %6f,\n", m33_1[i].domain, m33_1[i].frequency);
            m33_4 -= m33_1[i].frequency;
        }

        for (size_t i = 0; i < m33_2.size(); i++)
        {
            fprintf(F, "M3.3.2, %s, %6f,\n", m33_2[i].domain, m33_2[i].frequency);
            m33_4 -= m33_2[i].frequency;
        }

        for (size_t i = 0; i < m33_3.size(); i++)
        {
            fprintf(F, "M3.3.3, %s, %6f,\n", m33_3[i].domain, m33_3[i].frequency);
            m33_4 -= m33_3[i].frequency;
        }

        fprintf(F, "M3.3.4, , %6f,\n", (m33_4 > 0)?m33_4:0);

        fprintf(F, "M4.1, , %6f,\n", m4_1);

        for (size_t i = 0; i < m4_2.size(); i++)
        {
            fprintf(F, "M4.2, %s, %6f,\n", m4_2[i].domain, m4_2[i].frequency);
        }

        for (size_t i = 0; i < m4_3.size(); i++)
        {
            fprintf(F, "M4.3, %s, %6f,\n", m4_3[i].domain, m4_3[i].frequency);
        }

        for (size_t i = 0; i < m6.size(); i++)
        {
            fprintf(F, "%s.1, , %6f,\n", m6[i].m6_prefix, m6[i].m6_x_1);
            fprintf(F, "%s.2, , %6f,\n", m6[i].m6_prefix, m6[i].m6_x_2);
            for (size_t j = 0; j < m6[i].m6_x_3.size(); j++)
            {
                fprintf(F, "%s.3, %d, %d,\n", m6[i].m6_prefix, 
                    m6[i].m6_x_3[j].parameter_value,
                    m6[i].m6_x_3[j].parameter_count);
            }
        }

        fprintf(F, "M7, , %6f,\n", m7);
    }

    if (F != NULL)
    {
        fclose(F);
    }

    return ret;
}

void ithimetrics::GetM3_1(CaptureSummary * cs)
{
    uint32_t nb_noerror = cs->GetCountByNumber(
        DnsStats::GetTableName(REGISTRY_DNS_root_QR), 0);
    uint32_t nb_nxdomain = cs->GetCountByNumber(
        DnsStats::GetTableName(REGISTRY_DNS_root_QR), 3);
    nb_rootqueries = nb_noerror + nb_nxdomain;

    if (nb_rootqueries > 0)
    {
        m3_1 = (double)nb_nxdomain;
        m3_1 /= (double)nb_rootqueries;
    }
    else
    {
        m3_1 = 0;
    }
}

void ithimetrics::GetM3_2(CaptureSummary * cs)
{
    uint32_t nb_useful = cs->GetCountByNumber(
        DnsStats::GetTableName(REGISTRY_DNS_UsefulQueries), 1);
    uint32_t nb_useless = cs->GetCountByNumber(
        DnsStats::GetTableName(REGISTRY_DNS_UsefulQueries), 0);
    uint32_t total = nb_useful + nb_useless;

    if (total > 0)
    {
        m3_2 = (double)nb_useless;
        m3_2 /= (double)total;
    }
    else
    {
        m3_2 = 0;
    }
}

void ithimetrics::GetM33_1(CaptureSummary * cs)
{
    GetM3_X(cs, REGISTRY_DNS_RFC6761TLD, &m33_1, 0);
}

void ithimetrics::GetM33_2(CaptureSummary * cs)
{
    GetM3_X(cs, REGISTRY_DNS_LeakedTLD, &m33_2, 0.001);
}

void ithimetrics::GetM33_3(CaptureSummary * cs)
{
    if (nb_rootqueries > 0)
    {
        std::vector<CaptureLine *> extract;

        cs->Extract(
            DnsStats::GetTableName(REGISTRY_DNS_LeakByLength), &extract);
        m33_3.reserve(extract.size());

        for (size_t i = 0; i < extract.size(); i++)
        {
            metric4_line_t line;
            size_t indx = 0;

            line.domain[indx++] = 'l';
            line.domain[indx++] = 'e';
            line.domain[indx++] = 'n';
            line.domain[indx++] = 'g';
            line.domain[indx++] = 't';
            line.domain[indx++] = 'h';
            line.domain[indx++] = '_';
            /* Length is between 0 and 64, and itoa is not portable */
            if (extract[i]->key_number < 10)
            {
                line.domain[indx++] = '0' + extract[i]->key_number;
            }
            else
            {
                line.domain[indx++] = '0' + extract[i]->key_number/10;
                line.domain[indx++] = '0' + extract[i]->key_number%10;
            }
            line.domain[indx++] = 0;
            line.frequency = ((double)extract[i]->count) / ((double)nb_rootqueries);

            if (extract.size() < 8 || line.frequency >= 0.001)
            {
                m33_3.push_back(line);
            }
        }

        std::sort(m33_3.begin(), m33_3.end(), metric4_line_is_bigger);
    }
}

void ithimetrics::GetM4_1(CaptureSummary * cs)
{
    nb_nondelegated = cs->GetCountByNumber(
        DnsStats::GetTableName(REGISTRY_DNS_TLD_Usage_Count), 0);
    nb_delegated = cs->GetCountByNumber(
        DnsStats::GetTableName(REGISTRY_DNS_TLD_Usage_Count), 1);
    nb_userqueries = nb_nondelegated + nb_delegated;

    if (nb_userqueries > 0)
    {
        m4_1 = (double)nb_delegated;
        m4_1 /= (double)nb_userqueries;
    }
    else
    {
        m4_1 = 0;
    }
}

void ithimetrics::GetM4_2(CaptureSummary * cs)
{
    GetM4_X(cs, REGISTRY_DNS_RFC6761_Usage, &m4_2, 0);
}

void ithimetrics::GetM4_3(CaptureSummary * cs)
{
    GetM4_X(cs, REGISTRY_DNS_Frequent_TLD_Usage, &m4_3, 0.001);
}

void ithimetrics::GetM3_X(CaptureSummary * cs, uint32_t table_id, 
    std::vector<metric4_line_t>* mstring_x, double min_share)
{
    GetStringM_X(cs, table_id, mstring_x, nb_rootqueries, min_share);
}

void ithimetrics::GetM4_X(CaptureSummary * cs, uint32_t table_id, std::vector<metric4_line_t>* mstring_x, double min_share)
{
    /* compute using nb_userqueries */
    if (nb_userqueries > 0)
    {
        GetStringM_X(cs, table_id, mstring_x, nb_userqueries, min_share);
    }
}

void ithimetrics::GetStringM_X(CaptureSummary * cs, uint32_t table_id, 
    std::vector<metric4_line_t>* mstring_x, uint32_t nbqueries, double min_share)
{
    if (nbqueries > 0)
    {
        std::vector<CaptureLine *> extract;

        cs->Extract(
            DnsStats::GetTableName(table_id), &extract);
        mstring_x->reserve(extract.size());

        for (size_t i = 0; i < extract.size(); i++)
        {
            metric4_line_t line;

            line.domain[strlen(extract[i]->key_value)] = 0;
            line.frequency = ((double)extract[i]->count) / ((double)nbqueries);

            if (extract.size() < 8 || line.frequency >= min_share)
            {
                memcpy(line.domain, extract[i]->key_value, strlen(extract[i]->key_value));
                mstring_x->push_back(line);
            }
        }

        std::sort(mstring_x->begin(), mstring_x->end(), metric4_line_is_bigger);
    }
}

void ithimetrics::GetM6(CaptureSummary * cs)
{
    size_t m6_size = sizeof(m6_metrics_list) / sizeof(metric6_def_t);

    m6.reserve(m6_size);

    for (size_t i = 0; i < m6_size; i++)
    {
        std::vector<CaptureLine *> extract;
        metric6_line_t line;

        line.m6_prefix = m6_metrics_list[i].m6_prefix;

        cs->Extract(
            DnsStats::GetTableName(m6_metrics_list[i].table_id), &extract);

        if (extract.size() == 0)
        {
            line.m6_x_1 = 0;
            line.m6_x_2 = 0;
        }
        else
        {
            size_t count_present = 0;
            size_t count_instances = 0;
            size_t count_unregistered = 0;

            for (size_t j = 0; j < extract.size(); j++)
            {
                bool registered = false;
                metric6_parameter_t parameter_data;

                for (size_t k = 0; k < m6_metrics_list[i].nb_registered; k++)
                {
                    if (m6_metrics_list[i].registry[k].key == extract[j]->key_number)
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
}

bool ithimetrics::metric4_line_is_bigger(metric4_line_t x, metric4_line_t y)
{
    bool ret = false;

    if (x.frequency > y.frequency)
    {
        ret = true;
    }
    else if (x.frequency == y.frequency)
    {
        ret = CaptureSummary::compare_string(x.domain, y.domain) > 0;
    }

    return (ret);
}
