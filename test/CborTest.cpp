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

#include "CborTest.h"


/* Examples of CBOR encoding from RFC 7049 */

/*   83        -- Array of length 3
 *     01     -- 1
 *     82     -- Array of length 2
 *        02  -- 2
 *        03  -- 3
 *     82     -- Array of length 2
 *        04  -- 4
 *        05  -- 5
 */
uint8_t cbor_test1[] = { 0x83, 0x01, 0x82, 0x02, 0x03, 0x82, 0x04, 0x05 };
char const * cbor_test_out1 = "[1,[2,3],[4,5]]";

CborTest::CborTest()
{
}

CborTest::~CborTest()
{
}

#define CBOR_CLASS(x) (((x)>>5)&7)
#define CBOR_T_UINT 0
#define CBOR_T_NINT 1
#define CBOR_T_BYTES 2
#define CBOR_T_TEXT 3
#define CBOR_T_ARRAY 4
#define CBOR_T_MAP 5
#define CBOR_T_TAGGED 6
#define CBOR_T_FLOAT 7

#define CBOR_END_OF_ARRAY -1
#define CBOR_ILLEGAL_VALUE -2
#define CBOR_MALFORMED_VALUE -3
#define CBOR_NOT_IMPLEMENTED -4

uint8_t* cbor_get_number(uint8_t* in, uint8_t const * in_max, int64_t* val)
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

char* cbor_print_int(char* out, char const* out_max, int64_t val, int is_negative) 
{
    int n_char;

    if (is_negative) {
        if (out < out_max) {
            *out++ = '-';
        }
    }

#ifdef _WINDOWS
    n_char = sprintf_s(out, out_max - out, "%d", (int)val);
    if (n_char < 0) {
        /* Ignore errors for now */
        n_char = 0;
    }
#else 
    n_char = sprintf(out, "%d", (int)val);
#endif

    if (out + n_char < out_max) {
        out += n_char;
    }

    return out;
}

uint8_t* cbor_array_to_text(uint8_t* in, uint8_t const* in_max, char** p_out, char const* out_max, int64_t val, int* err);

uint8_t * cbor_to_text(uint8_t * in, uint8_t const * in_max, char** p_out, char const * out_max , int * err)
{
    uint8_t * current = in;
    char* out = *p_out;
    
    if (current != NULL && current < in_max) {
        int cbor_class = CBOR_CLASS(*current);
        int64_t val;

        current = cbor_get_number(current, in_max, &val);

        if (current == NULL) {
            *err = val;
        } else if (val == CBOR_END_OF_ARRAY) {
            switch (cbor_class) {
            case CBOR_T_UINT: /* unsigned integer */
            case CBOR_T_NINT: /* negative integer */
            case CBOR_T_FLOAT:
                current = NULL;
                *err = CBOR_MALFORMED_VALUE;
                break;
            case CBOR_T_BYTES:
            case CBOR_T_TEXT:
            case CBOR_T_ARRAY:
            case CBOR_T_MAP:
            case CBOR_T_TAGGED:
                /* TODO: manage these types */
                current = NULL;
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
                *p_out = cbor_print_int(out, out_max, val, 1);
                break;
            case CBOR_T_BYTES:
                /* TODO: manage these types */
                current = NULL;
                *err = CBOR_NOT_IMPLEMENTED;
                break;
            case CBOR_T_TEXT:
                /* TODO: manage these types */
                current = NULL;
                *err = CBOR_NOT_IMPLEMENTED;
                break;
            case CBOR_T_ARRAY:
                current = cbor_array_to_text(current, in_max, p_out, out_max, val, err);
                break;
            case CBOR_T_MAP:
                /* TODO: manage these types */
                current = NULL;
                *err = CBOR_NOT_IMPLEMENTED;
                break;
            case CBOR_T_TAGGED:
                /* TODO: manage these types */
                current = NULL;
                *err = CBOR_NOT_IMPLEMENTED;
                break;
            case CBOR_T_FLOAT:
                /* TODO: manage these types */
                current = NULL;
                *err = CBOR_NOT_IMPLEMENTED;
                break;
            }
        }
    }

    if (*p_out < out_max) {
        (*p_out)[0] = 0;
    }

    return current;
}

uint8_t* cbor_array_to_text(uint8_t* in, uint8_t const* in_max, char** p_out, char const* out_max, int64_t val, int* err)
{
    int not_first = 0;

    if (*p_out < out_max) {
        (*p_out)[0] = '[';
        *p_out += 1;
    }

    while (val > 0 && in != NULL) {
        val--;
        if (not_first && *p_out < out_max) {
            (*p_out)[0] = ',';
            *p_out += 1;
        }
        not_first = 1;
        in = cbor_to_text(in, in_max, p_out, out_max, err);
    }

    if (in != NULL && *p_out < out_max) {
        (*p_out)[0] = ']';
        *p_out += 1;
    }

    return in;
}


bool CborTest::DoOneTest(uint8_t* in, size_t in_length, char const* expected)
{
    bool ret = true;
    char buf[1024];
    int err = 0;
    char* p_buf;
    uint8_t* last;

    p_buf = &buf[0];

    last = cbor_to_text(in, in + in_length, &p_buf, buf + sizeof(buf), &err);

    if (err != 0) {
        TEST_LOG("Got error %d\n", err);
        ret = false;
    }
    else if (last == NULL) {
        TEST_LOG("Test returns NULL while error = %d\n", err);
        ret = false;
    }
    else if (last != in + in_length) {
        TEST_LOG("Decoded %d bytes instead of %d\n", (int)(last - in), (int)(in_length));
        ret = false;
    }
    else if (strcmp(buf, expected) != 0){
        TEST_LOG("Decoded <%s> instead of <%s>\n", buf, expected);
        ret = false;
    }

    return ret;
}

bool CborTest::DoTest()
{
    bool ret = true;

    ret &= DoOneTest(cbor_test1, sizeof(cbor_test1), cbor_test_out1);

    return ret;
}
