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

    for (size_t i = 0; i < nb_cbor_tests; i++) {
        if (!DoOneTest(cbor_tests[i].in, cbor_tests[i].in_length, cbor_tests[i].expected)) {
            TEST_LOG("CBOR test #%d fails\n", (int)i);
            ret = false;
        }
    }

    if (ret) {
        TEST_LOG("All CBOR tests pass\n");
    }

    return ret;
}

CborSkipTest::CborSkipTest()
{
}

CborSkipTest::~CborSkipTest()
{
}


bool CborSkipTest::DoOneTest(uint8_t* in, size_t in_length, char const* expected)
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
        if (!DoOneTest(cbor_tests[i].in, cbor_tests[i].in_length, cbor_tests[i].expected)) {
            TEST_LOG("CBOR skip test #%d fails\n", (int)i);
            ret = false;
        }
    }

    if (ret) {
        TEST_LOG("All CBOR skip tests pass\n");
    }

    return ret;
}
