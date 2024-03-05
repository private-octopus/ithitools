/*
* Author: Christian Huitema
* Copyright (c) 2024, Private Octopus, Inc.
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

#include "ithi_test_class.h"
#include "ipstats.h"
#include "MetricTest.h"
#include "IPStatsTest.h"


#ifdef _WINDOWS
#ifndef _WINDOWS64
static char const * ipstats_test_input = "..\\data\\tiny-capture.cbor";
static char const * ipstats_test_output = "..\\data\\ipstats-tiny-ref.csv";
#else
static char const * ipstats_test_input = "..\\..\\data\\tiny-capture.cbor";
static char const * ipstats_test_output = "..\\..\\data\\ipstats-tiny-ref.csv";
#endif
#else
static char const * ipstats_test_input = "data/tiny-capture.cbor";
static char const * ipstats_test_output = "data/ipstats-tiny-ref.csv";
#endif
static char const* ip_stats_csv = "tiny-capture-ipstats.csv";


IPStatsTest::IPStatsTest()
{}

IPStatsTest::~IPStatsTest()
{}

bool IPStatsTest::DoTest()
{
    IPStats ipstats;
    char const * list[1] = { ipstats_test_input };
    bool ret = ipstats.LoadCborFiles(1, list);

    if (!ret){
        TEST_LOG("Cannot process the CBOR input file: %s\n", list[0]);
    }
    else
    {
        ret = ipstats.SaveToCsv(ip_stats_csv);
        if (!ret) {
            TEST_LOG("Cannot save to csv file: %s.\n", ip_stats_csv);
        }
        else {
            TEST_LOG("IP Stats have been saved to %s\n", ip_stats_csv);
            
            ret = MetricTest::compare_files(ip_stats_csv, ipstats_test_output);
        }
    }

    return ret;
}

