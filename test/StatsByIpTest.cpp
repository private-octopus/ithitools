/*
* Author: Christian Huitema
* Copyright (c) 2018, Private Octopus, Inc.
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

#include "AddressFilter.h"

#include "StatsByIpTest.h"



StatsByIpTest::StatsByIpTest()
{
}


StatsByIpTest::~StatsByIpTest()
{
}

uint8_t ip1[] = { 10, 0, 0, 1 };
uint8_t ip2[] = { 10, 0, 0, 2 };
uint8_t ip3[] = { 10, 0, 0, 3 };
uint8_t ip4[] = { 10, 0, 0, 4 };
uint8_t ip5[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
uint8_t ip6[] = { 0x20, 1, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8 };
uint8_t ip7[] = { 0x20, 1, 0, 0, 0, 0, 0, 0, 2, 3, 4, 5, 6, 7, 8, 1 };
uint8_t ip8[] = { 0x20, 1, 0, 0, 0, 0, 0, 0, 3, 4, 5, 6, 7, 8, 1, 2 };
uint8_t ip9[] = { 0x20, 1, 0, 0, 0, 0, 0, 0, 4, 5, 6, 7, 8, 1, 2, 3 };

typedef struct st_stats_by_ip_test_in_t {
    uint8_t * addr;
    size_t addr_len;
    bool has_do;
    bool has_edns;
    bool mini_qname;
    bool is_new;
    bool add_query;
} stats_by_ip_test_in_t;

static const stats_by_ip_test_in_t stats_by_ip_test_input[] = {
    { ip1, sizeof(ip1), false, false, false, false, true },
    { ip2, sizeof(ip2), false, false, true, true, false },
    { ip3, sizeof(ip3), false, true, false, false, true },
    { ip4, sizeof(ip4), false, true, true, true, false },
    { ip5, sizeof(ip5), true, false, false, false, true },
    { ip6, sizeof(ip6), true, false, true, true, false },
    { ip7, sizeof(ip7), true, true, false, false, true },
    { ip8, sizeof(ip8), true, true, true, true, false },
    { ip1, sizeof(ip1), false, false, false, false, true },
    { ip2, sizeof(ip2), false, false, true, false, false },
    { ip3, sizeof(ip3), false, true, false, false, true },
    { ip4, sizeof(ip4), false, true, true, false, false },
    { ip5, sizeof(ip5), true, false, false, false, true },
    { ip6, sizeof(ip6), true, false, true, false, false },
    { ip7, sizeof(ip7), true, true, false, false, false },
    { ip1, sizeof(ip1), false, false, false, false, false },
    { ip2, sizeof(ip2), false, false, true, false, false },
    { ip3, sizeof(ip3), false, true, false, false, false },
    { ip4, sizeof(ip4), false, true, true, false, false },
    { ip5, sizeof(ip5), true, false, false, false, false },
    { ip6, sizeof(ip6), true, false, true, false, false },
    { ip1, sizeof(ip1), false, false, false, false, false },
    { ip2, sizeof(ip2), false, false, true, false, false },
    { ip3, sizeof(ip3), false, true, false, false, false },
    { ip4, sizeof(ip4), false, true, true, false, false },
    { ip5, sizeof(ip5), true, false, false, false, false },
    { ip1, sizeof(ip1), false, false, false, false, false },
    { ip2, sizeof(ip2), false, false, true, false, false },
    { ip3, sizeof(ip3), false, true, false, false, false },
    { ip4, sizeof(ip4), false, true, true, false, false },
    { ip1, sizeof(ip1), false, false, false, false, false },
    { ip2, sizeof(ip2), false, false, true, false, false },
    { ip3, sizeof(ip3), false, true, false, false, false },
    { ip1, sizeof(ip1), false, false, false, false, false },
    { ip2, sizeof(ip2), false, false, true, false, false },
    { ip1, sizeof(ip1), false, false, false, false, false },
    { ip9, sizeof(ip9), false, false, false, false, true },
    { ip9, sizeof(ip9), false, false, true, false, false },
    { ip9, sizeof(ip9), false, true, false, false, false },
    { ip9, sizeof(ip9), false, true, true, false, false },
    { ip9, sizeof(ip9), true, false, false, false, false },
    { ip9, sizeof(ip9), true, false, true, false, false },
    { ip9, sizeof(ip9), true, true, false, false, false },
    { ip9, sizeof(ip9), true, true, true, false, false }
};

static const size_t nb_stats_by_ip_test_input = sizeof(stats_by_ip_test_input) / sizeof(stats_by_ip_test_in_t);

typedef struct st_stats_by_ip_test_out_t {
    uint8_t * addr;
    size_t addr_len;
    uint32_t count;
    uint32_t nb_do;
    uint32_t nb_edns;
    uint32_t nb_mini_qname;
    bool is_do_used;
    bool is_edns_supported;
    bool is_qname_minimized;
} stats_by_ip_test_out_t;

static const stats_by_ip_test_out_t stats_by_ip_test_output[] = {
    { ip1, sizeof(ip1), 9, 0, 0, 0, false, false, false },
    { ip2, sizeof(ip2), 7, 0, 0, 7, false, false, true },
    { ip3, sizeof(ip3), 7, 0, 6, 0, false, true, false },
    { ip4, sizeof(ip4), 5, 0, 5, 5, false, true, true },
    { ip5, sizeof(ip5), 5, 4, 0, 0, true, false, false },
    { ip6, sizeof(ip6), 3, 3, 0, 3, true, false, true },
    { ip7, sizeof(ip7), 3, 2, 2, 0, true, true, false },
    { ip8, sizeof(ip8), 1, 1, 1, 1, true, true, true },
    { ip9, sizeof(ip9), 9, 4, 4, 4, true, true, false }
};


static const size_t nb_stats_by_ip_test_output = sizeof(stats_by_ip_test_output) / sizeof(stats_by_ip_test_out_t);


bool StatsByIpTest::DoTest()
{
    bool ret = true;
    BinHash<StatsByIP> stats;
    StatsByIP * y = NULL;

    for (size_t i = 0; ret && i < nb_stats_by_ip_test_input; i++) {
        StatsByIP * x = new StatsByIP(
            stats_by_ip_test_input[i].addr,
            stats_by_ip_test_input[i].addr_len,
            stats_by_ip_test_input[i].has_do,
            stats_by_ip_test_input[i].has_edns,
            stats_by_ip_test_input[i].mini_qname);
        x->response_seen = true;

        if (x == NULL) {
            TEST_LOG("Cannot create StatsByIp for input #%d\n", (int)i);
            ret = false;
        }
        else {
            bool stored = false;
            if (stats_by_ip_test_input[i].add_query) {
                StatsByIP q (
                    stats_by_ip_test_input[i].addr,
                    stats_by_ip_test_input[i].addr_len,
                    false,
                    false,
                    false);
                q.query_seen = true;
                if (stats.Retrieve(&q) == NULL &&
                    stats.InsertOrAdd(&q, true, &stored) == NULL) {
                    TEST_LOG("Cannot add query #%d to hash table\n", (int)i);
                    ret = false;
                }
            }
            if (ret) {
                y = stats.InsertOrAdd(x, false, &stored);
                if (y == NULL) {
                    TEST_LOG("Cannot add input #%d to hash table\n", (int)i);
                    ret = false;
                }
                else {
                    if (stats_by_ip_test_input[i].is_new) {
                        if (!stored) {
                            TEST_LOG("test input #%d was not stored, expected stored!\n", (int)i);
                            ret = false;
                        }
                    }
                    else if (stored) {
                        TEST_LOG("test input #%d was stored, not expected!\n", (int)i);
                        ret = false;
                    }
                    if (!stored) {
                        delete x;
                    }
                }
            }
        }
    }

    for (size_t i = 0; ret && i < nb_stats_by_ip_test_output; i++) {
        StatsByIP x(
            stats_by_ip_test_output[i].addr,
            stats_by_ip_test_output[i].addr_len,
            false, false, false);
        
        y = stats.Retrieve(&x);

        if (y == NULL) {
            TEST_LOG("Cannot retrieve output case #%d\n", (int)i);
            ret = false;
        }
        else if (y->count != stats_by_ip_test_output[i].count) {
            TEST_LOG("Output case #%d, count = %d instead of %d\n", (int)i,
                y->count, stats_by_ip_test_output[i].count);
            ret = false;
        }
        else if (y->nb_do != stats_by_ip_test_output[i].nb_do) {
            TEST_LOG("Output case #%d, nb_do = %d instead of %d\n", (int)i,
                y->nb_do, stats_by_ip_test_output[i].nb_do);
            ret = false;
        }
        else if (y->nb_edns != stats_by_ip_test_output[i].nb_edns) {
            TEST_LOG("Output case #%d, nb_edns = %d instead of %d\n", (int)i,
                y->nb_edns, stats_by_ip_test_output[i].nb_edns);
            ret = false;
        }
        else if (y->nb_mini_qname != stats_by_ip_test_output[i].nb_mini_qname) {
            TEST_LOG("Output case #%d, nb_mini_qname = %d instead of %d\n", (int)i,
                y->nb_mini_qname, stats_by_ip_test_output[i].nb_mini_qname);
            ret = false;
        }
        else if (y->IsDoUsed() != stats_by_ip_test_output[i].is_do_used) {
            TEST_LOG("Output case #%d, IsDoUsed() = %d instead of %d\n", (int)i,
                y->IsDoUsed(), stats_by_ip_test_output[i].is_do_used);
            ret = false;
        }
        else if (y->IsEdnsSupported() != stats_by_ip_test_output[i].is_edns_supported) {
            TEST_LOG("Output case #%d, IsEdnsSupported() = %d instead of %d\n", (int)i,
                y->IsEdnsSupported(), stats_by_ip_test_output[i].is_edns_supported);
            ret = false;
        }
        else if (y->IsQnameMinimized() != stats_by_ip_test_output[i].is_qname_minimized) {
            TEST_LOG("Output case #%d, IsQnameMinimized() = %d instead of %d\n", (int)i,
                y->IsQnameMinimized(), stats_by_ip_test_output[i].is_qname_minimized);
            ret = false;
        }
    }

    return ret;
}
