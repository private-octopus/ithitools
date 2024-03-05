#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "cbor.h"
#include "cdns.h"
#include "ipstats.h"
#include "DnsStats.h"
#include "AddressFilter.h"
#include <algorithm>
#include <math.h>
#include <time.h>

#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>

#else  /* Linux */

#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef __USE_XOPEN2K
#define __USE_XOPEN2K
#endif
#ifndef __USE_POSIX
#define __USE_POSIX
#endif

#endif /* _WINDOWS or Linux*/

/* Helper function to parse csv cells in text lines
*/
void ParseCsvCell(char const* line, size_t* index, size_t* index_first, size_t* index_last)
{
    size_t i = *index;

    /* Skip the first blanks */
    while (line[i] == ' ' || line[i] == '\t' || line[i] != '\r' || line[i] != '\n') {
        i++;
    }
    /* Skip until the next blank or comma */
    *index_first = i;
    while (line[i] != ' ' && line[i] != '\t' && line[i] != '\r' && line[i] != '\n' && line[i] != ',' && line[i] != 0) {
        i++;
    }
    *index_last = i;
    /* Skip the blanks until the comma */
    while (line[i] == ' ' || line[i] == '\t' || line[i] != '\r' || line[i] != '\n') {
        i++;
    }
    /* Skip the comma if present */
    if (line[i] == ',') {
        i++;
    }
    *index = i;
}

uint64_t ParseUint64Cell(char const* line, size_t* index)
{
    size_t index_first;
    size_t index_last;
    uint64_t number = 0;
    ParseCsvCell(line, index, &index_first, &index_last);
    for (size_t i = index_first; i <= index_last; i++) {
        char c = line[i];
        if (c >= '0' && c <= '9') {
            number *= 10;
            number += (c - '0');
        }
    }
    return number;
}

size_t ParseIPCell(char const* line, size_t* index, uint8_t * IPAddress)
{
    size_t index_first;
    size_t index_last;
    char addrText[64];
    size_t t_len;
    size_t addr_len = 0;

    ParseCsvCell(line, index, &index_first, &index_last);
    t_len = index_last - index_first;
    if (t_len > 0 && t_len <= 63) {
        struct in_addr ipv4_addr;
        struct in6_addr ipv6_addr;

        memcpy(addrText, line + index_first, t_len);
        addrText[t_len] = 0;

        if (inet_pton(AF_INET, addrText, &ipv4_addr) == 1)
        {
            /* Valid IPv4 address */
            addr_len = 4;
            memcpy(IPAddress, (uint8_t*)&ipv4_addr, addr_len);
        }
        else  if (inet_pton(AF_INET6, addrText, &ipv6_addr) == 1)
        {
            /* Valid IPv6 address */
            addr_len = 16;
            memcpy(IPAddress, (uint8_t*)&ipv6_addr, addr_len);
        }
    }

    return addr_len;
}


IPStatsRecord::IPStatsRecord() :
    ipaddr_length(0),
    query_volume(0),
    arpa_count(0),
    no_such_domain_queries(0),
    no_such_domain_reserved(0),
    no_such_domain_frequent(0),
    no_such_domain_chromioids(0),
    HashNext(NULL)
{
    memset(ip_addr, 0, sizeof(ip_addr));
    memset(hourly_volume, 0, sizeof(hourly_volume));
    memset(daily_volume, 0, sizeof(daily_volume));
    memset(tld_counts, 0, sizeof(tld_counts));
    memset(sld_counts, 0, sizeof(sld_counts));
    memset(name_parts, 0, sizeof(name_parts));
    memset(rr_types, 0, sizeof(rr_types));
    memset(locales, 0, sizeof(locales));
    /* Debug variables */
    tld_length = 0;
    sld_length = 0;
    memset(TLD, 0, 64);
    memset(SLD, 0, 64);
}

IPStatsRecord::~IPStatsRecord()
{
}

bool IPStatsRecord::IsSameKey(IPStatsRecord* key)
{
    return (key->ipaddr_length == this->ipaddr_length &&
        memcmp(key->ip_addr, this->ip_addr, this->ipaddr_length) == 0);
}

uint32_t IPStatsRecord::Hash()
{
    const uint32_t fnv_prime_32 = 0x01000193;
    const uint32_t fnv_offset_32 = 0x811c9dc5;
    uint32_t h = fnv_offset_32;
    for (size_t i = 0; i < ipaddr_length; i++) {
        h = h * fnv_prime_32;
        h ^= ip_addr[i];
    }
    return h;
}

IPStatsRecord* IPStatsRecord::CreateCopy()
{
    IPStatsRecord * isr = new IPStatsRecord();

    if (isr != NULL) {
        isr->ipaddr_length = ipaddr_length;
        isr->query_volume = query_volume;
        isr->arpa_count = arpa_count;
        isr->no_such_domain_queries = no_such_domain_queries;
        isr->no_such_domain_reserved = no_such_domain_reserved;
        isr->no_such_domain_frequent = no_such_domain_frequent;
        isr->no_such_domain_chromioids = no_such_domain_chromioids;

        memcpy(isr->ip_addr, ip_addr, ipaddr_length);
        memcpy(isr->hourly_volume, hourly_volume, sizeof(hourly_volume));
        memcpy(isr->daily_volume, daily_volume, sizeof(daily_volume));
        memcpy(isr->tld_counts, tld_counts, sizeof(tld_counts));
        isr->tld_hyperlog.AddLogs(&tld_hyperlog);
        memcpy(isr->sld_counts, sld_counts, sizeof(sld_counts));
        isr->sld_hyperlog.AddLogs(&sld_hyperlog);
        memcpy(isr->name_parts, name_parts, sizeof(name_parts));
        memcpy(isr->rr_types, rr_types, sizeof(rr_types));
        memcpy(isr->locales, locales, sizeof(locales));
    }
    return isr;
}

void IPStatsRecord::Add(IPStatsRecord* key)
{
    ipaddr_length = key->ipaddr_length;
    query_volume += key->query_volume;
    arpa_count += key->arpa_count;
    no_such_domain_queries += key->no_such_domain_queries;
    no_such_domain_reserved += key->no_such_domain_reserved;
    no_such_domain_frequent += key->no_such_domain_frequent;
    no_such_domain_chromioids += key->no_such_domain_chromioids;

    add_vec(hourly_volume, key->hourly_volume, sizeof(hourly_volume)/sizeof(uint64_t));
    add_vec(daily_volume, key->daily_volume, sizeof(daily_volume)/sizeof(uint64_t));
    add_vec(tld_counts, key->tld_counts, sizeof(tld_counts)/sizeof(uint64_t));
    add_vec(sld_counts, key->sld_counts, sizeof(sld_counts)/sizeof(uint64_t));
    add_vec(name_parts, key->name_parts, sizeof(name_parts)/sizeof(uint64_t));
    add_vec(rr_types, key->rr_types, sizeof(rr_types)/sizeof(uint64_t));
    add_vec(locales, locales, sizeof(locales)/sizeof(uint64_t));

    tld_hyperlog.AddLogs(&key->tld_hyperlog);
    sld_hyperlog.AddLogs(&key->sld_hyperlog);
}

void IPStatsRecord::add_vec(uint64_t* x, uint64_t* y, size_t l)
{
    for (size_t i = 0; i < l; i++) {
        x[i] += y[i];
    }
}

bool IPStatsRecord::Serialize(uint8_t buffer, size_t buffer_size, size_t* length)
{
    return false;
}

bool IPStatsRecord::Deserialize(uint8_t buffer, size_t buffer_size, size_t* length)
{
    return false;
}

bool IPStatsRecord::SetIP(size_t ipaddr_length, uint8_t* ipaddr_v)
{
    bool ret = true;

    if (ipaddr_length != 16 && ipaddr_length != 4) {
        ret = false;
    }
    else {
        this->ipaddr_length = ipaddr_length;
        memcpy(this->ip_addr, ipaddr_v, ipaddr_length);
    }
    return ret;
}

bool IPStatsRecord::SetTime(int64_t qr_time)
{
    bool ret = true;
    struct tm time_m;
#ifdef _WINDOWS
    const __time64_t sourceTime = (__time64_t)qr_time;
    if (_gmtime64_s(&time_m, &sourceTime) != 0) {
        ret = false;
    }
#else
    time_t sourceTime = (time_t)qr_time;
    if (gmtime_r(&sourceTime, &time_m) == NULL) {
        ret = false;
    }
#endif
    if (ret) {
        this->hourly_volume[time_m.tm_hour] += 1;
        this->daily_volume[time_m.tm_mday] += 1;
    }
    return ret;
}

bool IPStatsRecord::SetQName(uint8_t* q_name, uint32_t q_name_length, IPStats * ip_stats)
{
    bool ret = true;
    uint32_t tld_offset = 0;
    int nb_name_parts = 0;
    uint32_t previous_offset = 0;

    ret = DnsStats::GetTLD(q_name, q_name_length, 0, &tld_offset, &previous_offset, &nb_name_parts);

    if (ret) {
        uint8_t tld[65];
        size_t tld_length = *(q_name + tld_offset);

        if (tld_length > 63) {
            ret = false;
        }
        else if (tld_length == 0 || nb_name_parts == 0) {
            /* This is a tabulation of root queries */
            this->name_parts[0] += 1;
        }
        else {
            /* Normalize the domain name to upper case. */
            memcpy(tld, q_name + tld_offset + 1, tld_length);
            DnsStats::SetToUpperCase(tld, tld_length);

            if (tld_length == 4 && memcmp(tld, "ARPA", 4) == 0) {
                /* This is a tabulation of ARPA queries */
                this->arpa_count += 1;
            }
            else if (ip_stats->IsRegisteredTLD(tld, tld_length)) {
                /* Document the TLD count */
                IPStatsRecord::SetTLD(tld_length, tld);
                /* Find the SLD and document the SLD count */
                if (ret && nb_name_parts > 1) {
                    uint8_t sld[65];
                    size_t sld_length = *(q_name + previous_offset);
                    if (sld_length > 63) {
                        ret = false;
                    }
                    else {
                        /* Normalize the domain name to upper case. */
                        memcpy(sld, q_name + previous_offset + 1, sld_length);
                        DnsStats::SetToUpperCase(sld, sld_length);
                        IPStatsRecord::SetSLD(sld_length, sld);
                    }
                }
                /* Document the number of name parts */
                if (nb_name_parts < 7) {
                    this->name_parts[nb_name_parts] += 1;
                }
                else {
                    this->name_parts[7] += 1;
                }
            }
            else {
                /* find the type of error */
                this->no_such_domain_queries += 1;
                if (DnsStats::IsRfc6761Tld(tld, tld_length)) {
                    this->no_such_domain_reserved += 1;
                }
                if (DnsStats::IsFrequentLeakTld(tld, tld_length)) {
                    this->no_such_domain_frequent += 1;
                }
                if (DnsStats::IsProbablyChromiumProbe(tld, tld_length, nb_name_parts)) {
                    this->no_such_domain_chromioids += 1;
                }
            }
        }
    }
    return ret;
}

int RR_subset[8] = {
    2 /* NS */,
    1 /* A */,
    28 /* AAAA */,
    12 /* PTR */,
    43 /* DS */,
    47 /* NSEC */,
    50 /* NSEC3 */,
    6 /* SOA */
};

bool IPStatsRecord::SetRR(int rr_type, int rr_class)
{
    bool ret = true;

    if (rr_class == 1) {
        for (int i = 0; i < 8; i++) {
            if (rr_type == RR_subset[i]) {
                this->rr_types[i] += 1;
                break;
            }
        }
    }

    return ret;
}

bool IPStatsRecord::WriteRecord(FILE* F)
{
    bool ret = WriteIP(F);
    ret &= (fprintf(F, ",%" PRIu64, query_volume) > 0);
    for (int i = 0; i < 24; i++) {
        ret &= (fprintf(F, ",%" PRIu64, hourly_volume[i]) > 0);
    }
    for (int i = 0; i < 31; i++) {
        ret &= (fprintf(F, ",%" PRIu64, daily_volume[i]) > 0);
    }
    ret &= (fprintf(F, ",%" PRIu64, arpa_count) > 0);
    ret &= (fprintf(F, ",%" PRIu64, no_such_domain_queries) > 0);
    ret &= (fprintf(F, ",%" PRIu64, no_such_domain_reserved) > 0);
    ret &= (fprintf(F, ",%" PRIu64, no_such_domain_frequent) > 0);
    ret &= (fprintf(F, ",%" PRIu64, no_such_domain_chromioids) > 0);
    for (int i = 0; i < 8; i++) {
        ret &= (fprintf(F, ",%" PRIu64, tld_counts[i]) > 0);
    }
    ret &= (fprintf(F, ",") > 0);
    ret &= tld_hyperlog.WriteHyperLogLog(F);
    for (int i = 0; i < 8; i++) {
        ret &= (fprintf(F, ",%" PRIu64, sld_counts[i]) > 0);
    }
    ret &= (fprintf(F, ",") > 0);
    ret &= sld_hyperlog.WriteHyperLogLog(F);
    for (int i = 0; i < 8; i++) {
        ret &= (fprintf(F, ",%" PRIu64, name_parts[i]) > 0);
    }
    for (int i = 0; i < 8; i++) {
        ret &= (fprintf(F, ",%" PRIu64, rr_types[i]) > 0);
    }
    for (int i = 0; i < 8; i++) {
        ret &= (fprintf(F, ",%" PRIu64, locales[i]) > 0);
    }
    ret &= (fprintf(F, "\n") > 0);

    return ret;
}

IPStatsRecord* IPStatsRecord::ParseLine(char const* line)
{
    size_t index = 0;
    IPStatsRecord* record = new IPStatsRecord();

    if (record != NULL) {
        record->ipaddr_length = ParseIPCell(line, &index, record->ip_addr);
        record->query_volume = ParseUint64Cell(line, &index);
        for (int i = 0; i < 24; i++) {
            record->hourly_volume[i] = ParseUint64Cell(line, &index);
        }
        for (int i = 0; i < 31; i++) {
            record->daily_volume[i] = ParseUint64Cell(line, &index);
        }
        record->arpa_count = ParseUint64Cell(line, &index);
        record->no_such_domain_queries = ParseUint64Cell(line, &index);
        record->no_such_domain_reserved = ParseUint64Cell(line, &index);
        record->no_such_domain_frequent = ParseUint64Cell(line, &index);
        record->no_such_domain_chromioids = ParseUint64Cell(line, &index);
        for (int i = 0; i < 8; i++) {
            record->tld_counts[i] = ParseUint64Cell(line, &index);
        }
        record->tld_hyperlog.ParseHyperLogLog(line, &index);
        for (int i = 0; i < 8; i++) {
            record->sld_counts[i] = ParseUint64Cell(line, &index);
        }
        record->sld_hyperlog.ParseHyperLogLog(line, &index);
        for (int i = 0; i < 8; i++) {
            record->name_parts[i] = ParseUint64Cell(line, &index);
        }
        for (int i = 0; i < 8; i++) {
            record->rr_types[i] = ParseUint64Cell(line, &index);
        }
        for (int i = 0; i < 8; i++) {
            record->locales[i] = ParseUint64Cell(line, &index);
        }
    }
    return record;
}

bool IPStatsRecord::WriteIP(FILE* F)
{
    char text[256];
    bool ret = AddressFilter::AddressText(ip_addr, ipaddr_length, text, 256);
    if (ret) {
        ret = (fprintf(F, "%s", text) > 0);
    }
    return ret;
}

void IPStatsRecord::SetXLD(size_t xld_length, uint8_t* xld, const char** XLD_subset, size_t nb_XLD_subset, uint64_t* xld_counts, HyperLogLog* xld_hyperlog)
{
    /* Find whether the TLD is in the subset */
    size_t subset_rank = SIZE_MAX;
    for (size_t i = 0; i < nb_XLD_subset; i++) {
        if (strlen(XLD_subset[i]) == xld_length &&
            memcmp(xld, XLD_subset[i], xld_length) == 0){
            subset_rank = i;
            break;
        }
    }
    if (subset_rank < SIZE_MAX) {
        xld_counts[subset_rank] += 1;
    }
    else {
        xld_hyperlog->AddKey(xld, xld_length);
    }
}

const char* TLD_subset[8] = {
    "COM", "NET", "ORG", "INFO", "CN", "IN", "DE", "US",
};

const size_t nb_TLD_subset = sizeof(TLD_subset) / sizeof(const char*);

void IPStatsRecord::SetTLD(size_t tld_length, uint8_t* tld)
{
#if 1
    this->tld_length = tld_length;
    memcpy(this->TLD, tld, tld_length);
#endif
    IPStatsRecord::SetXLD(tld_length, tld, TLD_subset, nb_TLD_subset, this->tld_counts, &this->tld_hyperlog);
}

const char* SLD_subset[8] = {
    "RESOLVER", "EC2", "CLOUD", "WPAD", "CORP", "MAIL", "_TCP", "PROD"
};

const size_t nb_SLD_subset = sizeof(SLD_subset) / sizeof(const char*);

void IPStatsRecord::SetSLD(size_t sld_length, uint8_t* sld)
{
    IPStatsRecord::SetXLD(sld_length, sld, SLD_subset, nb_SLD_subset, this->sld_counts, &this->sld_hyperlog);
#if 1
    this->sld_length = sld_length;
    memcpy(this->SLD, sld, sld_length);
#endif
}

void IPStatsRecord::DebugPrint(FILE* F)
{
#if 1
    uint8_t test_ip[4] = { 10, 9, 1, 109 };
    if (memcmp(this->ip_addr, test_ip, 4) == 0) {
        uint64_t tld_hash = HyperLogLog::Fnv64(TLD, tld_length);
        uint64_t sld_hash = HyperLogLog::Fnv64(SLD, sld_length);
        fprintf(F, "IP:%d.%d.%d.%d,", this->ip_addr[0], this->ip_addr[1], this->ip_addr[2], this->ip_addr[3]);
        fprintf(F, "TLD[%zu, 0x% " PRIx64 "] = .", tld_length, tld_hash);
        for (size_t i = 0; i < tld_length; i++) {
            int c = TLD[i];
            if (c >= 32 && c < 127) {
                fprintf(F, "%c.", c);
            }
            else {
                fprintf(F, "\\%x.", c);
            }
        }

        fprintf(F, ", SLD[%zu, 0x% " PRIx64 "] = .", sld_length, sld_hash);
        for (size_t i = 0; i < sld_length; i++) {
            int c = SLD[i];
            if (c >= 32 && c < 127) {
                fprintf(F, "%c.", c);
            }
            else {
                fprintf(F, "\\%x.", c);
            }
        }
        fprintf(F, "\n");
    }
#endif
}

IPStats::IPStats()
{
}

IPStats::~IPStats()
{
}

bool IPStats::LoadCborFiles(size_t nb_files, char const** fileNames)
{
    bool ret = true;

    for (size_t i = 0; ret && i < nb_files; i++)
    {
        ret = LoadCborFile(fileNames[i]);
    }

    return ret;
}

bool IPStats::LoadCborFile(char const* fileName)
{
    cdns cdns_ctx;
    int err;
    bool ret = cdns_ctx.open(fileName);

    while (ret) {
        if (!cdns_ctx.open_block(&err)) {
            ret = (err == CBOR_END_OF_ARRAY);
            break;
        }
        for (size_t i = 0; i < cdns_ctx.block.queries.size(); i++) {
            SubmitCborPacket(&cdns_ctx, i);
        }
    }
    return ret;
}

bool IPStats::SaveToCsv(char const* file_name)
{
    bool ret = true;
    FILE* F;
#ifdef _WINDOWS
    errno_t err = fopen_s(&F, file_name, "wt");
    if (err != 0) {
        if (F != NULL) {
            fclose(F);
            F = NULL;
        }
        ret = false;
    }
#else
    F = fopen(file_name, "wt");

    ret &= (F != NULL);
#endif

    if (F != NULL) {
        /* Enumerate the records in the binhash, store the keys in a vector */
        std::vector<IPStatsRecord*> records(ip_records.GetCount());
        size_t record_index = 0;

        for (uint32_t i = 0; i < ip_records.GetSize(); i++)
        {
            IPStatsRecord* record = ip_records.GetEntry(i);

            while (record != NULL) {
                records[record_index] = record;
                record_index++;
                record = record->HashNext;
            }
        }
        /* Sort the records by IP addresses */
        std::sort(records.begin(), records.end(), IPAddressIsLower);
        /* print the records to the file */
        for (size_t i = 0; i < records.size(); i++) {
            records[i]->WriteRecord(F);
        }
        /* Close the file */
        fclose(F);
    }

    return ret;
}

uint32_t IPStats::GetCount()
{
    return ip_records.GetCount();
}

bool IPStats::IPAddressIsLower(IPStatsRecord * x, IPStatsRecord * y)
{
    bool ret = (x->ipaddr_length < y->ipaddr_length) ||
        (x->ipaddr_length == y->ipaddr_length && memcmp(x->ip_addr, y->ip_addr, x->ipaddr_length) < 0);

    return ret;
}

void IPStats::SubmitCborPacket(cdns* cdns_ctx, size_t packet_id)
{
    /* TODO: add to database. */
    bool is_valid = true;
    IPStatsRecord ipsr;
    cdns_query* query = &cdns_ctx->block.queries[packet_id];
    cdns_query_signature* q_sig = NULL;

    if (query->client_address_index >= cdns_ctx->index_offset){
        size_t addrid = (size_t)query->client_address_index - cdns_ctx->index_offset;
        if (!ipsr.SetIP(cdns_ctx->block.tables.addresses[addrid].l, cdns_ctx->block.tables.addresses[addrid].v)) {
            /* malformed IP address. Cannot do anything with that record. */
            is_valid = false;
        }
    }

    if (is_valid) {
        if (query->query_signature_index >= cdns_ctx->index_offset) {
            q_sig = &cdns_ctx->block.tables.q_sigs[(size_t)query->query_signature_index - cdns_ctx->index_offset];
        }


        ipsr.query_volume += 1;

        /* Queried name and response type */
        if (q_sig != NULL &&
            query->query_name_index >= cdns_ctx->index_offset) {
            size_t nid = (size_t)query->query_name_index - cdns_ctx->index_offset;
            uint8_t* q_name = cdns_ctx->block.tables.name_rdata[nid].v;
            uint32_t q_name_length = (uint32_t)cdns_ctx->block.tables.name_rdata[nid].l;
            is_valid = ipsr.SetQName(q_name, q_name_length, this);
        }
        else {
            is_valid = false;
        }
    }

    /* Set date */
    if (is_valid) {
        /* Find hour and date */
        is_valid = ipsr.SetTime(cdns_ctx->block.preamble.earliest_time_sec);
    }

    /* TODO: Set RR */
    if (is_valid) {
        /* Find qr_class, so leak analysis can distinguish between Internet and Chaos */
        int rr_class = 0;
        int rr_type = 0;
        if (q_sig != NULL &&
            query->query_name_index >= cdns_ctx->index_offset) {
            /* assume just one query per q_sig, but sometimes there is none. */
            size_t cid = (size_t)q_sig->query_classtype_index - cdns_ctx->index_offset;
            rr_class = cdns_ctx->block.tables.class_ids[cid].rr_class;
            rr_type = cdns_ctx->block.tables.class_ids[cid].rr_type;
            is_valid = ipsr.SetRR(rr_type, rr_class);
        }
    }

    /* If the packet is valid, add the record to the table of addresses */
    if (is_valid) {
        bool stored = false;
        this->ip_records.InsertOrAdd(&ipsr, true, &stored);
    }
    ipsr.DebugPrint(stdout);
}

bool IPStats::IsRegisteredTLD(uint8_t* x, size_t l)
{
    return dnsstats.IsRegisteredTLD(x, l);
}

HyperLogLog::HyperLogLog()
{
    memset(this->hllv, 0, sizeof(this->hllv));
}

HyperLogLog::~HyperLogLog()
{
}

void HyperLogLog::AddLogs(const HyperLogLog* y)
{
    for (int i = 0; i < 16; i++) {
        if (y->hllv[i] > hllv[i]) {
            hllv[i] = y->hllv[i];
        }
    }
}

uint64_t HyperLogLog::Fnv64(const uint8_t* x, size_t l)
{
    uint64_t fnv64 = 0xcbf29ce484222325ull;
    const uint64_t fnv64_prime = 0x00000100000001B3ull;

    /* Compute the FNV 64 bit hash */
    for (size_t i = 0; i < l; i++) {
        fnv64 ^= x[i];
        fnv64 *= fnv64_prime;
    }
    return fnv64;
}

int HyperLogLog::BucketID(uint64_t fnv64)
{
    /* To reduce potential bias, compute bucket id as hash of top nibbles in FNV64
    * Exclude the bottom nibbles, because the number of trailing zeroes is computed from them.
     */
    uint32_t  bucket_id = 0;
    uint32_t top64 = (uint32_t)(fnv64>>32);
    for (size_t i = 0; i < 4; i++) {
        bucket_id ^= (int)top64;
        top64 >>= 8;
    }
    bucket_id ^= (bucket_id >> 4);
    bucket_id &= 0x0f;
    return (int)bucket_id;
}

int HyperLogLog::TrailingZeroes(uint64_t fnv64)
{
    int nb_zeroes = 0;
    uint64_t v64 = fnv64;
    for (size_t i = 0; i < 8; i++) {
        uint8_t v = (uint8_t)(v64 & 0xff);
        if (v == 0) {
            nb_zeroes += 8;
            v64 >>= 8;
            continue;
        }
        else {
            if ((v&0xf) == 0) {
                nb_zeroes += 4;
                v >>= 4;
            }
            if ((v&0x3) == 0) {
                nb_zeroes += 2;
                v >>= 2;
            }
            if ((v&0x1)== 0) {
                nb_zeroes += 1;
            }
            break;
        }
    }
    return nb_zeroes;
}

void HyperLogLog::AddKey(const uint8_t* x, size_t l)
{
    uint64_t fnv64 = HyperLogLog::Fnv64(x, l);
    int bucket_id = HyperLogLog::BucketID(fnv64);

    /* compute the number of zeroes plus 1.
     * the number 1 is because Flajolet's original counted the 
     * position of the first 1, and the rest of the algorithm
     * depends on that.
     */
    uint8_t nb_zeroes = 1 + HyperLogLog::TrailingZeroes(fnv64);
    /* Update the array of buckets */
    if (nb_zeroes > hllv[bucket_id]) {
        hllv[bucket_id] = nb_zeroes;
    }
}

size_t HyperLogLog::Serialize(uint8_t* buffer, size_t buffer_size)
{
    size_t ret = buffer_size + 1;
    if (buffer_size > 16) {
        memcpy(buffer, hllv, 16);
        ret = 16;
    }
    return ret;
}

size_t HyperLogLog::Deserialize(const uint8_t* buffer, size_t buffer_size)
{
    size_t ret = buffer_size + 1;
    if (buffer_size > 16) {
        memcpy(hllv, buffer, 16);
        ret = 16;
    }
    return ret;
}

double HyperLogLog::Assess()
{
    /*
    * First, compute the "indicator" of the m=16 registers
    * Z = 1 / sum (1 / 2^hll[j])
    * The indicator is NOT the same as the harmonic mean. The relation is:
    * harmonic_mean = m * Z
    */
    double divider = 0;
    for (int j = 0; j < 16; j++) {
        divider += 1.0 / ((double)(1 << hllv[j]));
    }
    double Z = 1.0 / divider;

    /*
    * Then, compute E using a precomputed coefficient to correct the bias of the formula
    * E = alpha_m * (m^2) * Z
    * For m = 16, use the precomputed value alpha_m = 0.673
    * 0.673 * (16^2) = 0.673*256 = 172.288
     */
    double E = Z * 172.288;

    /*
    * For small values (E < (5/2)*m , use Linear counting
    */
    if (E < 40.0) {
        int V = 0;
        for (int j = 0; j < 16; j++) {
            if (hllv[j] == 0) {
                V += 1;
            }
        }
        if (V > 0) {
            E = 16 * log(16.0 / (double)V);
        }
    }

    return E;
}

bool HyperLogLog::WriteHyperLogLog(FILE* F)
{
    double E = Assess();
    bool ret = (fprintf(F, "%f", E) > 0);
    for (int i = 0; i < 16; i++) {
        ret &= (fprintf(F, ",%u", hllv[i]) > 0);
    }
    return ret;
}

void HyperLogLog::ParseHyperLogLog(char const* line, size_t* index)
{
    /* Skip the float value */
    size_t index_first;
    size_t index_last;
    ParseCsvCell(line, index, &index_first, &index_last);
    for (int i = 0; i < 16; i++) {
        hllv[i] = (uint8_t)ParseUint64Cell(line, index);
    }
}
