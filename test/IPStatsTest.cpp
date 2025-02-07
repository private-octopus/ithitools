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
#include "ithiutil.h"
#include "ipstats.h"
#include "MetricTest.h"
#include "IPStatsTest.h"


#ifdef _WINDOWS
#ifndef _WINDOWS64
static char const * ipstats_test_input = "..\\data\\tiny-capture.cbor";
static char const * ipstats_test_output = "..\\data\\tiny-capture-ipstats.csv";
static char const * ipstats_xz_test_input = "..\\data\\tiny-capture.cbor.xz";
static char const * ipstats_test_output_2 = "..\\data\\tiny-capture-ipstats-2.csv";
#else
static char const * ipstats_test_input = "..\\..\\data\\tiny-capture.cbor";
static char const * ipstats_test_output = "..\\..\\data\\tiny-capture-ipstats.csv";
static char const * ipstats_xz_test_input = "..\\..\\data\\tiny-capture.cbor.xz";
static char const * ipstats_test_output_2 = "..\\..\\data\\tiny-capture-ipstats-2.csv";
#endif
#else
static char const * ipstats_test_input = "data/tiny-capture.cbor";
static char const * ipstats_test_output = "data/tiny-capture-ipstats.csv";
static char const * ipstats_xz_test_input = "data/tiny-capture.cbor.xz";
static char const * ipstats_test_output_2 = "data/tiny-capture-ipstats-2.csv";
#endif
static char const* ip_stats_csv = "tiny-capture-ipstats.csv";
static char const* ip_stats_csv_csv = "tiny-capture-ipstats-csv.csv";
static char const* ip_stats_xz_csv = "tiny-capture-ipstats-xz.csv";
static char const * ip_stats_merge_csv = "tiny-capture-ipstats-2.csv";
static char const* ip_stats_text_list = "tiny_capture_text_list.txt";
static char const* ip_stats_text_list_csv = "tiny_capture_text_list.csv";


bool IPStatsTestOne(
    char const * result_file,
    char const * ref_file,
    char const** input_files,
    size_t nb_input_files
)
{
    IPStats ipstats;
    bool ret = ipstats.LoadInputFiles(nb_input_files, input_files);

    if (!ret){
        TEST_LOG("Cannot process the input file: %s\n", input_files[0]);
    }
    else
    {
        ret = ipstats.SaveToCsv(result_file);
        if (!ret) {
            TEST_LOG("Cannot save to csv file: %s.\n", result_file);
        }
        else {
            TEST_LOG("IP Stats have been saved to %s\n", result_file);

            ret = MetricTest::compare_files(result_file, ref_file);
        }
    }

    return ret;
}

IPStatsTest::IPStatsTest()
{}

IPStatsTest::~IPStatsTest()
{}

bool IPStatsTest::DoTest()
{
    char const * list[1] = { ipstats_test_input };

    bool ret = IPStatsTestOne(ip_stats_csv, ipstats_test_output, list, 1);

    return ret;
}

IPStatsCsvTest::IPStatsCsvTest()
{}

IPStatsCsvTest::~IPStatsCsvTest()
{}

bool IPStatsCsvTest::DoTest()
{
    IPStats ipstats;
    char const * list[1] = { ipstats_test_output };

    bool ret = IPStatsTestOne(ip_stats_csv_csv, ipstats_test_output, list, 1);

    return ret;
}


IPStatsMergeTest::IPStatsMergeTest()
{}

IPStatsMergeTest::~IPStatsMergeTest()
{}

bool IPStatsMergeTest::DoTest()
{
    IPStats ipstats;
    char const * list[2] = { ipstats_test_output,  ipstats_test_output};

    bool ret = IPStatsTestOne(ip_stats_merge_csv, ipstats_test_output_2, list, 2);

    return ret;
}


IPStatsTextListTest::IPStatsTextListTest()
{}

IPStatsTextListTest::~IPStatsTextListTest()
{}

bool IPStatsTextListTest::DoTest()
{
    bool ret = true;
    IPStats ipstats;
    char const * list[2] = { ipstats_test_output,  ipstats_test_output};
    FILE* F = ithi_file_open(ip_stats_text_list, "wt");
    if (F == NULL) {
        ret = false;
    }
    else {
        for (int i = 0; ret && i < 2; i++) {
            ret &= fprintf(F, "%s\n", list[i]) > 0;
        }
        fclose(F);

        if (ret) {
            char const* l_txt[1] = { ip_stats_text_list };
            ret = IPStatsTestOne(ip_stats_text_list_csv, ipstats_test_output_2, l_txt, 1);
        }
    }
    return ret;
}

IPStatsXZTest::IPStatsXZTest()
{}

IPStatsXZTest::~IPStatsXZTest()
{}

bool IPStatsXZTest::DoTest()
{
#ifdef _WINDOWS
    return true;
#else
    IPStats ipstats;
    char const * list[1] = { ipstats_xz_test_input };

    bool ret = IPStatsTestOne(ip_stats_xz_csv, ipstats_test_output, list, 1);

    return ret;
#endif
}

IPStatsLoadTest::IPStatsLoadTest()
{
}

IPStatsLoadTest::~IPStatsLoadTest()
{
}

bool IPStatsLoadTest::DoTest()
{
#ifdef _WINDOWS
    return true;
#else
    bool ret;
    cdns cdns_ctx;
    FILE* P = NULL;
    int err;
    if (!(ret = cdns_ctx.open(ipstats_test_input))) {
        fprintf(stderr, "Cannot read file %s\n", ipstats_test_input);
    }
    else {
        cdns cdns_ctx_xz;

        P = ithi_xzcat_decompress_open(ipstats_xz_test_input, &err);

        if (P == NULL) {
            fprintf(stderr, "Cannot open pipe for %s, err = 0x%x\n", ipstats_xz_test_input, err);
        }
        else if (!(ret = cdns_ctx_xz.read_entire_file(P))) {
            fprintf(stderr, "Cannot decompress file %s\n", ipstats_xz_test_input);
        }
        else if (!(ret = (cdns_ctx_xz.buf_read == cdns_ctx.buf_read))) {
            fprintf(stderr, "Read lengths differ, %zu from xz vs %zu\n", cdns_ctx_xz.buf_read, cdns_ctx.buf_read);
        }
        else if (!(ret = (memcmp(cdns_ctx_xz.buf, cdns_ctx.buf, cdns_ctx.buf_read) == 0))) {
            fprintf(stderr, "Decompressed content differs from file content.\n");
        }
    }
    return ret;
#endif
}
