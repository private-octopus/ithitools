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

#include "CborTest.h"


CborTest::CborTest()
{
}

CborTest::~CborTest()
{
}

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
char const* cbor_test_out1 = "[1,[2,3],[4,5]]";

/*
 *  9F        -- Start indefinite-length array
 *     01     -- 1
 *     82     -- Array of length 2
 *        02  -- 2
 *        03  -- 3
 *     9F     -- Start indefinite-length array
 *        04  -- 4
 *        05  -- 5
 *        FF  -- "break" (inner array)
 *     FF     -- "break" (outer array)
 */
uint8_t cbor_test2[] = { 0x9f, 0x01, 0x82, 0x02, 0x03, 0x9f, 0x04, 0x05, 0xff, 0xff };
char const* cbor_test_out2 = "[1,[2,3],[4,5]]";

/*
 *  9F        -- Start indefinite-length array
 *     01     -- 1
 *     82     -- Array of length 2
 *        02  -- 2
 *        03  -- 3
 *     82     -- Array of length 2
 *        04  -- 4
 *        05  -- 5
 *     FF     -- "break"
 */

uint8_t cbor_test3[] = { 0x9f, 0x01, 0x82, 0x02, 0x03, 0x82, 0x04, 0x05, 0xff };
char const* cbor_test_out3 = "[1,[2,3],[4,5]]";

/*
 *  83        -- Array of length 3
 *     01     -- 1
 *     82     -- Array of length 2
 *        02  -- 2
 *        03  -- 3
 *     9F     -- Start indefinite-length array
 *        04  -- 4
 *        05  -- 5
 *     FF  -- "break"
 */
uint8_t cbor_test4[] = { 0x83, 0x01, 0x82, 0x02, 0x03, 0x9f, 0x04, 0x05, 0xff };
char const* cbor_test_out4 = "[1,[2,3],[4,5]]";


/*
   0x83019f0203ff820405
   83        -- Array of length 3
      01     -- 1
      9F     -- Start indefinite-length array
         02  -- 2
         03  -- 3
         FF  -- "break"
      82     -- Array of length 2
         04  -- 4
         05  -- 5
         */
uint8_t cbor_test5[] = { 0x83, 0x01, 0x9f, 0x02, 0x03, 0xff, 0x82, 0x04, 0x05 };
char const* cbor_test_out5 = "[1,[2,3],[4,5]]";

/* Basic test string
 *   63        -- UTF-8 string length 3
 *      46756e --   "Fun"
 */
uint8_t cbor_test6[] = { 0x63, 0x46, 0x75, 0x6e };
char const* cbor_test_out6 = "\"Fun\"";

/* Basic octet string 
 *    44           --Byte string of length 4
 *       aabbccdd  -- Bytes content
 */

uint8_t cbor_test7[] = { 0x44, 0xaa, 0xbb, 0xcc, 0xdd };
char const* cbor_test_out7 = "0xaabbccdd";

/*
 * Indefinite length text string
 *   7F            -- Start indefinite - length byte string
 *       63        -- UTF-8 string length 3
 *          46756e --   "Fun"
 *       63        -- UTF-8 string length 3
 *          46756e --   "Fun"
 *       FF        -- "break"
 */

uint8_t cbor_test8[] = { 0x7F, 0x63, 0x46, 0x75, 0x6e, 0x63, 0x46, 0x75, 0x6e, 0xff };
char const* cbor_test_out8 = "\"FunFun\"";

/*
 * Indefinite length octet string
 * 5F              -- Start indefinite - length byte string
 *    44           --Byte string of length 4
 *       aabbccdd  -- Bytes content
 *    43           --Byte string of length 3
 *       eeff99    -- Bytes content
 * FF-- "break"
 */

uint8_t cbor_test9[] = { 0x5F, 0x44, 0xaa, 0xbb, 0xcc, 0xdd, 0x43, 0xee, 0xff, 0x99, 0xff };
char const* cbor_test_out9 = "0xaabbccddeeff99";

/*
   An example of an indefinite-length map (that happens to have two
   key/value pairs) might be:

   0xbf6346756ef563416d7421ff
   BF           -- Start indefinite-length map
      63        -- First key, UTF-8 string length 3
         46756e --   "Fun"
      80        -- First value, 0
      63        -- Second key, UTF-8 string length 3
         416d74 --   "Amt"
      21        -- -2
      FF        -- "break"
*/
uint8_t cbor_test10[] = { 0xbf, 0x63, 0x46, 0x75, 0x6e, 0x00, 0x63, 0x41, 0x6d, 0x74, 0x21, 0xff };
char const* cbor_test_out10 = "[\"Fun\",0,\"Amt\",-2]";

/*
   An other example of an indefinite-length map, including a boolean value:

   0xbf6346756ef563416d7421ff
   BF           -- Start indefinite-length map
      63        -- First key, UTF-8 string length 3
         46756e --   "Fun"
      F5        -- First value, true
      63        -- Second key, UTF-8 string length 3
         416d74 --   "Amt"
      21        -- -2
      FF        -- "break"
*/
uint8_t cbor_test11[] = { 0xbf, 0x63, 0x46, 0x75, 0x6e, 0xf5, 0x63, 0x41, 0x6d, 0x74, 0x21, 0xff };
char const* cbor_test_out11 = "[\"Fun\",true,\"Amt\",-2]";

typedef struct st_cbor_test_desc_t {
    uint8_t* in;
    size_t in_length;
    char const* expected;
} cbor_test_desc_t;

static cbor_test_desc_t cbor_tests[] = {
    { cbor_test1, sizeof(cbor_test1), cbor_test_out1},
    { cbor_test2, sizeof(cbor_test2), cbor_test_out2},
    { cbor_test3, sizeof(cbor_test3), cbor_test_out3},
    { cbor_test4, sizeof(cbor_test4), cbor_test_out4},
    { cbor_test5, sizeof(cbor_test5), cbor_test_out5}, 
    { cbor_test6, sizeof(cbor_test6), cbor_test_out6},
    { cbor_test7, sizeof(cbor_test7), cbor_test_out7},
    { cbor_test8, sizeof(cbor_test8), cbor_test_out8},
    { cbor_test9, sizeof(cbor_test9), cbor_test_out9},
    { cbor_test10, sizeof(cbor_test10), cbor_test_out10},
    { cbor_test11, sizeof(cbor_test11), cbor_test_out11}
};

size_t nb_cbor_tests = sizeof(cbor_tests) / sizeof(cbor_test_desc_t);

bool CborTest::DoOneDumpTest(uint8_t* in, size_t in_length, char const* expected)
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

static uint8_t int_test1[] = { 0x01 };
static uint8_t int_test2[] = { 0x18, 0x80 };
static uint8_t int_test3[] = { 0x19, 0x40, 0x01 };
static uint8_t int_test4[] = { 0x1a, 0x20, 0x80, 0x08, 0x01 };

typedef struct st_cbor_int_test_desc_t {
    uint8_t* in;
    size_t in_length;
    int expected;
} cbor_int_test_desc_t;

static cbor_int_test_desc_t int_tests[] = {
    {int_test1, sizeof(int_test1), 1},
    {int_test2, sizeof(int_test2), 0x80},
    {int_test3, sizeof(int_test3), 0x4001},
    {int_test4, sizeof(int_test4), 0x20800801}
};

static size_t nb_int_tests = sizeof(int_tests) / sizeof(cbor_int_test_desc_t);

bool CborTest::DoIntTest()
{
    uint8_t buf[256];
    size_t l = 0;
    bool ret = true;
    int err = 0;
    uint8_t* last;
    std::vector<int> v;

    buf[0] = 0x9F;
    l = 1;

    for (size_t i = 0; ret && i < nb_int_tests; i++) {
        int v;

        if (l + int_tests[i].in_length < sizeof(buf)) {
            memcpy(buf + l, int_tests[i].in, int_tests[i].in_length);
            l += int_tests[i].in_length;
        }

        last = cbor_object_parse(int_tests[i].in,
            int_tests[i].in + int_tests[i].in_length, &v, &err);

        if (err != 0) {
            TEST_LOG("Got error %d\n", err);
            ret = false;
        }
        else if (last == NULL) {
            TEST_LOG("Test returns NULL while error = %d\n", err);
            ret = false;
        }
        else if (last != int_tests[i].in + int_tests[i].in_length) {
            TEST_LOG("Decoded %d bytes instead of %d\n", (int)(last - int_tests[i].in),
                int_tests[i].in_length);
            ret = false;
        }
        else if (v != int_tests[i].expected) {
            TEST_LOG("Decoded %d instead of %d\n", v, int_tests[i].expected);
            ret = false;
        }
        if (!ret) {
            TEST_LOG("Int test %d fails\n", (int)i);
        }
    }

    if (ret) {
        for (int i = 0; i < 2; i++) {
            size_t ll;
            v.resize(0);

            if (i == 0) {
                ll = l + 1;
                if (ll < sizeof(buf)) {
                    buf[l] = 0xff;
                }
                else {
                    TEST_LOG("cannot run int array test %d\n", i);
                    ret = false;
                    break;
                }
            }
            else if (nb_int_tests < 0x1f) {
                buf[0] = (uint8_t)(nb_int_tests | 0x80);
                ll = l;
            }
            else {
                TEST_LOG("cannot run int array test %d\n", i);
                ret = false;
                break;
            }

            uint8_t* last = cbor_array_parse<int>(buf, buf + ll, &v, &err);
            if (err != 0) {
                TEST_LOG("Got error %d\n", err);
                ret = false;
            }
            else if (last == NULL) {
                TEST_LOG("Test returns NULL while error = %d\n", err);
                ret = false;
            }
            else if (last != buf + ll) {
                TEST_LOG("Decoded %d bytes instead of %d\n", (int)(last - buf), ll);
                ret = false;
            }
            else if (v.size() != nb_int_tests) {
                TEST_LOG("Decoded %d ints instead of %d\n", v.size(), nb_int_tests);
                ret = false;
            }
            else for (size_t x = 0; x < nb_int_tests; x++) {
                if (v[x] != int_tests[x].expected) {
                    TEST_LOG("Decoded v[%d]=%d instead of %d\n", (int)x, v[x], (int)int_tests[x].expected);
                    ret = false;
                    break;
                }
            }
            if (!ret) {
                TEST_LOG("Array of int test %d fails\n", i);
                break;
            }
        }
    }

    return ret;
}

static uint8_t bytes_test1[] = { 0x40 };
static uint8_t bytes_test2[] = { 0x41, 0x80 };
static uint8_t bytes_test3[] = { 0x42, 0x40, 0x01 };
static uint8_t bytes_test4[] = { 0x5f, 0x40, 0x41, 0x80, 0x42, 0x40, 0x01, 0xff };
static uint8_t bytes_expected2[] = { 0x80 };
static uint8_t bytes_expected3[] = { 0x40, 0x01 };
static uint8_t bytes_expected4[] = { 0x80, 0x40, 0x01 };


typedef struct st_cbor_bytes_test_desc_t {
    uint8_t* in;
    size_t in_length;
    uint8_t * expected;
    size_t expected_length;
} cbor_bytes_test_desc_t;

static cbor_bytes_test_desc_t bytes_tests[] = {
    {bytes_test1, sizeof(bytes_test1), NULL, 0},
    {bytes_test2, sizeof(bytes_test2), bytes_expected2, sizeof(bytes_expected2)},
    {bytes_test3, sizeof(bytes_test3), bytes_expected3, sizeof(bytes_expected3)},
    {bytes_test4, sizeof(bytes_test4), bytes_expected4, sizeof(bytes_expected4)}
};

static size_t nb_bytes_tests = sizeof(bytes_tests) / sizeof(cbor_bytes_test_desc_t);

bool CborTest::DoBytesTest()
{
    uint8_t buf[256];
    size_t l = 0;
    bool ret = true;
    int err = 0;
    uint8_t* last;
    std::vector<cbor_bytes> v;

    buf[0] = 0x9F;
    l = 1;

    for (size_t i = 0; ret && i < nb_bytes_tests; i++) {
        cbor_bytes v;

        if (l + bytes_tests[i].in_length < sizeof(buf)) {
            memcpy(buf + l, bytes_tests[i].in, bytes_tests[i].in_length);
            l += bytes_tests[i].in_length;
        }

        last = cbor_object_parse(bytes_tests[i].in,
            bytes_tests[i].in + bytes_tests[i].in_length, &v, &err);

        if (err != 0) {
            TEST_LOG("Got error %d\n", err);
            ret = false;
        }
        else if (last == NULL) {
            TEST_LOG("Test returns NULL while error = %d\n", err);
            ret = false;
        }
        else if (last != bytes_tests[i].in + bytes_tests[i].in_length) {
            TEST_LOG("Decoded %d bytes instead of %d\n", (int)(last - bytes_tests[i].in),
                bytes_tests[i].in_length);
            ret = false;
        }
        else if (v.l != bytes_tests[i].expected_length) {
            TEST_LOG("Decoded %d bytes instead of %d\n", v.l, bytes_tests[i].expected_length);
            ret = false;
        }
        else if (v.l != 0 && memcmp(bytes_tests[i].expected, v.v, v.l) != 0) {
            TEST_LOG("Decoded %d bytes do not match\n", v.l);
            ret = false;
        }
        if (!ret) {
            TEST_LOG("Bytes test %d fails\n", (int)i);
        }
    }

    if (ret) {
        for (int i = 0; i < 2; i++) {
            size_t ll;

            v.resize(0);

            if (i == 0) {
                ll = l + 1;
                if (ll < sizeof(buf)) {
                    buf[l] = 0xff;
                }
                else {
                    TEST_LOG("cannot run bytes array test %d\n", i);
                    ret = false;
                    break;
                }
            }
            else if (nb_int_tests < 0x1f) {
                buf[0] = (uint8_t)(nb_bytes_tests | 0x80);
                ll = l;
            }
            else {
                TEST_LOG("cannot run bytes array test %d\n", i);
                ret = false;
                break;
            }

            uint8_t* last = cbor_array_parse<cbor_bytes>(buf, buf + ll, &v, &err);
            if (err != 0) {
                TEST_LOG("Got error %d\n", err);
                ret = false;
            }
            else if (last == NULL) {
                TEST_LOG("Test returns NULL while error = %d\n", err);
                ret = false;
            }
            else if (last != buf + ll) {
                TEST_LOG("Decoded %d bytes instead of %d\n", (int)(last - buf), ll);
                ret = false;
            }
            else if (v.size() != nb_bytes_tests) {
                TEST_LOG("Decoded %d byte fields instead of %d\n", v.size(), nb_bytes_tests);
                ret = false;
            }
            else for (size_t x = 0; x < nb_int_tests; x++) {
                if (v[x].l != bytes_tests[x].expected_length) {
                    TEST_LOG("Decoded v[%d].l =%d ints instead of %d\n", (int)x, v[x].l, 
                        (int)bytes_tests[x].expected_length);
                    ret = false;
                    break;
                }
                if (v[x].l != 0 && memcmp(bytes_tests[x].expected, v[x].v, v[x].l) != 0) {
                    TEST_LOG("For v[%d], decoded %d bytes do not match\n", (int)x, v[x].l);
                    ret = false;
                    break;
                }
            }
            if (!ret) {
                TEST_LOG("Array of bytes test %d fails\n", i);
                break;
            }
        }
    }

    return ret;
}

class cbor_map_test
{
public:
    cbor_map_test():
        a(0),
        b(0),
        c(0)
    {}

    cbor_map_test(int a, int b, int c) :
        a(a),
        b(b),
        c(c)
    {}

    ~cbor_map_test()
    {}

    uint8_t * parse(uint8_t* in, uint8_t const* in_max, int* err)
    {
        return cbor_map_parse(in, in_max, this, err);
    }

    uint8_t* parse_map_item(uint8_t* in, uint8_t const* in_max, int64_t index, int* err)
    {
        switch (index) {
        case 0:
            in = cbor_object_parse(in, in_max, &a, err);
            break;
        case 1:
            in = cbor_object_parse(in, in_max, &b, err);
            break;
        case 2:
            in = cbor_object_parse(in, in_max, &c, err);
            break;
        default:
            in = NULL;
            *err = CBOR_ILLEGAL_VALUE;
            break;
        }

        return in;
    }

    int a;
    int b;
    int c;
};

static uint8_t map_test1[] = { 0xa1, 0x00, 0x05 };
static uint8_t map_test2[] = { 0xa1, 0x01, 0x07 };
static uint8_t map_test3[] = { 0xa2, 0x00, 0x03, 0x02, 0x09};
static uint8_t map_test4[] = { 0xbf, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0xff };


typedef struct st_cbor_map_test_desc_t {
    uint8_t* in;
    size_t in_length;
    cbor_map_test expected;
} cbor_map_test_desc_t;

static cbor_map_test_desc_t map_tests[] = {
    {map_test1, sizeof(map_test1), cbor_map_test(5,0,0)},
    {map_test2, sizeof(map_test2), cbor_map_test(0,7,0)},
    {map_test3, sizeof(map_test3), cbor_map_test(3,0,9)},
    {map_test4, sizeof(map_test4), cbor_map_test(1,2,3)}
};

static size_t nb_map_tests = sizeof(map_tests) / sizeof(cbor_map_test_desc_t);

bool CborTest::DoMapTest()
{
    uint8_t buf[256];
    size_t l = 0;
    bool ret = true;
    int err = 0;
    uint8_t* last;
    std::vector<cbor_map_test> v;

    buf[0] = 0x9F;
    l = 1;

    for (size_t i = 0; ret && i < nb_bytes_tests; i++) {
        cbor_map_test v;

        if (l + map_tests[i].in_length < sizeof(buf)) {
            memcpy(buf + l, map_tests[i].in, map_tests[i].in_length);
            l += map_tests[i].in_length;
        }

        last = cbor_object_parse(map_tests[i].in,
            map_tests[i].in + map_tests[i].in_length, &v, &err);

        if (err != 0) {
            TEST_LOG("Got error %d\n", err);
            ret = false;
        }
        else if (last == NULL) {
            TEST_LOG("Test returns NULL while error = %d\n", err);
            ret = false;
        }
        else if (last != map_tests[i].in + map_tests[i].in_length) {
            TEST_LOG("Decoded %d bytes instead of %d\n", (int)(last - map_tests[i].in),
                map_tests[i].in_length);
            ret = false;
        }
        else if (v.a != map_tests[i].expected.a ||
            v.b != map_tests[i].expected.b ||
            v.c != map_tests[i].expected.c
            ) {
            TEST_LOG("Decoded (%d,%d,%d) instead of (%d,%d,%d)\n",
                v.a, v.b, v.c, map_tests[i].expected.a, map_tests[i].expected.b, map_tests[i].expected.c);
            ret = false;
        }

        if (!ret) {
            TEST_LOG("Bytes test %d fails\n", (int)i);
        }
    }

    if (ret) {
        for (int i = 0; i < 2; i++) {
            size_t ll;

            v.resize(0);

            if (i == 0) {
                ll = l + 1;
                if (ll < sizeof(buf)) {
                    buf[l] = 0xff;
                }
                else {
                    TEST_LOG("cannot run map array test %d\n", i);
                    ret = false;
                    break;
                }
            }
            else if (nb_int_tests < 0x1f) {
                buf[0] = (uint8_t)(nb_map_tests | 0x80);
                ll = l;
            }
            else {
                TEST_LOG("cannot run bytes array test %d\n", i);
                ret = false;
                break;
            }

            uint8_t* last = cbor_array_parse<cbor_map_test>(buf, buf + ll, &v, &err);
            if (err != 0) {
                TEST_LOG("Got error %d\n", err);
                ret = false;
            }
            else if (last == NULL) {
                TEST_LOG("Test returns NULL while error = %d\n", err);
                ret = false;
            }
            else if (last != buf + ll) {
                TEST_LOG("Decoded %d bytes instead of %d\n", (int)(last - buf), ll);
                ret = false;
            }
            else if (v.size() != nb_bytes_tests) {
                TEST_LOG("Decoded %d maps instead of %d\n", v.size(), nb_bytes_tests);
                ret = false;
            }
            else for (size_t x = 0; x < nb_int_tests; x++) {
                if (v[x].a != map_tests[x].expected.a ||
                    v[x].b != map_tests[x].expected.b ||
                    v[x].c != map_tests[x].expected.c
                    ) {
                    TEST_LOG("Decoded (%d,%d,%d) instead of (%d,%d,%d)\n",
                        v[x].a, v[x].b, v[x].c, map_tests[x].expected.a, map_tests[x].expected.b, map_tests[x].expected.c);
                    ret = false;
                }
            }
            if (!ret) {
                TEST_LOG("Array of map test %d fails\n", i);
                break;
            }
        }
    }

    return ret;
}

bool CborTest::DoTest()
{
    bool ret = true;

    for (size_t i = 0; i < nb_cbor_tests; i++) {
        if (!DoOneDumpTest(cbor_tests[i].in, cbor_tests[i].in_length, cbor_tests[i].expected)) {
            TEST_LOG("CBOR test #%d fails\n", (int)i);
            ret = false;
        }
    }

    if (ret) {
        TEST_LOG("All CBOR dump tests pass\n");
    }

    if (ret) {
        ret = DoIntTest();
        if (ret) {
            TEST_LOG("All int parse tests pass\n");
        }
    }

    if (ret) {
        ret = DoBytesTest();
        if (ret) {
            TEST_LOG("All bytes parse tests pass\n");
        }
    }

    if (ret) {
        ret = DoMapTest();
        if (ret) {
            TEST_LOG("All map parse tests pass\n");
        }
    }

    return ret;
}

CborSkipTest::CborSkipTest()
{
}

CborSkipTest::~CborSkipTest()
{
}


bool CborSkipTest::DoOneTest(uint8_t* in, size_t in_length)
{
    bool ret = true;
    int err = 0;
    uint8_t* last;

    last = cbor_skip(in, in + in_length, &err);

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

    return ret;
}

bool CborSkipTest::DoTest()
{
    bool ret = true;

    for (size_t i = 0; i < nb_cbor_tests; i++) {
        if (!DoOneTest(cbor_tests[i].in, cbor_tests[i].in_length)) {
            TEST_LOG("CBOR skip test #%d fails\n", (int)i);
            ret = false;
        }
    }

    if (ret) {
        TEST_LOG("All CBOR skip tests pass\n");
    }

    return ret;
}
