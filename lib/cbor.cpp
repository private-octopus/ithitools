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
#include "cbor.h"

/* Generic functions
 */

uint8_t* cbor_get_number(uint8_t* in, uint8_t const* in_max, int64_t* val)
{
    int64_t v = (*in++) & 0x1F;

    if (v == 0x1F) {
        v = CBOR_END_OF_ARRAY;
    }
    else if (v > 27) {
        /* illegal value */
        v = CBOR_ILLEGAL_VALUE;
        in = NULL;
    }
    else if (v >= 24) {
        size_t l = ((size_t)1) << (v - 24);
        if (in + l > in_max) {
            /* Malformed value */
            v = CBOR_MALFORMED_VALUE;
            in = NULL;
        }
        else {
            v = 0;
            while (l > 0) {
                v <<= 8;
                v |= (*in++);
                l--;
            }
        }
    }

    *val = v;
    return in;
}

/* CBOR to text functions, mostly for debug purpose.
 */

char* cbor_print_int(char* out, char const* out_max, int64_t val, int is_negative)
{
    int c[20];
    int l = 0;

    if (is_negative) {
        if (out < out_max) {
            *out++ = '-';
        }
    }

    do {
        c[l++] = val % 10;
        val /= 10;
    } while (val > 0);

    if (out + l < out_max) {
        while (l > 0) {
            *out++ = '0' + c[--l];
        }
    }

    return out;
}

char* cbor_print_text_part(char* out, char const* out_max, uint8_t* in, int64_t val)
{
    for (int64_t i = 0; i < val && out < out_max; i++) {
        int c = in[i];

        if (c == '\\' || c == '"') {
            if (out + 1 < out_max) {
                *out++ = '\\';
                *out++ = c;
            }
        }
        else if (c < ' ' || c > 126) {
            if (out + 3 < out_max) {
                int n_char;
#ifdef _WINDOWS
                n_char = sprintf_s(out, out_max - out, "\\%03d", c);
                if (n_char < 0) {
                    /* Ignore errors for now */
                    n_char = 0;
                }
#else 
                n_char = sprintf(out, "\\%03d", c);
#endif
                out += n_char;
            }
        }
        else {
            *out++ = c;
        }
    }

    return out;
}

static int hex_to_char(int x)
{
    x &= 0x0F;
    return ((x < 10) ? '0' + x : 'a' - 10 + x);
}

char* cbor_print_bytes_part(char* out, char const* out_max, uint8_t* in, int64_t val)
{
    for (int64_t i = 0; i < val && out + 1 < out_max; i++) {
        int c = in[i];
        *out++ = hex_to_char(c >> 4);
		*out++ = hex_to_char(c);
    }

    return out;
}

uint8_t* cbor_text_to_text(uint8_t* in, uint8_t const* in_max, char** p_out, char const* out_max, int64_t val, int* err)
{
    char* out = *p_out;
    if (out < out_max) {
        *out++ = '"';
        if (val == CBOR_END_OF_ARRAY) {
            while (in < in_max && in != NULL) {
                if (*in == 0xff) {
                    in++;
                    break;
                }
                else {
                    int64_t val;
                    int cbor_class = CBOR_CLASS(*in);

                    in = cbor_get_number(in, in_max, &val);

                    if (in == NULL) {
                        *err = (int)val;
                    }
                    else if (val < 0 || in + val > in_max || cbor_class != CBOR_T_TEXT) {
                        *err = CBOR_MALFORMED_VALUE;
                        in = NULL;
                    }
                    else if (val > 0) {
                        out = cbor_print_text_part(out, out_max, in, val);
                        in += val;
                    }
                }
            }
        }
        else {
            if (val < 0 || in + val > in_max) {
                *err = CBOR_MALFORMED_VALUE;
                in = NULL;
            }
            else if (val > 0) {
                out = cbor_print_text_part(out, out_max, in, val);
                in += val;
            }
        }
        if (out < out_max) {
            *out++ = '"';
        }
    }
    *p_out = out;
    return in;
}

uint8_t* cbor_bytes_to_text(uint8_t* in, uint8_t const* in_max, char** p_out, char const* out_max, int64_t val, int* err)
{
    char* out = *p_out;
    if (out + 2 < out_max) {
        *out++ = '0';
        *out++ = 'x';

        if (val == CBOR_END_OF_ARRAY) {
            while (in < in_max && in != NULL) {
                if (*in == 0xff) {
                    in++;
                    break;
                }
                else {
                    int64_t val;
                    int cbor_class = CBOR_CLASS(*in);

                    in = cbor_get_number(in, in_max, &val);

                    if (in == NULL) {
                        *err = (int)val;
                    }
                    else if (val < 0 || in + val > in_max || cbor_class != CBOR_T_BYTES) {
                        *err = CBOR_MALFORMED_VALUE;
                        in = NULL;
                    }
                    else if (val > 0) {
                        out = cbor_print_bytes_part(out, out_max, in, val);
                        in += val;
                    }
                }
            }
        }
        else {
            if (val < 0 || in + val > in_max) {
                *err = CBOR_MALFORMED_VALUE;
                in = NULL;
            }
            else if (val > 0) {
                out = cbor_print_bytes_part(out, out_max, in, val);
                in += val;
            }
        }
    }
    *p_out = out;
    return in;
}

/* Float is a misnomer. There are a variety of values in that basic type class:
 * 0..23       -- Simple value (value 0..23)
 *     0..19   -- unassigned
 *     20      -- false
 *     21      -- true
 *     22      -- null
 *     23      -- undefined
 * 24          -- Simple value (value 32..255 in following byte)
 * 25          -- IEEE 754 Half-Precision Float (16 bits follow)
 * 26          -- IEEE 754 Single-Precision Float (32 bits follow)
 * 27          -- IEEE 754 Double-Precision Float (64 bits follow)
 * 28-30       -- (Unassigned)
 * 31          -- "break" stop code for indefinite-length items
 */

uint8_t* cbor_float_to_text(uint8_t* in, uint8_t const* in_max, char** p_out, char const* out_max, int64_t val, int* err)
{
    char* out = *p_out;

    if (val < 20 || val > 27) {
        in = NULL;
        *err = CBOR_MALFORMED_VALUE;
    }
    else if (val < 24) {
        char const* t = NULL;
        size_t l = 0;

        if (val == 20) {
            t = "false";
            l = 5;
        }
        else if (val == 21) {
            t = "true";
            l = 4;
        }
        else if (val == 22) {
            t = "null";
            l = 4;
        }
        else if (val == 22) {
            t = "undef";
            l = 4;
        }

        if (l > 0 && out + l < out_max) {
            memcpy(out, t, l);
            out += l;
        }
    }
    else if (val == 24) {
        /* single value in following byte */
		if (in < in_max){
            in = NULL;
            *err = CBOR_NOT_IMPLEMENTED;
        }
        else {
            in = NULL;
            *err = CBOR_MALFORMED_VALUE;
        }
    }
    else {
        /* TODO: manage these  floating point types */
        in = NULL;
        *err = CBOR_NOT_IMPLEMENTED;
    }
    *p_out = out;
    return in;
}

uint8_t* cbor_to_text(uint8_t* in, uint8_t const* in_max, char** p_out, char const* out_max, int* err)
{
    char* out = *p_out;

    if (in != NULL && in < in_max) {
        int cbor_class = CBOR_CLASS(*in);
        int64_t val;

        in = cbor_get_number(in, in_max, &val);

        if (in == NULL) {
            *err = (int)val;
        }
        else if (val == CBOR_END_OF_ARRAY) {
            switch (cbor_class) {
            case CBOR_T_UINT: /* unsigned integer */
            case CBOR_T_NINT: /* negative integer */
            case CBOR_T_FLOAT:
                in = NULL;
                *err = CBOR_MALFORMED_VALUE;
                break;
            case CBOR_T_BYTES:
                in = cbor_bytes_to_text(in, in_max, p_out, out_max, val, err);
                break;
            case CBOR_T_TEXT:
                in = cbor_text_to_text(in, in_max, p_out, out_max, val, err);
                break;
            case CBOR_T_ARRAY:
                in = cbor_array_to_text(in, in_max, p_out, out_max, val, 0, err);
                break;
            case CBOR_T_MAP:
                in = cbor_array_to_text(in, in_max, p_out, out_max, val, 1, err);
                break;
            case CBOR_T_TAGGED:
                /* TODO: manage these types */
                in = NULL;
                *err = CBOR_NOT_IMPLEMENTED;
                break;
            }
        }
        else {
            switch (cbor_class) {
            case CBOR_T_UINT: /* unsigned integer */
                *p_out = cbor_print_int(out, out_max, val, 0);
                break;
            case CBOR_T_NINT: /* negative integer */
                *p_out = cbor_print_int(out, out_max, val + 1, 1);
                break;
            case CBOR_T_BYTES:
                in = cbor_bytes_to_text(in, in_max, p_out, out_max, val, err);
                break;
            case CBOR_T_TEXT:
                in = cbor_text_to_text(in, in_max, p_out, out_max, val, err);
                break;
            case CBOR_T_ARRAY:
                in = cbor_array_to_text(in, in_max, p_out, out_max, val, 0, err);
                break;
            case CBOR_T_MAP:
                /* TODO: manage these types */
                in = cbor_array_to_text(in, in_max, p_out, out_max, val, 1, err);
                break;
            case CBOR_T_TAGGED:
                /* TODO: manage these types */
                in = NULL;
                *err = CBOR_NOT_IMPLEMENTED;
                break;
            case CBOR_T_FLOAT:
                /* TODO: manage these types */
                in = cbor_float_to_text(in, in_max, p_out, out_max, val, err);
                break;
            }
        }
    }

    if (*p_out < out_max) {
        (*p_out)[0] = 0;
    }

    return in;
}

uint8_t* cbor_array_to_text(uint8_t* in, uint8_t const* in_max, char** p_out, char const* out_max, int64_t val, int is_map, int* err)
{
    int not_first = 0;
    int is_undef = 0;

    if (*p_out < out_max) {
        (*p_out)[0] = '[';
        *p_out += 1;
    }

    if (val == CBOR_END_OF_ARRAY) {
        is_undef = 1;
        val = 0xffffffff;
    }
    else if (is_map != 0) {
        val *= 2;
    }

    while (val > 0 && in != NULL) {
        if (in >= in_max) {
            *err = CBOR_MALFORMED_VALUE;
            in = NULL;
        }
        else if (*in == 0xFF) {
            if (is_undef) {
                in++;
                break;
            }
            else {
                *err = CBOR_MALFORMED_VALUE;
                in = NULL;
            }
        }
        else {
            val--;
            if (not_first && *p_out < out_max) {
                (*p_out)[0] = ',';
                *p_out += 1;
            }
            not_first = 1;
            in = cbor_to_text(in, in_max, p_out, out_max, err);
        }
    }

    if (in != NULL && *p_out < out_max) {
        (*p_out)[0] = ']';
        *p_out += 1;
    }

    if (is_map != 0 && is_undef != 0 && in != NULL) {
        int64_t nb_pairs = ((int64_t)0xffffffff) - val;

        if ((nb_pairs & 1) != 0) {
            *err = CBOR_MALFORMED_VALUE;
            in = NULL;
        }
    }

    return in;
}

/* Skip functions
 */

 /* CBOR to text functions, mostly for debug purpose.
 */

uint8_t* cbor_text_skip(uint8_t* in, uint8_t const* in_max, int64_t val, int* err)
{
    if (val == CBOR_END_OF_ARRAY) {
        while (in < in_max && in != NULL) {
            if (*in == 0xff) {
                in++;
                break;
            }
            else {
                int64_t val;
                int cbor_class = CBOR_CLASS(*in);

                in = cbor_get_number(in, in_max, &val);

                if (in == NULL) {
                    *err = (int)val;
                }
                else if (val < 0 || in + val > in_max || cbor_class != CBOR_T_TEXT) {
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
                else if (val > 0) {
                    in += val;
                }
            }
        }
    }
    else {
        if (val < 0 || in + val > in_max) {
            *err = CBOR_MALFORMED_VALUE;
            in = NULL;
        }
        else if (val > 0) {
            in += val;
        }
    }
    return in;
}

uint8_t* cbor_bytes_skip(uint8_t* in, uint8_t const* in_max, int64_t val, int* err)
{
    if (val == CBOR_END_OF_ARRAY) {
        while (in < in_max && in != NULL) {
            if (*in == 0xff) {
                in++;
                break;
            }
            else {
                int64_t val;
                int cbor_class = CBOR_CLASS(*in);

                in = cbor_get_number(in, in_max, &val);

                if (in == NULL) {
                    *err = (int)val;
                }
                else if (val < 0 || in + val > in_max || cbor_class != CBOR_T_BYTES) {
                    *err = CBOR_MALFORMED_VALUE;
                    in = NULL;
                }
                else if (val > 0) {
                    in += val;
                }
            }
        }
    }
    else {
        if (val < 0 || in + val > in_max) {
            *err = CBOR_MALFORMED_VALUE;
            in = NULL;
        }
        else if (val > 0) {
            in += val;
        }
    }
    return in;
}

/* Only considering legit values (20 to 24) for now.
 */

uint8_t* cbor_float_skip(uint8_t* in, uint8_t const* in_max, int64_t val, int* err)
{
    if (val < 20 || val > 27) {
        in = NULL;
        *err = CBOR_MALFORMED_VALUE;
    }
    else if (val == 24) {
        /* single value in following byte */
        if (in < in_max) {
            in++;
        } else {
            in = NULL;
            *err = CBOR_MALFORMED_VALUE;
        }
    }
    else if (val > 24) {
        /* TODO: manage these  floating point types */
        in = NULL;
        *err = CBOR_NOT_IMPLEMENTED;
    }

    return in;
}

uint8_t* cbor_array_skip(uint8_t* in, uint8_t const* in_max, int64_t val, int is_map, int* err)
{
    int is_undef = 0;

    if (val == CBOR_END_OF_ARRAY) {
        is_undef = 1;
        val = 0xffffffff;
    }
    else if (is_map != 0) {
        val *= 2;
    }

    while (val > 0 && in != NULL) {
        if (in >= in_max) {
            *err = CBOR_MALFORMED_VALUE;
            in = NULL;
        }
        else if (*in == 0xFF) {
            if (is_undef) {
                in++;
                break;
            }
            else {
                *err = CBOR_MALFORMED_VALUE;
                in = NULL;
            }
        }
        else {
            val--;
            in = cbor_skip(in, in_max, err);
        }
    }

    if (is_map != 0 && is_undef != 0 && in != NULL) {
        int64_t nb_pairs = ((int64_t)0xffffffff) - val;

        if ((nb_pairs & 1) != 0) {
            *err = CBOR_MALFORMED_VALUE;
            in = NULL;
        }
    }

    return in;
}

uint8_t* cbor_skip(uint8_t* in, uint8_t const* in_max, int* err)
{
    if (in != NULL && in < in_max) {
        int cbor_class = CBOR_CLASS(*in);
        int64_t val;

        in = cbor_get_number(in, in_max, &val);

        if (in == NULL) {
            *err = (int)val;
        }
        else if (val == CBOR_END_OF_ARRAY) {
            switch (cbor_class) {
            case CBOR_T_UINT: /* unsigned integer */
            case CBOR_T_NINT: /* negative integer */
            case CBOR_T_FLOAT:
                in = NULL;
                *err = CBOR_MALFORMED_VALUE;
                break;
            case CBOR_T_BYTES:
                in = cbor_bytes_skip(in, in_max, val, err);
                break;
            case CBOR_T_TEXT:
                in = cbor_text_skip(in, in_max, val, err);
                break;
            case CBOR_T_ARRAY:
                in = cbor_array_skip(in, in_max, val, 0, err);
                break;
            case CBOR_T_MAP:
                in = cbor_array_skip(in, in_max, val, 1, err);
                break;
            case CBOR_T_TAGGED:
                /* TODO: manage these types */
                in = NULL;
                *err = CBOR_NOT_IMPLEMENTED;
                break;
            }
        }
        else {
            switch (cbor_class) {
            case CBOR_T_UINT: /* unsigned integer */
                break;
            case CBOR_T_NINT: /* negative integer */
                break;
            case CBOR_T_BYTES:
                in = cbor_bytes_skip(in, in_max, val, err);
                break;
            case CBOR_T_TEXT:
                in = cbor_text_skip(in, in_max, val, err);
                break;
            case CBOR_T_ARRAY:
                in = cbor_array_skip(in, in_max, val, 0, err);
                break;
            case CBOR_T_MAP:
                in = cbor_array_skip(in, in_max, val, 1, err);
                break;
            case CBOR_T_TAGGED:
                /* TODO: manage these types */
                in = NULL;
                *err = CBOR_NOT_IMPLEMENTED;
                break;
            case CBOR_T_FLOAT:
                /* TODO: manage these types */
                in = cbor_float_skip(in, in_max, val, err);
                break;
            }
        }
    }

    return in;
}

uint8_t* cbor_parse_int(uint8_t* in, uint8_t const* in_max, int* v, int is_signed, int* err)
{
    int64_t val;
    int outer_type = CBOR_CLASS(*in);

    in = cbor_get_number(in, in_max, &val);

    if (val < 0) {
        *err = CBOR_MALFORMED_VALUE;
        in = NULL;
    }
    else if (outer_type == CBOR_T_UINT) {
        *v = (int)val;
    }
    else if (outer_type == CBOR_T_NINT && is_signed) {
        *v = -1 * (int)(val + 1);
    }
    else {
        *err = CBOR_MALFORMED_VALUE;
        in = NULL;
    }

    return in;
}



cbor_bytes::cbor_bytes() :
    v(NULL),
    l(0)
{
}

cbor_bytes::cbor_bytes(const cbor_bytes& other)
{
    l = other.l;
    if (l > 0) {
        v = new uint8_t[l];
        memcpy(v, other.v, l);
    }
    else {
        v = NULL;
    }
}

cbor_bytes::~cbor_bytes()
{
    if (v != NULL) {
        delete[] v;
        v = NULL;
    }

    l = 0;
}

uint8_t* cbor_bytes::parse(uint8_t* in, uint8_t const* in_max, int* err)
{
    uint8_t* first = in;

    if (v != NULL || l != 0 || in == NULL) {
        *err = CBOR_UNEXPECTED;
        in = NULL;
    }
    else {
        int outer_type = CBOR_CLASS(*in);
        int64_t val;

        in = cbor_get_number(in, in_max, &val);

        if (in == NULL || outer_type != CBOR_T_BYTES) {
            *err = CBOR_MALFORMED_VALUE;
            in = NULL;
        }
        else  if (val == CBOR_END_OF_ARRAY) {
            /* Need to allocate enough bytes to hold the content. */
            uint8_t* last = cbor_skip(first, in_max, err);

            if (last == NULL) {
                in = NULL;
            }
            else {
                size_t allocated = last - first;
                v = new uint8_t[allocated];

                if (v == NULL) {
                    in = NULL;
                    *err = CBOR_MEMORY;
                }
            }

            while (in < in_max && in != NULL) {
                if (*in == 0xff) {
                    in++;
                    break;
                }
                else {
                    int64_t val;
                    int cbor_class = CBOR_CLASS(*in);

                    in = cbor_get_number(in, in_max, &val);

                    if (in == NULL) {
                        *err = (int)val;
                    }
                    else if (val < 0 || in + val > in_max || cbor_class != CBOR_T_BYTES) {
                        *err = CBOR_MALFORMED_VALUE;
                        in = NULL;
                    }
                    else if (val > 0) {
                        memcpy(v + l, in, (size_t)val);
                        l += (size_t)val;
                        in += val;
                    }
                }
            }
        }
        else {
            if (val < 0 || in + val > in_max) {
                *err = CBOR_MALFORMED_VALUE;
                in = NULL;
            }
            else if (val > 0) {
                v = new uint8_t[(size_t)val];
                if (v == NULL) {
                    in = NULL;
                    *err = CBOR_MEMORY;
                }
                else {
                    memcpy(v, in, (size_t)val);
                    l = (size_t)val;
                    in += val;
                }
            }
        }
    }

    return in;
}

uint8_t* cbor_object_parse(uint8_t* in, uint8_t const* in_max, int* v, int* err)
{
    in = cbor_parse_int(in, in_max, v, 0, err);
    return in;
}