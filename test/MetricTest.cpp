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

#include "ithimetrics.h"
#include "MetricTest.h"

#ifdef _WINDOWS
static char const * metric_test_input = "..\\data\\tiny-capture.csv";
static char const * metric_test_output = "metric-test.csv";
static char const * metric_test_ref = "..\\data\\tiny-metrics.csv";
#else
static char const * metric_test_input = "data/tiny-capture.csv";
static char const * metric_test_output = "metric-test.csv";
static char const * metric_test_ref = "data/tiny-metrics.csv";
#endif


MetricTest::MetricTest()
{
}


MetricTest::~MetricTest()
{
}

bool MetricTest::DoTest()
{
    CaptureSummary cs;
    bool ret = cs.Load(metric_test_input);

    if (ret)
    {
        ithimetrics met;

        ret = met.GetMetrics(&cs);

        if (ret)
        {
            ret = met.Save(metric_test_output);

            if (ret)
            {
                ret = compare_files(metric_test_output, metric_test_ref);
            }
        }
    }

    return ret;
}

bool MetricTest::compare_files(char const * fname1, char const * fname2)
{
    FILE* F1 = NULL;
    FILE* F2 = NULL;

#ifdef _WINDOWS
    errno_t err = fopen_s(&F1, fname1, "r");
    bool ret = (err == 0);

    if (ret)
    {
        err = fopen_s(&F2, fname2, "r");
        ret = (err == 0);
    }
#else
    bool ret;
    F1 = fopen(fname1, "r");
    ret = (F1 != NULL);
    if (ret)
    {
        F2 = fopen(fname2, "r");
        ret = (F2 != NULL);
    }
#endif
    if (ret)
    {
        char buffer1[256];
        char buffer2[256];

        while (ret && fgets(buffer1, sizeof(buffer1), F1) != NULL)
        {
            if (fgets(buffer2, sizeof(buffer2), F2) == NULL)
            {
                /* F2 is too short */
                ret = false;
            }
            else
            {
                ret = compare_lines(buffer1, buffer2);
            }
        }

        if (ret && fgets(buffer2, sizeof(buffer2), F2) != NULL)
        {
            /* F2 is too long */
            ret = false;
        }
    }


    if (F1 != NULL)
    {
        fclose(F1);
    }

    if (F2 != NULL)
    {
        fclose(F2);
    }

    return ret;
}

bool MetricTest::compare_lines(char const * b1, char const * b2)
{
    bool ret = true;

    while (*b1 != 0 && *b2 != 0)
    {
        if (*b1 != *b2)
        {
            break;
        }
        b1++;
        b2++;
    }

    while (*b1 == '/n' || *b1 == '/r')
    {
        b1++;
    }

    while (*b2 == '/n' || *b2 == '/r')
    {
        b2++;
    }

    return (*b1 == 0 && *b2 == 0);
}
