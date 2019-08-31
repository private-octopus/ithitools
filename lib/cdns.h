/*
* Author: Christian Huitema
* Copyright (c) 2019, Private Octopus, Inc.
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
#ifndef CDNS_H
#define CDNS_H

#include <vector>

class cdns_qr_extended {
public:
    cdns_qr_extended();
    ~cdns_qr_extended();

    uint8_t* parse(uint8_t* in, uint8_t const* in_max, int* err);

    uint8_t* parse_map_item(uint8_t* in, uint8_t const* in_max, int64_t val, int* err);

    int question_index;
    int answer_index;
    int authority_index;
    int additional_index;
};

class cdns_query {
public:
    cdns_query();
    ~cdns_query();

    uint8_t* parse(uint8_t* in, uint8_t const* in_max, int* err);

    uint8_t* parse_map_item(uint8_t* in, uint8_t const* in_max, int64_t val, int* err);

    int time_offset_usec;
    int client_address_index;
    int client_port;
    int transaction_id;
    int query_signature_index;
    int client_hoplimit;
    int delay_useconds;
    int query_name_index;
    int query_size;
    int response_size;
    cdns_qr_extended q_extended;
    cdns_qr_extended r_extended;
};


class cdns_class_id {
public:
    cdns_class_id();
    ~cdns_class_id();

    uint8_t * parse(uint8_t* in, uint8_t const* in_max, int* err);

    uint8_t* parse_map_item(uint8_t* in, uint8_t const * in_max, int64_t val, int* err);

    int rr_type;
    int rr_class;
};

class cdns_query_signature {
public:
    cdns_query_signature();
    ~cdns_query_signature();

    uint8_t* parse(uint8_t* in, uint8_t const* in_max, int* err);

    uint8_t* parse_map_item(uint8_t* in, uint8_t const* in_max, int64_t val, int* err);

    int server_address_index;
    int server_port;
    int transport_flags;
    int qr_sig_flags;
    int query_opcode;
    int qr_dns_flags;
    int query_rcode;
    int query_classtype_index;
    int query_qd_count;
    int query_an_count;
    int query_ar_count;
    int query_ns_count;
    int edns_version;
    int udp_buf_size;
    int opt_rdata_index;
    int response_rcode;
};

class cdns_question {
public:
    cdns_question();
    ~cdns_question();

    uint8_t* parse(uint8_t* in, uint8_t const* in_max, int* err);

    uint8_t* parse_map_item(uint8_t* in, uint8_t const* in_max, int64_t val, int* err);

    int name_index;
    int classtype_index;
};

class cdns_rr_field {
public:
    cdns_rr_field();
    ~cdns_rr_field();

    uint8_t* parse(uint8_t* in, uint8_t const* in_max, int* err);

    uint8_t* parse_map_item(uint8_t* in, uint8_t const* in_max, int64_t val, int* err);

    int name_index;
    int classtype_index;
    int ttl;
    int rdata_index;
};

class cdns_rr_list{
public:
    cdns_rr_list();
    ~cdns_rr_list();

    uint8_t* parse(uint8_t* in, uint8_t const* in_max, int* err);

    std::vector<int> rr_index;
};


class cdnsBlockTables
{
public:
    cdnsBlockTables();

    ~cdnsBlockTables();

    uint8_t* parse(uint8_t* in, uint8_t const* in_max, int* err);

    uint8_t* parse_map_item(uint8_t* in, uint8_t const* in_max, int64_t val, int* err);

    std::vector<cbor_bytes> addresses;
    std::vector<cdns_class_id> class_ids;
    std::vector<cbor_bytes> name_rdata;
    std::vector<cdns_query_signature> q_sigs;
    std::vector<cdns_question> question_list;
    std::vector<cdns_rr_field> qrr;
    std::vector<cdns_rr_list> rr_list;
    std::vector<cdns_rr_field> rrs;
};




class cdnsBlock
{
public:
    cdnsBlock();

    ~cdnsBlock();

    uint8_t* parse(uint8_t* in, uint8_t const* in_max, int* err);

    uint8_t* parse_map_item(uint8_t* in, uint8_t const* in_max, int64_t val, int* err);

    cdnsBlockTables tables;
    std::vector<cdns_query> queries;

};

class cdns
{
public:
    cdns();

    ~cdns();

    bool open(char const* file_name, size_t buf_size);

    bool dump(char const* file_out);

private:
    FILE* F;
    uint8_t* buf;
    size_t buf_size;
    size_t buf_read;
    size_t buf_parsed;
    bool end_of_file;

    bool load_buffer();
    uint8_t* dump_preamble(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out);
    uint8_t* dump_block_parameters(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out);
    uint8_t* dump_block(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out);
    uint8_t* dump_block_properties(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out);
    uint8_t* dump_block_tables(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out); 
    uint8_t* dump_queries(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out);
    uint8_t* dump_query(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out);
    uint8_t* dump_class_types(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out);
    uint8_t* dump_class_type(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out);
    uint8_t* dump_qr_sigs(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out);
    uint8_t* dump_qr_sig(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out);

    uint8_t* dump_list(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, char const* indent, char const* list_name, int* err, FILE* F_out);
};

#endif

