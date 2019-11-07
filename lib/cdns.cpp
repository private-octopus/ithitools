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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ithiutil.h"
#include "cbor.h"
#include "cdns.h"

cdns::cdns():
    first_block_start_us(0),
    F(NULL),
    buf(NULL),
    buf_size(0),
    buf_read(0),
    buf_parsed(0),
    end_of_file(false),
    preamble_parsed(false),
    file_head_undef(false),
    block_list_undef(false),
    nb_blocks_present(0),
    nb_blocks_read(0)
{
}

cdns::~cdns()
{
    if (F != NULL) {
        fclose(F);
    }
    
    if (buf != NULL) {
        delete[] buf;
    }
}

bool cdns::open(char const* file_name)
{
    bool ret = false;

    if (F == NULL && buf == NULL) {

        F = ithi_file_open(file_name, "rb");
        if (F != NULL) {
            ret = load_entire_file();
        }
    }

    return ret;
}

bool cdns::dump(char const* file_out)
{
    FILE * F_out = ithi_file_open(file_out, "w");
    size_t out_size = 10 * buf_size;
    char* out_buf = new char[out_size];
    bool ret = (F_out != NULL && out_buf != NULL);

    if (ret) {
        int err = 0;
        char* p_out;
        int64_t val;
        uint8_t* in = buf;
        uint8_t* in_max = in + buf_read;
        int outer_type = CBOR_CLASS(*in);
        int is_undef = 0;

        in = cbor_get_number(in, in_max, &val);

        if (in == NULL || outer_type != CBOR_T_ARRAY) {
            fprintf(F_out, "Error, cannot parse the first bytes, type %d.\n", outer_type);
            err = CBOR_MALFORMED_VALUE;
            in = NULL;
            ret = false;
        }
        else {
            int rank = 0;

            fprintf(F_out, "[\n");
            if (val == CBOR_END_OF_ARRAY) {
                is_undef = 1;
                val = 0xffffffff;
            }

            if (in < in_max && *in != 0xff) {
                p_out = out_buf;
                in = cbor_to_text(in, in_max, &p_out, out_buf + out_size, &err);
                fprintf(F_out, "-- File type:\n    %s,\n", out_buf);
                val--;
            }

            if (in != NULL && in < in_max) {
                /*
                p_out = out_buf;
                in = cbor_to_text(in, in_max, &p_out, out_buf + out_size, &err);
                fprintf(F_out, "-- File preamble\n    %s\n", out_buf);*/
                in = dump_preamble(in, in_max, out_buf, out_buf + out_size, & err, F_out);
                fprintf(F_out, ",\n");
                val--;
            }

            while (val > 0 && in != NULL && in < in_max && *in != 0xff) {
                rank++;
                fprintf(F_out, "-- Block %d:\n", rank);
                in = dump_block(in, in_max, out_buf, out_buf + out_size, &err, F_out);
                val--;
            }

            if (in != NULL && in < in_max && *in != 0xff) {
                if (is_undef) {
                    in++;
                }
                else {
                    fprintf(F_out, "Error, end of array mark unexpected.\n");
                    err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
            }

            if (in != NULL) {
                fprintf(F_out, "\n]\n-- Processed=%d\n-- Err = %d\n", (int)(in - buf), err);
            }
        }
    }

    if (F_out != NULL){
        fclose(F_out);
    }
    if (out_buf != NULL) {
        delete[] out_buf;
    }
    return ret;
}

bool cdns::open_block(int* err)
{
    bool ret = true;

    *err = 0;
    if (!preamble_parsed) {
        ret = read_preamble(err);
    }

    if (ret && nb_blocks_read >= nb_blocks_present) {
        *err = CBOR_END_OF_ARRAY;
        ret = false;
    }

    if (ret){
        uint8_t* in = buf + buf_parsed;
        uint8_t* in_max = buf + buf_read;

        if (*in == CBOR_END_MARK) {
            in++;
            buf_parsed = in - buf;
            if (block_list_undef) {
                nb_blocks_present = nb_blocks_read;
                *err = CBOR_END_OF_ARRAY;
            }
            else {
                *err = CBOR_MALFORMED_VALUE;
            }
            ret = false;
        }
        else {
            uint8_t* old_in = in;

            in = block.parse(old_in, in_max, err);

            if (in != NULL) {
                nb_blocks_read++;
                buf_parsed = in - buf;
            }
            else {
                char out_buf[1024];
                int err = 0;

                fprintf(stderr, "\nBlock parsing error at position %lld:\n", (unsigned long long)(old_in - buf));

                (void)dump_block_properties(old_in, in_max, out_buf, out_buf + sizeof(out_buf), &err, stderr);

                fprintf(stderr, "\n");

                ret = false;
            }
        }
    }

    if (ret && first_block_start_us == 0) {
        first_block_start_us = block.block_start_us;
    }

    return ret;
}

int cdns::get_dns_flags(int q_dns_flags, bool is_response)
{
    int flags = 0;

    if (is_response) {
        flags = (q_dns_flags >> 8) & 0x7E;
    }
    else {
        flags = q_dns_flags & 0x7C;
    }

    return flags;
}

int cdns::get_edns_flags(int q_dns_flags)
{
    return (q_dns_flags << 8) & (1<<15);
}

bool cdns::load_entire_file()
{
    bool ret = true;
    do {
        if (buf == NULL) {
            /* Initialize to a small size, so we make sure that the reallocation code is well tested */
            buf_size = 0x20000;
            buf = new uint8_t[buf_size];
            if (buf == NULL) {
                ret = false;
                buf_size = 0;
            }
        } else if (buf_read > 0 && buf_read == buf_size) {
            uint8_t* new_buf;
            size_t new_size = 4 * buf_size;
            new_buf = new uint8_t[new_size];
            if (new_buf == NULL) {
                ret = false;
            }
            else {
                memcpy(new_buf, buf, buf_read);
                delete[] buf;
                buf = new_buf;
                buf_size = new_size;
            }
        }
        else {
            size_t asked = buf_size - buf_read;
            size_t n_bytes = fread(buf + buf_read, 1, asked, F);
            end_of_file = (n_bytes < asked);
            buf_read += n_bytes;
        }
    } while (ret && !end_of_file);

    return (ret);
}


bool cdns::read_preamble(int * err)
{
    bool ret = true;
    int64_t val;
    uint8_t * in = buf;
    uint8_t * in_max = in + buf_read;
    int outer_type = CBOR_CLASS(*in);
    in = cbor_get_number(in, in_max, &val);

    if (preamble_parsed) {
        return true;
    }

    if (in == NULL || outer_type != CBOR_T_ARRAY) {
        *err = CBOR_MALFORMED_VALUE;
        in = NULL;
        ret = false;
    }
    else {
        if (val == CBOR_END_OF_ARRAY) {
            file_head_undef = 1;
            val = 0xffffffff;
        }
        else if (val < 3) {
            *err = CBOR_MALFORMED_VALUE;
            in = NULL;
            ret = false;
        }

        if (ret) {
            if (in != NULL && in < in_max && *in != 0xff) {
                /* Skip file type -- to do, check for value == CDNS */
                in = cbor_skip(in, in_max, err);
                val--;
            }

            if (in != NULL && in < in_max && *in != 0xff) {
                /* Skip preamble -- to do, parse and store values  */
                in = cbor_skip(in, in_max, err);
                val--;
            }

            if (in != NULL && in < in_max && *in != 0xff) {
                int64_t nb_blocks;
                int blocks_type = CBOR_CLASS(*in);
                in = cbor_get_number(in, in_max, &nb_blocks);

                if (in == NULL || blocks_type != CBOR_T_ARRAY) {
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                    ret = false;
                }
                else {
                    if (nb_blocks == CBOR_END_OF_ARRAY) {
                        block_list_undef = 1;
                        nb_blocks_present = 0xffffffff;
                    }
                    else {
                        nb_blocks_present = val;
                    }
                    buf_parsed = in - buf;
                }
            }
        }
    }

    preamble_parsed = true;

    if (in == NULL || !ret) {
        buf_parsed = buf_read;
        nb_blocks_present = 0;
        ret = false;
    }

    return ret;
}

uint8_t* cdns::dump_preamble(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out)
{
    char* p_out;
    int64_t val;
    int outer_type = CBOR_CLASS(*in);
    int is_undef = 0;

    in = cbor_get_number(in, in_max, &val);

    if (in == NULL || outer_type != CBOR_T_MAP) {
        fprintf(F_out, "Error, cannot parse the first bytes of preamble, type %d.\n", outer_type);
        *err = CBOR_MALFORMED_VALUE;
        in = NULL;
    }
    else {
        int rank = 0;

        fprintf(F_out, "-- Preamble:\n    [\n");
        if (val == CBOR_END_OF_ARRAY) {
            is_undef = 1;
            val = 0xffffffff;
        }

        while (val > 0 && in != NULL && in < in_max) {
            if (*in == 0xff) {
                fprintf(F_out, "\n        --end of array");
                if (is_undef) {
                    in++;
                }
                else {
                    fprintf(F_out, "Error, end of array mark unexpected.\n");
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
                break;
            }
            else {
                /* There should be two elements for each map item */
                int inner_type = CBOR_CLASS(*in);
                int64_t inner_val;

                in = cbor_get_number(in, in_max, &inner_val);

                if (inner_type != CBOR_T_UINT) {
                    fprintf(F_out, "                Unexpected type: %d(%d)\n", inner_type, (int)inner_val);
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
                else {
                    if (rank != 0) {
                        fprintf(F_out, ",\n");
                    }
                    rank++;
                    if (inner_val == 0) {
                        fprintf(F_out, "        --major-format-version\n");
                    }else if (inner_val == 1) {
                        fprintf(F_out, "        --minor-format-version\n");
                    }
                    else if (inner_val == 2) {
                        fprintf(F_out, "        --private-version\n");
                    }
                    else if (inner_val == 3) {
                        fprintf(F_out, "        --block-parameters\n");
                    }
                    else if (inner_val == 4) {
                        fprintf(F_out, "        --generator-id\n"); /* V0.5 only? */
                    }
                    else if (inner_val == 5) {
                        fprintf(F_out, "        --host-id\n"); /* v0.5 only? */
                    }
                    fprintf(F_out, "        %d, ", (int)inner_val);
                    if (inner_val == 3) {
                        in = dump_block_parameters(in, in_max, out_buf, out_max, err, F_out);
                    }
                    else {
                        p_out = out_buf;
                        in = cbor_to_text(in, in_max, &p_out, out_max, err);
                        fwrite(out_buf, 1, p_out - out_buf, F_out);
                    }
                    val--;
                }
            }
        }

        if (in != NULL) {
            fprintf(F_out, "\n    ]");
        }
    }

    return in;
}

uint8_t* cdns::dump_block_parameters(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out)
{
    char* p_out;
    int64_t val;
    int outer_type = CBOR_CLASS(*in);
    int is_undef = 0;

    in = cbor_get_number(in, in_max, &val);

    if (in == NULL || outer_type != CBOR_T_MAP) {
        fprintf(F_out, "       Error, cannot parse the first bytes of block parameters, type %d.\n", outer_type);
        *err = CBOR_MALFORMED_VALUE;
        in = NULL;
    }
    else {
        int rank = 0;
        fprintf(F_out, "[\n");
        if (val == CBOR_END_OF_ARRAY) {
            is_undef = 1;
            val = 0xffffffff;
        }

        while (val > 0 && in != NULL && in < in_max) {
            if (*in == 0xff) {
                fprintf(F_out, "\n            --end of array");
                if (is_undef) {
                    in++;
                }
                else {
                    fprintf(F_out, "            Error, end of array mark unexpected.\n");
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
                break;
            }
            else {
                /* There should be two elements for each map item */
                int inner_type = CBOR_CLASS(*in);
                int64_t inner_val;

                in = cbor_get_number(in, in_max, &inner_val);
                if (inner_type != CBOR_T_UINT) {
                    fprintf(F_out, "        Unexpected type: %d(%d)\n", inner_type, (int)inner_val);
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
                else {
                    if (rank != 0) {
                        fprintf(F_out, ",\n");
                    }
                    rank++;
                    {
                        /* Type definitions copies fro source code of compactor */
                        p_out = out_buf;
                        if (inner_val == 0) {
                            fprintf(F_out, "            --query-timeout\n");
                        }
                        else if (inner_val == 1) {
                            fprintf(F_out, "            --skew-timeout\n");
                        }
                        else if (inner_val == 2) {
                            fprintf(F_out, "            --snaplen\n");
                        }
                        else if (inner_val == 3) {
                            fprintf(F_out, "            --promisc\n");
                        }
                        else if (inner_val == 4) {
                            fprintf(F_out, "            --interfaces\n"); /* V0.5 only? */
                        }
                        else if (inner_val == 5) {
                            fprintf(F_out, "            --server-addresses\n"); /* v0.5 only? */
                        }
                        else if (inner_val == 6) {
                            fprintf(F_out, "            --vlan-ids\n"); /* v0.5 only? */
                        }
                        else if (inner_val == 7) {
                            fprintf(F_out, "            --filter\n"); /* v0.5 only? */
                        }
                        else if (inner_val == 8) {
                            fprintf(F_out, "            --query-options\n"); /* v0.5 only? */
                        }
                        else if (inner_val == 9) {
                            fprintf(F_out, "            --response-options\n"); /* v0.5 only? */
                        }
                        else if (inner_val == 10) {
                            fprintf(F_out, "            --accept-rr-types\n"); /* v0.5 only? */
                        }
                        else if (inner_val == 11) {
                            fprintf(F_out, "            --ignore-rr-types\n"); /* v0.5 only? */
                        }
                        else if (inner_val == 12) {
                            fprintf(F_out, "            --max-block-qr-items\n"); /* v0.5 only? */
                        }
                        fprintf(F_out, "            %d,", (int)inner_val);
                        in = cbor_to_text(in, in_max, &p_out, out_max, err);
                        fwrite(out_buf, 1, p_out - out_buf, F_out);
                    }
                    val--;
                }
            }
        }

        if (in != NULL) {
            fprintf(F_out, "\n        ]");
        }
    }

    return in;
}

uint8_t* cdns::dump_block(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out)
{
    char* p_out;
    int64_t val;
    int outer_type = CBOR_CLASS(*in);
    int is_undef = 0;

    in = cbor_get_number(in, in_max, &val);

    if (in == NULL || outer_type != CBOR_T_ARRAY) {
        fprintf(F_out, "   Error, cannot parse the first bytes of block, type=%d.\n", outer_type);
        *err = CBOR_MALFORMED_VALUE;
        in = NULL;
    }
    else {
        int rank = 0;

        fprintf(F_out, "    [\n");
        if (val == CBOR_END_OF_ARRAY) {
            is_undef = 1;
            val = 0xffffffff;
        }

        while (val > 0 && in != NULL && in < in_max) {
            if (*in == 0xff) {
                fprintf(F_out, "        --end of array");
                if (is_undef) {
                    in++;
                }
                else {
                    fprintf(F_out, "        Error, end of array mark unexpected.\n");
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
                break;
            }
            else {
                int inner_type = CBOR_CLASS(*in);

                if (inner_type == CBOR_T_MAP) {
                    /* Records are held in a map */
                    in = dump_block_properties(in, in_max, out_buf, out_max, err, F_out);
                }
                else {
                    p_out = out_buf;
                    rank++;
                    fprintf(F_out, "        -- Property %d:\n    ", rank);
                    in = cbor_to_text(in, in_max, &p_out, out_max, err);
                    fwrite(out_buf, 1, p_out - out_buf, F_out);
                    fprintf(F_out, "\n");
                }
                val--;
            }
        }

        if (in != NULL) {
            fprintf(F_out, "\n    ]\n");
        }

    }

    return in;
}

uint8_t* cdns::dump_block_properties(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out)
{
    char* p_out;
    int64_t val;
    int outer_type = CBOR_CLASS(*in);
    int is_undef = 0;

    in = cbor_get_number(in, in_max, &val);

    if (in == NULL || outer_type != CBOR_T_MAP) {
        fprintf(F_out, "   Error, cannot parse the first bytes of properties, type: %d.\n", outer_type);
        *err = CBOR_MALFORMED_VALUE;
        in = NULL;
    }
    else {
        int rank = 0;
        fprintf(F_out, "        [\n");
        if (val == CBOR_END_OF_ARRAY) {
            is_undef = 1;
            val = 0xffffffff;
        }

        while (val > 0 && in != NULL && in < in_max ) {
            if (*in == 0xff) {
                fprintf(F_out, "\n            --end of array");
                if (is_undef) {
                    in++;
                }
                else {
                    fprintf(F_out, "            Error, end of array mark unexpected.\n");
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
                break;
            }
            else {
                /* There should be two elements for each map item */
                int inner_type = CBOR_CLASS(*in);
                int64_t inner_val;

                in = cbor_get_number(in, in_max, &inner_val);
                if (inner_type != CBOR_T_UINT) {
                    fprintf(F_out, "            Unexpected type: %d(%d)\n", inner_type, (int)inner_val);
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
                else {
                    if (rank != 0) {
                        fprintf(F_out, ",\n");
                    }
                    rank++;
                    fprintf(F_out, "            %d, ", (int)inner_val);
                    if (inner_val == 2) {
                        in = dump_block_tables(in, in_max, out_buf, out_max, err, F_out);
                    }
                    else if (inner_val == 3) {
                        fprintf(F_out, "[\n");
                        in = dump_queries(in, in_max, out_buf, out_max,  err, F_out);
                        fprintf(F_out, "            ]");
                    }
                    else if (inner_val == 4) {
                        fprintf(F_out, "[\n");
                        in = dump_list(in, in_max, out_buf, out_max, "                ", "address-event-counts", err, F_out);
                        fprintf(F_out, "            ]");
                    }
                    else {
                        p_out = out_buf;
                        in = cbor_to_text(in, in_max, &p_out, out_max, err);
                        fwrite(out_buf, 1, p_out - out_buf, F_out);
                    }
                    val--;
                }
            }
        }

        if (in != NULL) {
            fprintf(F_out, "\n        ]\n");
        }
    }

    return in;
}

uint8_t* cdns::dump_block_tables(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out)
{
    char* p_out;
    int64_t val;
    int outer_type = CBOR_CLASS(*in);
    int is_undef = 0;

    in = cbor_get_number(in, in_max, &val);

    if (in == NULL || outer_type != CBOR_T_MAP) {
        fprintf(F_out, "       Error, cannot parse the first bytes of block tables, type = %d.\n",
            outer_type);
        *err = CBOR_MALFORMED_VALUE;
        in = NULL;
    }
    else {
        int rank = 0;
        fprintf(F_out, "[\n");
        if (val == CBOR_END_OF_ARRAY) {
            is_undef = 1;
            val = 0xffffffff;
        }

        while (val > 0 && in != NULL && in < in_max) {
            if (*in == 0xff) {
                fprintf(F_out, "\n                --end of array");
                if (is_undef) {
                    in++;
                }
                else {
                    fprintf(F_out, "                Error, end of array mark unexpected.\n");
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
                break;
            }
            else {
                /* There should be two elements for each map item */
                int inner_type = CBOR_CLASS(*in);
                int64_t inner_val;

                in = cbor_get_number(in, in_max, &inner_val);
                if (inner_type != CBOR_T_UINT) {
                    fprintf(F_out, "                Unexpected type: %d(%d)\n", inner_type, (int)inner_val);
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
                else {
                    if (rank != 0) {
                        fprintf(F_out, ",\n");
                    }
                    rank++;
                    fprintf(F_out, "                %d, ", (int)inner_val);
                    if (inner_val == 0) {
                        fprintf(F_out, "[\n");
                        in = dump_list(in, in_max, out_buf, out_max, "                    ", "adresses", err, F_out);
                        fprintf(F_out, "                ]");
                    }
                    else if (inner_val == 1) {
                        fprintf(F_out, "[\n");
                        in = dump_class_types(in, in_max, out_buf, out_max, err, F_out);
                        fprintf(F_out, "                ]");
                    }
                    else if (inner_val == 2) {
                        fprintf(F_out, "[\n");
                        in = dump_list(in, in_max, out_buf, out_max, "                    ", "name-rdata", err, F_out);
                        fprintf(F_out, "                ]");
                    }
                    else if (inner_val == 3) {
                        fprintf(F_out, "[\n");
                        in = dump_qr_sigs(in, in_max, out_buf, out_max, err, F_out);
                        fprintf(F_out, "                ]");
                    }
                    else if (inner_val == 4) {
                        fprintf(F_out, "[\n");
                        in = dump_list(in, in_max, out_buf, out_max, "                    ", "q-lists", err, F_out);
                        fprintf(F_out, "                ]");
                    }
                    else if (inner_val == 5) {
                        fprintf(F_out, "[\n");
                        in = dump_list(in, in_max, out_buf, out_max, "                    ", "qrr", err, F_out);
                        fprintf(F_out, "                ]");
                    }
                    else if (inner_val == 6) {
                        fprintf(F_out, "[\n");
                        in = dump_list(in, in_max, out_buf, out_max, "                    ", "rr-lists", err, F_out);
                        fprintf(F_out, "                ]");
                    }
                    else if (inner_val == 7) {
                        fprintf(F_out, "[\n");
                        in = dump_list(in, in_max, out_buf, out_max, "                    ", "rrs", err, F_out);
                        fprintf(F_out, "                ]");
                    }
                    else {
                        p_out = out_buf;
                        in = cbor_to_text(in, in_max, &p_out, out_max, err);
                        fwrite(out_buf, 1, p_out - out_buf, F_out);
                    }
                    val--;
                }
            }
        }

        if (in != NULL) {
            fprintf(F_out, "\n            ]");
        }
    }

    return in;
}

uint8_t* cdns::dump_queries(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out)
{
    int64_t val;
    int outer_type = CBOR_CLASS(*in);
    int is_undef = 0;

    in = cbor_get_number(in, in_max, &val);

    if (in == NULL || outer_type != CBOR_T_ARRAY) {
        fprintf(F_out, "                Error, cannot parse the first bytes of queries, type= %d.\n",
            outer_type);
        *err = CBOR_MALFORMED_VALUE;
        in = NULL;
    }
    else {
        int rank = 0;
        if (val == CBOR_END_OF_ARRAY) {
            is_undef = 1;
            val = 0xffffffff;
        }

        while (rank < val && in != NULL && in < in_max) {
            if (*in == 0xff) {
                fprintf(F_out, "                --end of array\n");
                if (is_undef) {
                    in++;
                }
                else {
                    fprintf(F_out, "                Error, end of array mark unexpected.\n");
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
                break;
            }
            rank++;
            if (rank <= 10) {
                if (rank != 1) {
                    fprintf(F_out, ",\n");
                }
                /* One item per line, only print the first 10 items */
                in = dump_query(in, in_max, out_buf, out_max, err, F_out);
            }
            else {
                in = cbor_skip(in, in_max, err);
            }
        }

        if (in != NULL) {
            if (rank > 10) {
                fprintf(F_out, ",\n                ...\n");
            }
            fprintf(F_out, "                -- found %d queries\n", rank);
        }
    }

    return in;
}

uint8_t* cdns::dump_query(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out)
{
    char* p_out;
    int64_t val;
    int outer_type = CBOR_CLASS(*in);
    int is_undef = 0;

    in = cbor_get_number(in, in_max, &val);

    if (in == NULL || outer_type != CBOR_T_MAP) {
        fprintf(F_out, "                Error, cannot parse the first bytes of query, type: %d.\n", outer_type);
        *err = CBOR_MALFORMED_VALUE;
        in = NULL;
    }
    else {
        int rank = 0;
        fprintf(F_out, "                [\n");
        if (val == CBOR_END_OF_ARRAY) {
            is_undef = 1;
            val = 0xffffffff;
        }

        while (val > 0 && in != NULL && in < in_max) {
            if (*in == 0xff) {
                fprintf(F_out, "\n                    --end of array");
                if (is_undef) {
                    in++;
                }
                else {
                    fprintf(F_out, "                    Error, end of array mark unexpected.\n");
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
                break;
            }
            else {
                /* There should be two elements for each map item */
                int inner_type = CBOR_CLASS(*in);
                int64_t inner_val;

                in = cbor_get_number(in, in_max, &inner_val);
                if (inner_type != CBOR_T_UINT) {
                    fprintf(F_out, "                Unexpected type: %d(%d)\n", inner_type, (int)inner_val);
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
                else {
                    if (rank != 0) {
                        fprintf(F_out, ",\n");
                    }
                    rank++;
                    if (inner_val == 0) {
                        fprintf(F_out, "                    --time_useconds,\n");
                    } else if (inner_val == 1) {
                        fprintf(F_out, "                    --time_pseconds,\n");
                    }
                    else if (inner_val == 2) {
                        fprintf(F_out, "                    --client_address_index,\n");
                    }
                    else if (inner_val == 3) {
                        fprintf(F_out, "                    --client_port,\n");
                    }
                    else if (inner_val == 4) {
                        fprintf(F_out, "                    --transaction_id,\n");
                    }
                    else if (inner_val == 5) {
                        fprintf(F_out, "                    --query_signature_index,\n");
                    }
                    else if (inner_val == 6) {
                        fprintf(F_out, "                    --client_hoplimit,\n");
                    }
                    else if (inner_val == 7) {
                        fprintf(F_out, "                    --delay_useconds,\n");
                    }
                    else if (inner_val == 8) {
                        fprintf(F_out, "                    --delay_pseconds,\n");
                    }
                    else if (inner_val == 9) {
                        fprintf(F_out, "                    --query_name_index,\n");
                    }
                    else if (inner_val == 10) {
                        fprintf(F_out, "                    --query_size,\n");
                    }
                    else if (inner_val == 11) {
                        fprintf(F_out, "                    --response_size,\n");
                    }
                    else if (inner_val == 12) {
                        fprintf(F_out, "                    --query_extended,\n");
                    }
                    else if (inner_val == 13) {
                        fprintf(F_out, "                    --response_extended,\n");
                    }

                    fprintf(F_out, "                    %d, ", (int)inner_val);
                    p_out = out_buf;
                    in = cbor_to_text(in, in_max, &p_out, out_max, err);
                    fwrite(out_buf, 1, p_out - out_buf, F_out);
                    val--;
                }
            }
        }

        if (in != NULL) {
            fprintf(F_out, "\n                ]");
        }
    }

    return in;
}

uint8_t* cdns::dump_class_types(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out)
{
    int64_t val;
    int outer_type = CBOR_CLASS(*in);
    int is_undef = 0;

    in = cbor_get_number(in, in_max, &val);

    if (in == NULL || outer_type != CBOR_T_ARRAY) {
        fprintf(F_out, "                Error, cannot parse the first bytes of class-types, type= %d.\n",
            outer_type);
        *err = CBOR_MALFORMED_VALUE;
        in = NULL;
    }
    else {
        int rank = 0;
        if (val == CBOR_END_OF_ARRAY) {
            is_undef = 1;
            val = 0xffffffff;
        }

        while (rank < val && in != NULL && in < in_max) {
            if (*in == 0xff) {
                fprintf(F_out, "                --end of array\n");
                if (is_undef) {
                    in++;
                }
                else {
                    fprintf(F_out, "                Error, end of array mark unexpected.\n");
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
                break;
            }
            rank++;
            if (rank <= 10) {
                if (rank != 1) {
                    fprintf(F_out, ",\n");
                }
                /* One item per line, only print the first 10 items */
                in = dump_class_type(in, in_max, out_buf, out_max, err, F_out);
            }
            else {
                in = cbor_skip(in, in_max, err);
            }
        }

        if (in != NULL) {
            if (rank > 10) {
                fprintf(F_out, ",\n                    ...\n");
            }
            fprintf(F_out, "                    -- found %d class-types\n", rank);
        }
    }

    return in;
}

uint8_t* cdns::dump_class_type(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out)
{
    char* p_out;
    int64_t val;
    int outer_type = CBOR_CLASS(*in);
    int is_undef = 0;

    in = cbor_get_number(in, in_max, &val);

    if (in == NULL || outer_type != CBOR_T_MAP) {
        fprintf(F_out, "                    Error, cannot parse the first bytes of class-type, type: %d.\n", outer_type);
        *err = CBOR_MALFORMED_VALUE;
        in = NULL;
    }
    else {
        int rank = 0;
        fprintf(F_out, "                    [\n");
        if (val == CBOR_END_OF_ARRAY) {
            is_undef = 1;
            val = 0xffffffff;
        }

        while (val > 0 && in != NULL && in < in_max) {
            if (*in == 0xff) {
                fprintf(F_out, "\n                        --end of array");
                if (is_undef) {
                    in++;
                }
                else {
                    fprintf(F_out, "                        Error, end of array mark unexpected.\n");
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
                break;
            }
            else {
                /* There should be two elements for each map item */
                int inner_type = CBOR_CLASS(*in);
                int64_t inner_val;

                in = cbor_get_number(in, in_max, &inner_val);
                if (inner_type != CBOR_T_UINT) {
                    fprintf(F_out, "                    Unexpected type: %d(%d)\n", inner_type, (int)inner_val);
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
                else {
                    if (rank != 0) {
                        fprintf(F_out, ",\n");
                    }
                    rank++;
                    if (inner_val == 0) {
                        fprintf(F_out, "                        --type-id,\n");
                    }
                    else if (inner_val == 1) {
                        fprintf(F_out, "                        --class-id,\n");
                    }

                    fprintf(F_out, "                        %d, ", (int)inner_val);
                    p_out = out_buf;
                    in = cbor_to_text(in, in_max, &p_out, out_max, err);
                    fwrite(out_buf, 1, p_out - out_buf, F_out);
                    val--;
                }
            }
        }

        if (in != NULL) {
            fprintf(F_out, "]");
        }
    }

    return in;
}

uint8_t* cdns::dump_qr_sigs(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out)
{
    int64_t val;
    int outer_type = CBOR_CLASS(*in);
    int is_undef = 0;

    in = cbor_get_number(in, in_max, &val);

    if (in == NULL || outer_type != CBOR_T_ARRAY) {
        fprintf(F_out, "                Error, cannot parse the first bytes of qr-sigs, type= %d.\n",
            outer_type);
        *err = CBOR_MALFORMED_VALUE;
        in = NULL;
    }
    else {
        int rank = 0;
        if (val == CBOR_END_OF_ARRAY) {
            is_undef = 1;
            val = 0xffffffff;
        }

        while (rank < val && in != NULL && in < in_max) {
            if (*in == 0xff) {
                fprintf(F_out, "                --end of array\n");
                if (is_undef) {
                    in++;
                }
                else {
                    fprintf(F_out, "                Error, end of array mark unexpected.\n");
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
                break;
            }
            rank++;
            if (rank <= 10) {
                if (rank != 1) {
                    fprintf(F_out, ",\n");
                }
                /* One item per line, only print the first 10 items */
                in = dump_qr_sig(in, in_max, out_buf, out_max, err, F_out);
            }
            else {
                in = cbor_skip(in, in_max, err);
            }
        }

        if (in != NULL) {
            if (rank > 10) {
                fprintf(F_out, ",\n                    ...\n");
            }
            fprintf(F_out, "                    -- found %d qr-sigs\n", rank);
        }
    }

    return in;
}

uint8_t* cdns::dump_qr_sig(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out)
{
    char* p_out;
    int64_t val;
    int outer_type = CBOR_CLASS(*in);
    int is_undef = 0;

    in = cbor_get_number(in, in_max, &val);

    if (in == NULL || outer_type != CBOR_T_MAP) {
        fprintf(F_out, "                    Error, cannot parse the first bytes of qr-sig, type: %d.\n", outer_type);
        *err = CBOR_MALFORMED_VALUE;
        in = NULL;
    }
    else {
        int rank = 0;
        fprintf(F_out, "                    [\n");
        if (val == CBOR_END_OF_ARRAY) {
            is_undef = 1;
            val = 0xffffffff;
        }

        while (val > 0 && in != NULL && in < in_max) {
            if (*in == 0xff) {
                fprintf(F_out, "\n                        --end of array");
                if (is_undef) {
                    in++;
                }
                else {
                    fprintf(F_out, "                        Error, end of array mark unexpected.\n");
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
                break;
            }
            else {
                /* There should be two elements for each map item */
                int inner_type = CBOR_CLASS(*in);
                int64_t inner_val;

                in = cbor_get_number(in, in_max, &inner_val);
                if (inner_type != CBOR_T_UINT) {
                    fprintf(F_out, "                    Unexpected type: %d(%d)\n", inner_type, (int)inner_val);
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
                else {
                    if (rank != 0) {
                        fprintf(F_out, ",\n");
                    }
                    rank++;
                    if (inner_val == 0) {
                        fprintf(F_out, "                        --server_address_index,\n");
                    }
                    else if (inner_val == 1) {
                        fprintf(F_out, "                        --server_port,\n");
                    }
                    else if (inner_val == 2) {
                        fprintf(F_out, "                        --transport_flags,\n");
                    }
                    else if (inner_val == 3) {
                        fprintf(F_out, "                        --qr_sig_flags,\n");
                    }
                    else if (inner_val == 4) {
                        fprintf(F_out, "                        --query_opcode,\n");
                    }
                    else if (inner_val == 5) {
                        fprintf(F_out, "                        --qr_dns_flags,\n");
                    }
                    else if (inner_val == 6) {
                        fprintf(F_out, "                        --query_rcode,\n");
                    }
                    else if (inner_val == 7) {
                        fprintf(F_out, "                        --query_classtype_index,\n");
                    }
                    else if (inner_val == 8) {
                        fprintf(F_out, "                        --query_qd_count,\n");
                    }
                    else if (inner_val == 9) {
                        fprintf(F_out, "                        --query_an_count,\n");
                    }
                    else if (inner_val == 10) {
                        fprintf(F_out, "                        --query_ar_count,\n");
                    }
                    else if (inner_val == 11) {
                        fprintf(F_out, "                        --query_ns_count,\n");
                    }
                    else if (inner_val == 12) {
                        fprintf(F_out, "                        --edns_version,\n");
                    }
                    else if (inner_val == 13) {
                        fprintf(F_out, "                        --udp_buf_size,\n");
                    }
                    else if (inner_val == 14) {
                        fprintf(F_out, "                        --opt_rdata_index,\n");
                    }
                    else if (inner_val == 15) {
                        fprintf(F_out, "                        --response_rcode,\n");
                    }

                    fprintf(F_out, "                        %d, ", (int)inner_val);
                    p_out = out_buf;
                    in = cbor_to_text(in, in_max, &p_out, out_max, err);
                    fwrite(out_buf, 1, p_out - out_buf, F_out);
                    val--;
                }
            }
        }

        if (in != NULL) {
            fprintf(F_out, "]");
        }
    }

    return in;
}

uint8_t* cdns::dump_list(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, char const * indent, char const * list_name, int* err, FILE* F_out)
{
    char* p_out;
    int64_t val;
    int outer_type = CBOR_CLASS(*in);
    int is_undef = 0;

    in = cbor_get_number(in, in_max, &val);

    if (in == NULL) {
        fprintf(F_out, "%sError, cannot parse the first bytes of %s, type= %d.\n", 
            indent, list_name, outer_type);
        *err = CBOR_MALFORMED_VALUE;
        in = NULL;
    }
    else {
        int rank = 0;
        if (val == CBOR_END_OF_ARRAY) {
            is_undef = 1;
            val = 0xffffffff;
        }

        while (rank < val && in != NULL && in < in_max) {
            if (*in == 0xff) {
                fprintf(F_out, "%s--end of array\n", indent);
                if (is_undef) {
                    in++;
                }
                else {
                    fprintf(F_out, "%sError, end of array mark unexpected.\n", indent);
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
                break;
            }
            rank++;
            if (rank <= 10) {
                /* One item per line, only print the first 10 items */
                fprintf(F_out, "%s", indent);
                p_out = out_buf;
                in = cbor_to_text(in, in_max, &p_out, out_max, err);
                fwrite(out_buf, 1, p_out - out_buf, F_out);
                fprintf(F_out, ",");
                fprintf(F_out, "\n");
            }
            else {
                in = cbor_skip(in, in_max, err);
            }
        }

        if (in != NULL) {
            if (rank > 10) {
                fprintf(F_out, "%s...\n", indent);
            }
            fprintf(F_out, "%s-- found %d %s\n", indent, rank, list_name);
        }
    }

    return in;
}

cdnsBlock::cdnsBlock():
    is_filled(false),
    block_start_us(0)
{
}

cdnsBlock::~cdnsBlock()
{
}

uint8_t * cdnsBlock::parse(uint8_t* in, uint8_t const* in_max, int* err)
{
    /* Records are held in a map */
    clear();
    is_filled = 1;
    in = cbor_map_parse(in, in_max, this, err);

    if (preamble.is_filled) {
        block_start_us = preamble.earliest_time_sec;
        block_start_us *= 1000000;
        block_start_us += preamble.earliest_time_usec;
    }

    return in;
}

uint8_t* cdnsBlock::parse_map_item(uint8_t* in, uint8_t const* in_max, int64_t val, int* err)
{
    switch (val) {
    case 0: /* Block preamble */
        in = preamble.parse(in, in_max, err);
        break;
    case 1: /* Block statistics */
        in = statistics.parse(in, in_max, err);
        break;
    case 2: /* Block Tables */
        in = tables.parse(in, in_max, err);
        break;
    case 3: /* Block Queries */
        in = cbor_array_parse(in, in_max, &queries, err);
        break;
    case 4: /* Address event counts */
        in = cbor_array_parse(in, in_max, &address_events, err);
        break;
    default:
        in = cbor_skip(in, in_max, err);
        break;
    }
    return in;
}

void cdnsBlock::clear()
{
    if (is_filled) {
        preamble.clear();
        statistics.clear();
        tables.clear();
        queries.clear();
        address_events.clear();
        is_filled = false;
        block_start_us = 0;
    }
}
   
cdns_class_id::cdns_class_id() :
    rr_type(0),
    rr_class(0)
{
}

cdns_class_id::~cdns_class_id()
{
}

uint8_t * cdns_class_id::parse(uint8_t* in, uint8_t const* in_max, int* err)
{
    return cbor_map_parse(in, in_max, this, err);
}

uint8_t* cdns_class_id::parse_map_item(uint8_t* in, uint8_t const * in_max, int64_t val, int* err)
{
    switch (val) {
    case 0:
        in = cbor_parse_int(in, in_max, &rr_type, 0, err);
        break;
    case 1:
        in = cbor_parse_int(in, in_max, &rr_class, 0, err);
        break;
    default:
        in = NULL;
        *err = CBOR_ILLEGAL_VALUE;
        break;
    }
    return in;
}

cdnsBlockTables::cdnsBlockTables():
    is_filled(false)
{
}

cdnsBlockTables::~cdnsBlockTables()
{
}

uint8_t* cdnsBlockTables::parse(uint8_t* in, uint8_t const* in_max, int* err)
{
    if (is_filled) {
        clear();
    }
    is_filled = true;
    return cbor_map_parse(in, in_max, this, err);
}

uint8_t* cdnsBlockTables::parse_map_item(uint8_t* in, uint8_t const* in_max, int64_t val, int* err)
{
    switch (val) {
    case 0: // ip_address
        in = cbor_array_parse(in, in_max, &addresses, err);
        break;
    case 1: // classtype
        in = cbor_array_parse(in, in_max, &class_ids, err);
        break;
    case 2: // name_rdata
        in = cbor_array_parse(in, in_max, &name_rdata, err);
        break;
    case 3: // query_signature
        in = cbor_array_parse(in, in_max, &q_sigs, err);
        break;
    case 4: // question_list,
        in = cbor_array_parse(in, in_max, &question_list, err);
        break;
    case 5: // question_rr,
        in = cbor_array_parse(in, in_max, &qrr, err);
        break;
    case 6: // rr_list,
        in = cbor_array_parse(in, in_max, &rr_list, err);
        break;
    case 7: // rr,
        in = cbor_array_parse(in, in_max, &rrs, err);
        break;
    default:
        in = cbor_skip(in, in_max, err);
    }
    return in;
}

void cdnsBlockTables::clear()
{
    addresses.clear();
    class_ids.clear();
    name_rdata.clear();
    q_sigs.clear();
    question_list.clear();
    qrr.clear();
    rr_list.clear();
    rrs.clear();
    is_filled = false;
}

cdns_query::cdns_query():
    time_offset_usec(0),
    client_address_index(-1),
    client_port(0),
    transaction_id(0),
    query_signature_index(-1),
    client_hoplimit(0),
    delay_useconds(0),
    query_name_index(-1),
    query_size(0),
    response_size(0)
{
}

cdns_query::~cdns_query()
{
}

uint8_t* cdns_query::parse(uint8_t* in, uint8_t const* in_max, int* err)
{
    return cbor_map_parse(in, in_max, this, err);
}

uint8_t* cdns_query::parse_map_item(uint8_t* old_in, uint8_t const* in_max, int64_t val, int* err)
{
    uint8_t* in = old_in;
    switch (val) {
    case 0: // time_useconds
        in = cbor_parse_int(in, in_max, &time_offset_usec, 1, err);
        break;
    case 1: // time_pseconds
    {
        int64_t t = 0;
        in = cbor_parse_int64(in, in_max, &t, 1, err);
        if (in != NULL) {
            t /= 1000000;
            time_offset_usec = (int)t;
        }
        break;
    }
    case 2: // client_address_index
        in = cbor_parse_int(in, in_max, &client_address_index, 0, err);
        break;
    case 3: // client_port,
        in = cbor_parse_int(in, in_max, &client_port, 0, err);
        break;
    case 4: // transaction_id
        in = cbor_parse_int(in, in_max, &transaction_id, 0, err);
        break;
    case 5: // query_signature_index
        in = cbor_parse_int(in, in_max, &query_signature_index, 0, err);
        break;
    case 6: // client_hoplimit
        in = cbor_parse_int(in, in_max, &client_hoplimit, 0, err);
        break;
    case 7: // delay_useconds
        in = cbor_parse_int(in, in_max, &delay_useconds, 1, err);
        break;
    case 8: // delay_pseconds
    {
        int64_t t = 0;
        in = cbor_parse_int64(in, in_max, &t, 0, err);
        if (in != NULL) {
            t /= 1000000;
            delay_useconds = (int)t;
        }
        break;
    }
    case 9: // query_name_index
        in = cbor_parse_int(in, in_max, &query_name_index, 0, err);
        break;
    case 10: // query_size
        in = cbor_parse_int(in, in_max, &query_size, 0, err);
        break;
    case 11: // response_size
        in = cbor_parse_int(in, in_max, &response_size, 0, err);
        break;
    case 12: // query_extended
        in = q_extended.parse(in, in_max, err);
        break;
    case 13: // response_extended,\n");
        in = r_extended.parse(in, in_max, err);
        break;
    default:
        /* TODO: something */
        in = cbor_skip(in, in_max, err);
        break;
    }

    if (in == NULL) {
        fprintf(stderr, "\nError parsing query field type %d\n", (int)val);
    }

    return in;
}

cdns_qr_extended::cdns_qr_extended():
    question_index(-1),
    answer_index(-1),
    authority_index(-1),
    additional_index(-1),
    is_filled(false)
{
}

cdns_qr_extended::~cdns_qr_extended()
{
}

uint8_t* cdns_qr_extended::parse(uint8_t* in, uint8_t const* in_max, int* err)
{
    if (is_filled) {
        clear();
    }
    is_filled = true;

    return cbor_map_parse(in, in_max, this, err);
}

uint8_t* cdns_qr_extended::parse_map_item(uint8_t* in, uint8_t const* in_max, int64_t val, int* err)
{
    switch (val) {
    case 0: // question_index
        in = cbor_parse_int(in, in_max, &question_index, 0, err);
        break;
    case 1: // answer_index
        in = cbor_parse_int(in, in_max, &answer_index, 0, err);
        break;
    case 2: // authority_index
        in = cbor_parse_int(in, in_max, &authority_index, 0, err);
        break;
    case 3: // additional_index
        in = cbor_parse_int(in, in_max, &additional_index, 0, err);
        break;
    default:
        /* TODO: something */
        in = cbor_skip(in, in_max, err);
        break;
    }
    return in;
}

void cdns_qr_extended::clear()
{
    question_index = -1;
    answer_index = -1;
    authority_index = -1;
    additional_index = -1;
    is_filled = false;
}

cdns_query_signature::cdns_query_signature():
    server_address_index(-1),
    server_port(0),
    transport_flags(0),
    qr_sig_flags(0),
    query_opcode(0),
    qr_dns_flags(0),
    query_rcode(0),
    query_classtype_index(-1),
    query_qd_count(0),
    query_an_count(0),
    query_ar_count(0),
    query_ns_count(0),
    edns_version(-1),
    udp_buf_size(0),
    opt_rdata_index(-1),
    response_rcode(0)
{
}

cdns_query_signature::~cdns_query_signature()
{
}

uint8_t* cdns_query_signature::parse(uint8_t* in, uint8_t const* in_max, int* err)
{
    return cbor_map_parse(in, in_max, this, err);
}

uint8_t* cdns_query_signature::parse_map_item(uint8_t* in, uint8_t const* in_max, int64_t val, int* err)
{
    switch (val) {
    case 0: /*  server_address_index */
        in = cbor_parse_int(in, in_max, &server_address_index, 0, err);
        break;
    case 1: /*  server_port */
        in = cbor_parse_int(in, in_max, &server_port, 0, err);
        break;
    case 2: /*  transport_flags */
        in = cbor_parse_int(in, in_max, &transport_flags, 0, err);
        break;
    case 3: /*  qr_sig_flags */
        in = cbor_parse_int(in, in_max, &qr_sig_flags, 0, err);
        break;
    case 4: /*  query_opcode */
        in = cbor_parse_int(in, in_max, &query_opcode, 0, err);
        break;
    case 5: /*  qr_dns_flags */
        in = cbor_parse_int(in, in_max, &qr_dns_flags, 0, err);
        break;
    case 6: /*  query_rcode */
        in = cbor_parse_int(in, in_max, &query_rcode, 0, err);
        break;
    case 7: /*  query_classtype_index */
        in = cbor_parse_int(in, in_max, &query_classtype_index, 0, err);
        break;
    case 8: /*  query_qd_count */
        in = cbor_parse_int(in, in_max, &query_qd_count, 0, err);
        break;
    case 9: /*  query_an_count */
        in = cbor_parse_int(in, in_max, &query_an_count, 0, err);
        break;
    case 10: /*  query_ar_count */
        in = cbor_parse_int(in, in_max, &query_ar_count, 0, err);
        break;
    case 11: /*  query_ns_count */
        in = cbor_parse_int(in, in_max, &query_ns_count, 0, err);
        break;
    case 12: /*  edns_version */
        in = cbor_parse_int(in, in_max, &edns_version, 0, err);
        break;
    case 13: /*  udp_buf_size */
        in = cbor_parse_int(in, in_max, &udp_buf_size, 0, err);
        break;
    case 14: /*  opt_rdata_index */
        in = cbor_parse_int(in, in_max, &opt_rdata_index, 0, err);
        break;
    case 15: /*  response_rcode */
        in = cbor_parse_int(in, in_max, &response_rcode, 0, err);
        break;
    default:
        in = cbor_skip(in, in_max, err);
        break;
    }
    return in;
}

cdns_question::cdns_question():
    name_index(-1),
    classtype_index(-1)
{
}

cdns_question::~cdns_question()
{
}

uint8_t* cdns_question::parse(uint8_t* in, uint8_t const* in_max, int* err)
{
    return cbor_map_parse(in, in_max, this, err);
}

uint8_t* cdns_question::parse_map_item(uint8_t* in, uint8_t const* in_max, int64_t val, int* err)
{
    switch (val) {
    case 0: // name_index
        in = cbor_parse_int(in, in_max, &name_index, 0, err);
        break;
    case 1: // classtype_index
        in = cbor_parse_int(in, in_max, &classtype_index, 0, err);
        break;
    default:
        /* TODO: something */
        in = cbor_skip(in, in_max, err);
        break;
    }
    return in;
}

cdns_rr_field::cdns_rr_field() :
    name_index(-1),
    classtype_index(-1),
    ttl(0),
    rdata_index(-1)
{
}

cdns_rr_field::~cdns_rr_field()
{
}

uint8_t* cdns_rr_field::parse(uint8_t* in, uint8_t const* in_max, int* err)
{
    return cbor_map_parse(in, in_max, this, err);
}

uint8_t* cdns_rr_field::parse_map_item(uint8_t* in, uint8_t const* in_max, int64_t val, int* err)
{
    switch (val) {
    case 0: // name_index
        in = cbor_parse_int(in, in_max, &name_index, 0, err);
        break;
    case 1: // classtype_index
        in = cbor_parse_int(in, in_max, &classtype_index, 0, err);
        break;
    case 2: // ttl
        in = cbor_parse_int(in, in_max, &ttl, 0, err);
        break;
    case 3: // rdata_index
        in = cbor_parse_int(in, in_max, &rdata_index, 0, err);
        break;
    default:
        /* TODO: something */
        in = cbor_skip(in, in_max, err);
        break;
    }
    return in;
}

cdns_rr_list::cdns_rr_list()
{
}

cdns_rr_list::~cdns_rr_list()
{
}

uint8_t* cdns_rr_list::parse(uint8_t* in, uint8_t const* in_max, int* err)
{
    return cbor_array_parse(in, in_max, &rr_index, err);
}

cdns_block_statistics::cdns_block_statistics() :
    total_packets(0),
    total_pairs(0),
    unmatched_queries(0),
    unmatched_responses(0),
    completely_malformed_packets(0),
    partially_malformed_packets(0),
    compactor_non_dns_packets(0),
    compactor_out_of_order_packets(0),
    compactor_missing_pairs(0),
    compactor_missing_packets(0),
    compactor_missing_non_dns(0),
    is_filled(false)
{
}

cdns_block_statistics::~cdns_block_statistics()
{
}

uint8_t* cdns_block_statistics::parse(uint8_t* in, uint8_t const* in_max, int* err)
{
    clear();
    is_filled = true;
    return cbor_map_parse(in, in_max, this, err);
}

uint8_t* cdns_block_statistics::parse_map_item(uint8_t* in, uint8_t const* in_max, int64_t val, int* err)
{

    switch (val) {
    case 0: // total_packets
        in = cbor_parse_int(in, in_max, &total_packets, 0, err);
        break;
    case 1: // total_pairs,
        in = cbor_parse_int(in, in_max, &total_pairs, 0, err);
        break;
    case 2: // unmatched_queries,
        in = cbor_parse_int(in, in_max, &unmatched_queries, 0, err);
        break;
    case 3: // unmatched_responses,
        in = cbor_parse_int(in, in_max, &unmatched_responses, 0, err);
        break;
    case 4: // completely_malformed_packets,
        in = cbor_parse_int(in, in_max, &completely_malformed_packets, 0, err);
        break;
    case 5: // partially_malformed_packets,
        in = cbor_parse_int(in, in_max, &partially_malformed_packets, 0, err);
        break;
    case 6: // compactor_non_dns_packets,
        in = cbor_parse_int(in, in_max, &compactor_non_dns_packets, 0, err);
        break;
    case 7: // compactor_out_of_order_packets,
        in = cbor_parse_int(in, in_max, &compactor_out_of_order_packets, 0, err);
        break;
    case 8: // compactor_missing_pairs,
        in = cbor_parse_int(in, in_max, &compactor_missing_pairs, 0, err);
        break;
    case 9: // compactor_missing_packets,
        in = cbor_parse_int(in, in_max, &compactor_missing_packets, 0, err);
        break;
    case 10: // compactor_missing_non_dns
        in = cbor_parse_int(in, in_max, &compactor_missing_non_dns, 0, err);
        break;
    default:
        in = cbor_skip(in, in_max, err);
    }

    return in;
}

void cdns_block_statistics::clear()
{
    if (is_filled) {
        total_packets = 0;
        total_pairs = 0;
        unmatched_queries = 0;
        unmatched_responses = 0;
        completely_malformed_packets = 0;
        partially_malformed_packets = 0;
        compactor_non_dns_packets = 0;
        compactor_out_of_order_packets = 0;
        compactor_missing_pairs = 0;
        compactor_missing_packets = 0;
        compactor_missing_non_dns = 0;
        is_filled = 0;
    }
}

cdns_block_preamble::cdns_block_preamble():
    earliest_time_sec(0), 
    earliest_time_usec(0),
    is_filled(false)
{
}

cdns_block_preamble::~cdns_block_preamble()
{
}

uint8_t* cdns_block_preamble::parse(uint8_t* in, uint8_t const* in_max, int* err)
{
    clear();

    in = cbor_map_parse(in, in_max, this, err);

    if (!is_filled) {
        *err = CBOR_MALFORMED_VALUE;
        in = NULL;
    }
    return(in);
}

uint8_t* cdns_block_preamble::parse_map_item(uint8_t* in, uint8_t const* in_max, int64_t val, int* err)
{
    switch (val) {
    case 1: // total_packets
    {
        std::vector<int> t;
        in = cbor_array_parse(in, in_max, &t, err);

        if (in != NULL) {
            if (t.size() == 2) {
                earliest_time_sec = t[0];
                earliest_time_usec = t[1];
                is_filled = true;
            }
            else {
                *err = CBOR_MALFORMED_VALUE;
                in = NULL;
            }
        }

        break;
    }
    default:
        in = cbor_skip(in, in_max, err);
        break;
    }

    return in;
}

void cdns_block_preamble::clear()
{
    earliest_time_sec = 0;
    earliest_time_usec = 0;
    is_filled = false;
}

cdns_address_event_count::cdns_address_event_count():
    ae_type(0),
    ae_code(0),
    ae_address_index(0),
    ae_count(0)
{
}

cdns_address_event_count::~cdns_address_event_count()
{
}

uint8_t* cdns_address_event_count::parse(uint8_t* in, uint8_t const* in_max, int* err)
{
    return cbor_map_parse(in, in_max, this, err);
}

uint8_t* cdns_address_event_count::parse_map_item(uint8_t* in, uint8_t const* in_max, int64_t val, int* err)
{
    switch (val) {
    case 0: // ae_type
        in = cbor_parse_int(in, in_max, &ae_type, 1, err);
        break;
    case 1: // ae_code,
        in = cbor_parse_int(in, in_max, &ae_code, 1, err);
        break;
    case 2: // ae_address_index,
        in = cbor_parse_int(in, in_max, &ae_address_index, 1, err);
        break;
    case 3: // ae_count
        in = cbor_parse_int(in, in_max, &ae_count, 1, err);
        break;
    default:
        in = cbor_skip(in, in_max, err);
    }

    return in;
}
