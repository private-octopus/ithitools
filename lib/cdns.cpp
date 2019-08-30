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
    F(NULL),
    buf(NULL),
    buf_size(0),
    buf_read(0),
    buf_parsed(0),
    end_of_file(false)
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

bool cdns::open(char const* file_name, size_t buf_size)
{
    bool ret = false;

    if (F == NULL && buf == NULL) {
        if (buf_size == 0) {
            this->buf_size = 0x10000;
        }
        else {
            this->buf_size = buf_size;
        }

        F = ithi_file_open(file_name, "rb");
        if (F != NULL) {
            buf = new uint8_t[this->buf_size];

            if (buf != NULL) {
                ret = load_buffer();
            }
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

bool cdns::load_buffer()
{
    if (buf_parsed < buf_read) {
        buf_read -= buf_parsed;
        memmove(buf, buf + buf_parsed, buf_read);
    }
    else {
        buf_read = 0;
    }
    buf_parsed = 0;

    if (buf_read < buf_size && !end_of_file) {
        size_t asked = buf_size - buf_read;
        size_t n_bytes = fread(buf + buf_read, 1, asked, F);

        end_of_file = (n_bytes < asked);
        buf_read += n_bytes;
    }

    return (buf_read > 0);
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

cdnsBlock::cdnsBlock()
{
}

cdnsBlock::~cdnsBlock()
{
}

bool cdnsBlock::load(uint8_t* in, uint8_t* in_max, int* err, FILE* F_out)
{
    return false;
}
   
cdns_class_id::cdns_class_id() :
    rr_type(0),
    rr_class(0)
{
}

cdns_class_id::~cdns_class_id()
{
}

uint8_t * cdns_class_id::parse(uint8_t* in, uint8_t* in_max, int* err)
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
