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

// ithitest.cpp : Defines the entry point for the test application.
//

#include <stdio.h>
#include <stdlib.h>
#include "pcap_reader.h"
#include "DnsStats.h"
#include "getopt.h"
#include "CaptureSummary.h"
#include "ithimetrics.h"

#include "hashtest.h"
#include "testRfc6761.h"
#include "LoadTest.h"
#include "SaveTest.h"
#include "MergeTest.h"
#include "CaptureTest.h"
#include "MetricTest.h"
#include "PatternTest.h"
#include "PluginTest.h"

int main(int argc, char ** argv)
{
    bool ret = false;
    hashtest hash_test;
    testRfc6761 test6761;
    LoadTest load_test;
    SaveTest save_test;
    MergeTest merge_test;
    CaptureTest capture_test;
    MetricTest metric_test;
    PatternTest pattern_test;
    PluginTest plugin_test;

    if (!hash_test.DoTest())
    {
        fprintf(stderr, "hash test fails.\n");
    }
    else if (!test6761.DoTest())
    {
        fprintf(stderr, "RFC 6761 test fails.\n");
    }
    else if (!load_test.DoTest())
    {
        fprintf(stderr, "Load test fails.\n");
    }
    else if (!save_test.DoTest())
    {
        fprintf(stderr, "Save test fails.\n");
    }
    else if (!merge_test.DoTest())
    {
        fprintf(stderr, "Merge test fails.\n");
    }
    else if (!capture_test.DoTest())
    {
        fprintf(stderr, "Capture test fails.\n");
    }
    else if (!metric_test.DoTest())
    {
        fprintf(stderr, "Metric test fails.\n");
    }
    else if (!pattern_test.DoTest())
    {
        fprintf(stderr, "Pattern test fails.\n");
    }
    else if (!plugin_test.DoTest())
    {
        fprintf(stderr, "Plugin test fails.\n");
    }
    else
    {
        printf("All tests pass.\n");
        ret = true;
    }

    return (ret) ? 0 : -1;
}