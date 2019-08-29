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
    size_t byte_read = 0;

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
    bool ret = true;
    char* p_out;
    int64_t val;
    int outer_type = CBOR_CLASS(*in);
    int is_undef = 0;

    in = cbor_get_number(in, in_max, &val);

    if (in == NULL || outer_type != CBOR_T_MAP) {
        fprintf(F_out, "Error, cannot parse the first bytes of preamble, type %d.\n", outer_type);
        *err = CBOR_MALFORMED_VALUE;
        in = NULL;
        ret = false;
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
                /*
                if (inner_type != CBOR_T_UINT) {
                    fprintf(F_out, "                Unexpected type: %d(%d)\n", inner_type, (int)inner_val);
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
                else */ {
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
    bool ret = true;
    char* p_out;
    int64_t val;
    int outer_type = CBOR_CLASS(*in);
    int is_undef = 0;

    in = cbor_get_number(in, in_max, &val);

    if (in == NULL || outer_type != CBOR_T_MAP) {
        fprintf(F_out, "       Error, cannot parse the first bytes of block parameters, type %d.\n", outer_type);
        *err = CBOR_MALFORMED_VALUE;
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
                        p_out = out_buf;
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
    bool ret = true;
    char* p_out;
    int64_t val;
    int outer_type = CBOR_CLASS(*in);
    int is_undef = 0;

    in = cbor_get_number(in, in_max, &val);

    if (in == NULL || outer_type != CBOR_T_ARRAY) {
        fprintf(F_out, "   Error, cannot parse the first bytes of block, type=%d.\n", outer_type);
        *err = CBOR_MALFORMED_VALUE;
        in = NULL;
        ret = false;
    }
    else {
        int rank = 0;

        fprintf(F_out, "  [\n");
        if (val == CBOR_END_OF_ARRAY) {
            is_undef = 1;
            val = 0xffffffff;
        }

        while (val > 0 && in != NULL && in < in_max) {
            if (*in == 0xff) {
                fprintf(F_out, "    --end of array");
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
                int inner_type = CBOR_CLASS(*in);

                if (inner_type == CBOR_T_MAP) {
                    /* Records are held in a map */
                    in = dump_block_properties(in, in_max, out_buf, out_max, err, F_out);
                }
                else {
                    p_out = out_buf;
                    rank++;
                    fprintf(F_out, "    -- Property %d:\n    ", rank);
                    in = cbor_to_text(in, in_max, &p_out, out_max, err);
                    fwrite(out_buf, 1, p_out - out_buf, F_out);
                    fprintf(F_out, "\n");
                }
                val--;
            }
        }

        if (in != NULL) {
            fprintf(F_out, "\n  ]\n");
        }

    }

    return in;
}

uint8_t* cdns::dump_block_properties(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out)
{
    bool ret = true;
    char* p_out;
    int64_t val;
    int outer_type = CBOR_CLASS(*in);
    int is_undef = 0;

    in = cbor_get_number(in, in_max, &val);

    if (in == NULL || outer_type != CBOR_T_MAP) {
        fprintf(F_out, "   Error, cannot parse the first bytes of properties, type: %d.\n", outer_type);
        *err = CBOR_MALFORMED_VALUE;
        in = NULL;
        ret = false;
    }
    else {
        int rank = 0;
        fprintf(F_out, "    [\n");
        if (val == CBOR_END_OF_ARRAY) {
            is_undef = 1;
            val = 0xffffffff;
        }

        while (val > 0 && in != NULL && in < in_max ) {
            if (*in == 0xff) {
                fprintf(F_out, "\n        --end of array");
                if (is_undef) {
                    in++;
                }
                else {
                    fprintf(F_out, "    Error, end of array mark unexpected.\n");
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
                    fprintf(F_out, "        %d, ", (int)inner_val);
                    if (inner_val == 2) {
                        in = dump_block_tables(in, in_max, out_buf, out_max, err, F_out);
                    }
                    else if (inner_val == 3) {
                        fprintf(F_out, "[\n");
                        in = dump_list(in, in_max, out_buf, out_max, "            ", "queries", err, F_out);
                        fprintf(F_out, "        ]");
                    }
                    else if (inner_val == 4) {
                        fprintf(F_out, "[\n");
                        in = dump_list(in, in_max, out_buf, out_max, "            ", "address-event-counts", err, F_out);
                        fprintf(F_out, "        ]");
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
            fprintf(F_out, "\n    ]\n");
        }
    }

    return in;
}

uint8_t* cdns::dump_block_tables(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, int* err, FILE* F_out)
{
    bool ret = true;
    char* p_out;
    int64_t val;
    int outer_type = CBOR_CLASS(*in);
    int is_undef = 0;

    in = cbor_get_number(in, in_max, &val);

    if (in == NULL) {
        fprintf(F_out, "       Error, cannot parse the first bytes of block tables.\n");
        *err = CBOR_MALFORMED_VALUE;
        in = NULL;
        ret = false;
    }
    else {
        int rank = 0;
        fprintf(F_out, "  -- block tables type %d, val: %d\n            [\n", outer_type, (int)val);
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
                        in = dump_list(in, in_max, out_buf, out_max, "                    ", "class-types", err, F_out);
                        fprintf(F_out, "                ]");
                    }
                    else if (inner_val == 2) {
                        fprintf(F_out, "[\n");
                        in = dump_list(in, in_max, out_buf, out_max, "                    ", "name-rdata", err, F_out);
                        fprintf(F_out, "                ]");
                    }
                    else if (inner_val == 3) {
                        fprintf(F_out, "[\n");
                        in = dump_list(in, in_max, out_buf, out_max, "                    ", "qr-sigs", err, F_out);
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


uint8_t* cdns::dump_list(uint8_t* in, uint8_t* in_max, char* out_buf, char* out_max, char const * indent, char const * list_name, int* err, FILE* F_out)
{

    bool ret = true;
    char* p_out;
    int64_t val;
    int outer_type = CBOR_CLASS(*in);
    int is_undef = 0;

    in = cbor_get_number(in, in_max, &val);

    if (in == NULL) {
        fprintf(F_out, "%sError, cannot parse the first bytes of %s.\n", indent, list_name);
        *err = CBOR_MALFORMED_VALUE;
        in = NULL;
        ret = false;
    }
    else {
        int rank = 0;
        fprintf(F_out, "%s-- %s type %d, val: %d\n", indent, list_name, outer_type, (int)val);
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
