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

#ifndef CBOR_H
#define CBOR_H

#ifdef __cplusplus
extern "C" {
#endif

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

    uint8_t* cbor_get_number(uint8_t* in, uint8_t const* in_max, int64_t* val);
    char* cbor_print_int(char* out, char const* out_max, int64_t val, int is_negative);
    char* cbor_print_text_part(char* out, char const* out_max, uint8_t* in, int64_t val);
    char* cbor_print_bytes_part(char* out, char const* out_max, uint8_t* in, int64_t val);
    uint8_t* cbor_text_to_text(uint8_t* in, uint8_t const* in_max, char** p_out, char const* out_max, int64_t val, int* err);
    uint8_t* cbor_bytes_to_text(uint8_t* in, uint8_t const* in_max, char** p_out, char const* out_max, int64_t val, int* err);
    uint8_t* cbor_float_to_text(uint8_t* in, uint8_t const* in_max, char** p_out, char const* out_max, int64_t val, int* err);
    uint8_t* cbor_array_to_text(uint8_t* in, uint8_t const* in_max, char** p_out, char const* out_max, int64_t val, int is_map, int* err);
    uint8_t* cbor_to_text(uint8_t* in, uint8_t const* in_max, char** p_out, char const* out_max, int* err);

    uint8_t* cbor_skip(uint8_t* in, uint8_t const* in_max, int* err);

#ifdef __cplusplus
}
#endif

#endif
