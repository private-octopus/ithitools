#include "cdns.h"
#include "ipstats.h"
#include "DnsStats.h"
#include <math.h>

IPStatsRecord::IPStatsRecord() :
    ipaddr_length(0),
    query_volume(0),
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
    ipaddr_length += key->ipaddr_length;
    query_volume += key->query_volume;
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

bool IPStatsRecord::Write(FILE* F)
{
    return false;
}

IPStats::IPStats():
    IPF(NULL)
{
}

IPStats::~IPStats()
{
    if (this->IPF != NULL && this->IPF != stdout) {
        fclose(this->IPF);
    }
}

bool IPStats::SetOutputFile(char const* file_name)
{
    bool ret = true;

    if (this->IPF != NULL && this->IPF != stdout) {
        fclose(this->IPF);
    }
    if (strcmp(file_name, "-") == 0) {
        this->IPF = stdout;
    }
    else {
#ifdef _WINDOWS
        errno_t err = fopen_s(&this->IPF, file_name, "wt");
        if (err != 0) {
            if (this->IPF != NULL) {
                fclose(this->IPF);
                this->IPF = NULL;
            }
            ret = false;
        }
#else
        this->IPF = fopen(file_name, "wt");

        ret &= (this->IPF != NULL);
#endif
    }
    return ret;
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
#if 0
        for (size_t i = 0; i < cdns_ctx.block.queries.size(); i++) {
            SubmitCborPacket(&cdns_ctx, i);
        }
#endif
    }

    return ret;
}

#if 0
void IPStats::SubmitCborPacket(cdns* cdns_ctx, size_t packet_id)
{
    IPStatsRecord ipsr;
    cdns_query* query = &cdns_ctx->block.queries[packet_id];
    cdns_query_signature* q_sig = NULL; 
    size_t c_address_id = (size_t)query->client_address_index - cdns_ctx->index_offset;

    if (query->query_signature_index >= cdns_ctx->index_offset) {
        q_sig = &cdns_ctx->block.tables.q_sigs[(size_t)query->query_signature_index - cdns_ctx->index_offset];
    }
    /* TODO: this should really be IP entry, not a name! */
    NamePartEntry* ip_entry = AddNamePartEntry(&this->h_ip, (uint32_t)cdns_ctx->block.tables.addresses[c_address_id].l, cdns_ctx->block.tables.addresses[c_address_id].v);

    if (ip_entry != NULL) {
        ipsr.ip_addr = (char *)ip_entry->name_part_value;
    }

    ipsr.t_start_sec = (uint32_t)cdns_ctx->block.preamble.earliest_time_sec;

    if (cdns_ctx->is_old_version()) {
        ipsr.is_udp = (q_sig == NULL) || (q_sig->qr_transport_flags & 1) == 0;
    }
    else {
        ipsr.is_udp = (q_sig == NULL) || ((q_sig->qr_transport_flags >> 1)&0xF) == 0;
    }

    if (q_sig != NULL)
    {
        /* Some QSIG flags bits vary between RFC and draft, but bit 0 and bit 5 have the same meaning. */
        if ((q_sig->qr_sig_flags&0x01) != 0)
        {
            ipsr.query_count++;

            SubmitCborPacketQuery(cdns_ctx, query, q_sig, &ipsr);
        }
    }
}

void IPStats::SubmitCborPacketQuery(cdns* cdns_ctx, cdns_query* query, cdns_query_signature* q_sig, IPStatsRecord * ipsr)
{
    uint64_t query_time_usec = cdns_ctx->block.block_start_us + query->time_offset_usec;

    /* Check that all work is done here.. */
    if (q_sig->query_opcode == DNS_OPCODE_QUERY &&
        q_sig->query_rcode == DNS_RCODE_NOERROR &&
        query->query_name_index >= 0)
    {
        /* This works because parsing of OPT records sets the proper values for OPT fields */
        size_t nid = (size_t)query->query_name_index - cdns_ctx->index_offset;
        size_t addrid = (size_t)query->client_address_index - cdns_ctx->index_offset;
        uint32_t name_len = (uint32_t)cdns_ctx->block.tables.name_rdata[nid].l;
        uint8_t* name = cdns_ctx->block.tables.name_rdata[nid].v;
        uint8_t* tld = NULL;
        uint32_t tld_len = 0;
        uint8_t* sld = NULL;
        uint32_t sld_len = 0;
        uint32_t i = 0;

        while (i < name_len) {
            uint8_t l = name[i];

            if (l == 0)
            {
                /* end of parsing*/
                break;
            }
            else if (l > 0x3F || i + l + 1 > name_len)
            {
                /* Name compression, but we don't support that. */
                tld = NULL;
                tld_len = 0;
                sld = NULL;
                sld_len = 0;
                break;
            }
            else
            {
                i += 1;
                sld = tld;
                sld_len = tld_len;
                tld = &name[i];
                tld_len = l;
                i += l;
            }
        }
        /* Document SLD & TLD */
        if (tld_len > 0) {
            ipsr->TLD = this->AddNamePartEntry(&this->htld, tld_len, tld);
        }
        if (sld_len > 0) {
            ipsr->SLD = this->AddNamePartEntry(&this->hsld, sld_len, sld);
        }
        ipsr->Write(this->IPF);
    }
}

bool IPStats::LoadPcapFile(char const * fileName)
{
    bool ret = true;
#if 0
    pcap_reader reader;
    size_t nb_udp_dns = 0;
    uint64_t data_udp53 = 0;
    uint64_t data_tcp53 = 0;
    uint64_t data_tcp853 = 0;
    uint64_t data_tcp443 = 0;

    if (!reader.Open(fileName, NULL))
    {
        ret = false;
    }
    else
    {
        while (reader.ReadNext())
        {
            if (reader.tp_version == 17 &&
                (reader.tp_port1 == 53 || reader.tp_port2 == 53))
            {
                data_udp53 += (uint64_t)reader.tp_length - 8;

                if (!reader.is_fragment)
                {
                    my_bpftimeval ts;

                    ts.tv_sec = reader.frame_header.ts_sec;
                    ts.tv_usec = reader.frame_header.ts_usec;
                    SubmitPacket(reader.buffer + reader.tp_offset + 8,
                        reader.tp_length - 8, reader.ip_version, reader.buffer + reader.ip_offset, ts);
                    nb_udp_dns++;

                    if (target_number_dns_packets > 0 &&
                        nb_udp_dns >= target_number_dns_packets) {
                        /* Break when enough data captured */
                        break;
                    }
                }
            }
            else if (reader.tp_version == 6) {
                /* Do simple statistics on TCP traffic */
                size_t header_length32 = reader.buffer[reader.tp_offset + 12] >> 4;
                size_t tcp_payload = reader.tp_length - 4 * header_length32;
                bool is_port_853 = false;
                bool is_port_443 = false;

                if (reader.tp_port1 == 53 || reader.tp_port2 == 53) {
                    data_tcp53 += tcp_payload;
                }
                else if (reader.tp_port1 == 853 || reader.tp_port2 == 853) {
                    data_tcp853 += tcp_payload;
                    is_port_853 = true;
                    is_capture_dns_only = false;
                }
                else if (reader.tp_port1 == 443 || reader.tp_port2 == 443) {
                    data_tcp443 += tcp_payload;
                    is_port_443 = true;
                    is_capture_dns_only = false;
                }
                else {
                    is_capture_dns_only = false;
                }
                if (is_port_443 || is_port_853) {
                    uint8_t flags = reader.buffer[reader.tp_offset + 13] & 0x3F;
                    if (flags == 0x02) {
                        uint8_t * addr;
                        size_t addr_length;
                        GetSourceAddress(reader.ip_version, reader.buffer + reader.ip_offset,
                            &addr, &addr_length);
                        RegisterTcpSynByIp(addr, addr_length, is_port_853, is_port_443);
                    }
                }
            }
            else {
                is_capture_dns_only = false;
            }
        }
    }
#endif
    return ret;
}
#endif

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

void HyperLogLog::AddKey(const uint8_t* x, size_t l)
{
    uint64_t fnv64 = 0xcbf29ce484222325ull;
    const uint64_t fnv64_prime = 0x00000100000001B3;
    uint8_t* hash_buffer = (uint8_t*)&fnv64;
    int bucket_id = 0;
    /* Compute the FNV 64 bit hash */
    for (size_t i = 0; i < l; i++) {
        fnv64 ^= x[i];
        fnv64 *= fnv64_prime;
    }
    /* To reduce potential bias, compute bucket id as hash of all nibbles in FNV64 */
    for (size_t i = 0; i < 8; i++) {
        bucket_id ^= (int)((fnv64 >> (8 * i)) & 0xff);
    }
    bucket_id ^= (bucket_id >> 4);
    bucket_id &= 0x0f;

    /* compute the number of zeroes plus 1.
     * the number 1 is because Flajolet's original counted the 
     * position of the first 1, and the rest of the algorithm
     * depends on that.
     */
    uint8_t nb_zeroes = 1;
    uint8_t* y = hash_buffer;
    for (int j = 0; j < 7; j++) {
        int v = y[j];
        if (v == 0) {
            nb_zeroes += 8;
            continue;
        }
        else {
            if (v < 16) {
                nb_zeroes += 4;
            }
            else {
                v >>= 4;
            }
            if (v < 4) {
                nb_zeroes += 2;
            }
            else {
                v >>= 2;
            }
            if (v < 2) {
                nb_zeroes += 1;
            }
            break;
        }
    }
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
