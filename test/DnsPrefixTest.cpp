/*
* Author: Christian Huitema
* Copyright (c) 2017, Private Octopus, Inc.
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
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "DnsStats.h"
#include "DnsPrefixTest.h"

DnsPrefixTest::DnsPrefixTest()
{
}

DnsPrefixTest::~DnsPrefixTest()
{
}

typedef struct _dns_prefix_test_case_st {
    const char * test_input;
    const char * expected;
} _dns_prefix_test_case_t;

static _dns_prefix_test_case_t dnsPrefixTestList[] = {
    { NULL, NULL },
    { "", NULL },
    { "COM", NULL },
    { "example.COM", "example.com" },
    { "WwW.example.COM", "example.com" },
    { ".com", NULL },
    { ".example", NULL },
    { ".example.com", NULL },
    { ".example.example", NULL },
    { "example", NULL },
    { "example.example", "example.example" },
    { "b.example.example", "example.example" },
    { "a.b.example.example", "example.example" },
    { "biz", NULL },
    { "domain.biz", "domain.biz" },
    { "b.domain.biz", "domain.biz" },
    { "a.b.domain.biz", "domain.biz" },
    { "com", NULL },
    { "example.com", "example.com" },
    { "b.example.com", "example.com" },
    { "a.b.example.com", "example.com" },
    { "uk.com", NULL },
    { "example.uk.com", "example.uk.com" },
    { "b.example.uk.com", "example.uk.com" },
    { "a.b.example.uk.com", "example.uk.com" },
    { "test.ac", "test.ac" },
    { "mm", NULL },
    { "c.mm", NULL },
    { "b.c.mm", "b.c.mm" },
    { "a.b.c.mm", "b.c.mm" },
    { "jp", NULL },
    { "test.jp", "test.jp" },
    { "www.test.jp", "test.jp" },
    { "ac.jp", NULL },
    { "test.ac.jp", "test.ac.jp" },
    { "www.test.ac.jp", "test.ac.jp" },
    { "kyoto.jp", NULL },
    { "test.kyoto.jp", "test.kyoto.jp" },
    { "ide.kyoto.jp", NULL },
    { "b.ide.kyoto.jp", "b.ide.kyoto.jp" },
    { "a.b.ide.kyoto.jp", "b.ide.kyoto.jp" },
    { "c.kobe.jp", NULL },
    { "b.c.kobe.jp", "b.c.kobe.jp" },
    { "a.b.c.kobe.jp", "b.c.kobe.jp" },
    { "city.kobe.jp", "city.kobe.jp" },
    { "www.city.kobe.jp", "city.kobe.jp" },
    { "ck", NULL },
    { "test.ck", NULL },
    { "b.test.ck", "b.test.ck" },
    { "a.b.test.ck", "b.test.ck" },
    { "www.ck", "www.ck" },
    { "www.www.ck", "www.ck" },
    { "us", NULL },
    { "test.us", "test.us" },
    { "www.test.us", "test.us" },
    { "ak.us", NULL },
    { "test.ak.us", "test.ak.us" },
    { "www.test.ak.us", "test.ak.us" },
    { "k12.ak.us", NULL },
    { "test.k12.ak.us", "test.k12.ak.us" },
    { "www.test.k12.ak.us", "test.k12.ak.us" },
    { "xn--85x722f.com.cn", "xn--85x722f.com.cn" },
    { "xn--85x722f.xn--55qx5d.cn", "xn--85x722f.xn--55qx5d.cn" },
    { "www.xn--85x722f.xn--55qx5d.cn", "xn--85x722f.xn--55qx5d.cn" },
    { "shishi.xn--55qx5d.cn", "shishi.xn--55qx5d.cn" },
    { "xn--55qx5d.cn", NULL },
    { "xn--85x722f.xn--fiqs8s", "xn--85x722f.xn--fiqs8s" },
    { "www.xn--85x722f.xn--fiqs8s", "xn--85x722f.xn--fiqs8s" },
    { "shishi.xn--fiqs8s", "shishi.xn--fiqs8s" },
    { "xn--fiqs8s", NULL }
};

static size_t nbDnsPrefixTest = sizeof(dnsPrefixTestList) / sizeof(_dns_prefix_test_case_t);

bool DnsPrefixTest::DoTest()
{
    bool ret = true;
    DnsStats stats;

    for (size_t i = 0; i < nbDnsPrefixTest; i++) {
        if (!DoOneTest(&stats, dnsPrefixTestList[i].test_input, dnsPrefixTestList[i].expected)) {
            ret = false;
        }
    }

    return ret;
}

char * DnsPrefixTest::ToUpper(char const * test_val, char * test_upper, size_t test_size)
{
    char * ret = NULL;

    if (test_val != NULL && test_size > 1) {
        for (size_t i = 0; i < test_size; i++) {
            if (test_val[i] == 0) {
                test_upper[i] = 0;
                break;
            } else if (islower(test_val[i])) {
                test_upper[i] = toupper(test_val[i]);
            } else {
                test_upper[i] = test_val[i];
            }
        }

        test_upper[test_size - 1] = 0;
        ret = test_upper;
    }

    return ret;
}

bool DnsPrefixTest::DoOneTest(DnsStats * stats, char const * test_input, char const * expected)
{
    bool ret = true;
    char tiu[256];
    char teu[256];
    char * ti;
    char * te;
    const char * tr;

    ti = ToUpper(test_input, tiu, sizeof(tiu));
    te = ToUpper(expected, teu, sizeof(teu));

    tr = stats->GetZonePrefix(tiu);

    if (tr == NULL) {
        if (te != NULL) {
            ret = false;
        }
    } else if (te == NULL) {
        ret = false;
    } else if (strcmp(tr, te) != 0) {
        ret = false;
    }

    if (!ret) {
        TEST_LOG("DnsPrefixTest fails for input <%s>, expected <%s>, got <%s>\n",
            (test_input == NULL) ? "NULL" : test_input,
            (te == NULL) ? "NULL" : te,
            (tr == NULL) ? "NULL" : tr);
    }
    return ret;
}
