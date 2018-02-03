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

#ifdef _WINDOWS
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#endif
#include <stdio.h>
#include <string.h>
#include "ithimetrics.h"
#include "MetricTest.h"

#ifdef _WINDOWS
#ifndef _WINDOWS64
static char const * metric_test_input = "..\\data\\tiny-capture.csv";
static char const * metric_test_output = "metric-test.csv";
static char const * metric_test_ref = "..\\data\\tiny-metrics.csv";
static char const * root_zone_file = "..\\data\\root.zone";

#else
static char const * metric_test_input = "..\\..\\data\\tiny-capture.csv";
static char const * metric_test_output = "metric-test.csv";
static char const * metric_test_ref = "..\\..\\data\\tiny-metrics.csv";
static char const * root_zone_file = "..\\..\\data\\root.zone";
#endif
#else
static char const * metric_test_input = "data/tiny-capture.csv";
static char const * metric_test_output = "metric-test.csv";
static char const * metric_test_ref = "data/tiny-metrics.csv";
static char const * root_zone_file = "data/root.zone";
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

    if (!ret)
    {
        TEST_LOG("Could not load input from %s\n", metric_test_input);
    }

    if (ret)
    {
        ithimetrics met;

        ret = met.GetM7(root_zone_file); 
        if (!ret)
        {
            TEST_LOG("Could not load M7 from %s\n", root_zone_file);
        }
        
        if (ret)
        {
            ret = met.GetMetrics(&cs);
            if (!ret)
            {
                TEST_LOG("Could not get metrics out of %s\n", metric_test_input);
            }
        }

        if (ret)
        {
            ret = met.Save(metric_test_output);

            if (ret)
            {
                ret = compare_files(metric_test_output, metric_test_ref);
            }
            else
            {
                TEST_LOG("Could not save metrics to %s\n", metric_test_output);
            }
        }
    }

    if (ret)
    {
        TEST_LOG("Saved metrics to %s\n", metric_test_output);
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
    if (F1 == NULL)
    {
        TEST_LOG("Cannot open %s.\n", fname1);
    }

    if (F2 == NULL)
    {
        TEST_LOG("Cannot open %s.\n", fname2);
    }

    if (ret && F1 != NULL && F2 != NULL)
    {
        char buffer1[256];
        char buffer2[256];

        while (ret && fgets(buffer1, sizeof(buffer1), F1) != NULL)
        {
            if (fgets(buffer2, sizeof(buffer2), F2) == NULL)
            {
                /* F2 is too short */
                TEST_LOG("File comparison fails - %s is too short.\n", fname2);
                TEST_LOG("    Missing line: %s", buffer1);
                ret = false;
            }
            else
            {
                ret = compare_lines(buffer1, buffer2);
                if (!ret)
                {
                    TEST_LOG("File comparison fails - different value:\n");
                    TEST_LOG("    %s", buffer1); 
                    TEST_LOG(" vs %s", buffer2);
                }
            }
        }

        if (ret && fgets(buffer2, sizeof(buffer2), F2) != NULL)
        {
            /* F2 is too long */
            TEST_LOG("File comparison fails - %s is too long.\n", fname2);
            TEST_LOG("    Extra line: %s", buffer2);
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

    while (*b1 == '\n' || *b1 == '\r')
    {
        b1++;
    }

    while (*b2 == '\n' || *b2 == '\r')
    {
        b2++;
    }

    return (*b1 == 0 && *b2 == 0);
}

MetricDateTest::MetricDateTest()
{
}

MetricDateTest::~MetricDateTest()
{
}

static char const * metric_date_test_jan_31_2017 = "2017-01-31";
static time_t metric_date_time_t_jan_31_2017 = 1485896024;

bool MetricDateTest::DoTest()
{
    ithimetrics met;
    bool ret;

    ret = met.SetDefaultDate(metric_date_time_t_jan_31_2017);

    if (ret)
    {
        if (met.GetMetricDate() == NULL)
        {
            TEST_LOG("Metric test: cannot get metric's date.\n");
            ret = false;
        }
        else
        {
            if (strcmp(metric_date_test_jan_31_2017, met.GetMetricDate()) != 0)
            {
                TEST_LOG("Metric test: date value is not test date, 2017/01/31.\n");
                ret = false;
            }
        }
    }

    return ret;
}

MetricCaptureFileTest::MetricCaptureFileTest()
{
}

MetricCaptureFileTest::~MetricCaptureFileTest()
{
}

#ifdef _WINDOWS
static char const * metric_test_dir_ithi = ".\\ithi";
static char const * metric_test_dir_input = ".\\ithi\\input";
static char const * metric_test_dir_m346 = ".\\ithi\\input\\M346";
static char const * metric_test_dir_m346_l1 = ".\\ithi\\input\\M346\\b-root-lax";
static char const * metric_test_dir_m346_l2 = ".\\ithi\\input\\M346\\b-root-mia";
static char const * metric_test_dir_m346_l3 = ".\\ithi\\input\\M346\\ams-nic";

static char const * metric_test_dir_m346_l1_f = ".\\ithi\\input\\M346\\b-root-lax\\M346-2017-01-31-b-root-lax.csv";
static char const * metric_test_dir_m346_l2_b = ".\\ithi\\input\\M346\\b-root-mia\\M346-2017-01-30-b-root-mia.csv";
static char const * metric_test_dir_m346_l2_f = ".\\ithi\\input\\M346\\b-root-mia\\M346-2017-01-31-b-root-mia.csv";
static char const * metric_test_dir_m346_b = ".\\ithi\\input\\M346\\M346-2017-01-30.csv";

#ifdef _WINDOWS64
static char const * metric_test_capture = "..\\..\\data\\tiny-capture.csv";
#else
static char const * metric_test_capture = "..\\data\\tiny-capture.csv";
#endif

#else
static char const * metric_test_dir_ithi = "./ithi";
static char const * metric_test_dir_input = "./ithi/input";
static char const * metric_test_dir_m346 = "./ithi/input/M346";
static char const * metric_test_dir_m346_l1 = "./ithi/input/M346/b-root-lax";
static char const * metric_test_dir_m346_l2 = "./ithi/input/M346/b-root-mia";
static char const * metric_test_dir_m346_l3 = "./ithi/input/M346/ams-nic";

static char const * metric_test_dir_m346_l1_f = "./ithi/input/M346/b-root-lax/M346-2017-01-31-b-root-lax.csv";
static char const * metric_test_dir_m346_l2_b = "./ithi/input/M346/b-root-mia/M346-2017-01-30-b-root-mia.csv";
static char const * metric_test_dir_m346_l2_f = "./ithi/input/M346/b-root-mia/M346-2017-01-31-b-root-mia.csv";
static char const * metric_test_dir_m346_b = "./ithi/input/M346/M346-2017-01-30.csv";

static char const * metric_test_capture = "data/tiny-capture.csv";
#endif

static const char * metric_test_dir_list[] = {
    metric_test_dir_ithi, metric_test_dir_input, metric_test_dir_m346,
    metric_test_dir_m346_l1, metric_test_dir_m346_l2, metric_test_dir_m346_l3
};

static const char * metric_test_file_list[] = {
    metric_test_dir_m346_l1_f, metric_test_dir_m346_l2_b, metric_test_dir_m346_l2_f, metric_test_dir_m346_b
};

bool MetricCaptureFileTest::DoTest()
{
    bool ret = true;
    ithimetrics met;

    /* First, prepare the ITHI directories in the current folder 
     * Use three locations for the capture summaries */
    for (size_t i = 0; ret && i < sizeof(metric_test_dir_list) / sizeof(char *); i++)
    {
        ret = CreateDirectoryIfAbsent(metric_test_dir_list[i]);
    }

    /* Copy the default capture test file at the appropriate locations,
     * with appropriate names */
    for (size_t i = 0; ret && i < sizeof(metric_test_file_list) / sizeof(char *); i++)
    {
        ret = CopyFileToDestination(metric_test_file_list[i], metric_test_capture);
    }

    /* Set the target directory and target date in the metric object */
    if (ret)
    {
        ret = met.SetIthiFolder(metric_test_dir_ithi);
    }

    if (ret)
    {
        ret = met.SetDateString(metric_date_test_jan_31_2017);
    }

    /* Obtain the default capture files for the date. */
    if (ret)
    {
        ret = met.SetDefaultCaptureFiles();
    }

    /* Verify that they match expectations */
    if (ret)
    {
        if (met.GetNbCaptureFiles() != 2)
        {
            ret = false;
        }
        else if (strcmp(metric_test_dir_m346_l1_f, met.GetCaptureFileName(0)) != 0)
        {
            ret = false;
        }
        else if (strcmp(metric_test_dir_m346_l2_f, met.GetCaptureFileName(1)) != 0)
        {
            ret = false;
        }
    }

    if (!ret)
    {
        TEST_LOG("Metric test: cannot create and populate the test directory %s.\n",
            metric_test_dir_ithi);
    }

    return ret;
}

bool MetricCaptureFileTest::CreateDirectoryIfAbsent(char const * dir_name)
{
    bool ret = true;
#ifdef _WINDOWS
    if (_mkdir(dir_name) != 0)
    {
        if (errno == EEXIST)
        {
            ret = true;
        }
        else
        {
            ret = false;
        }
    }
#else
    if (mkdir(dir_name, 0777) != 0)
    {
        if (errno == EEXIST)
        {
            ret = true;
        }
        else
        {
            ret = false;
        }
    }
#endif
    return ret;
}

bool MetricCaptureFileTest::CopyFileToDestination(char const * target_name, char const * source_name)
{
    FILE* F1 = NULL;
    FILE* F2 = NULL;

#ifdef _WINDOWS
    errno_t err = fopen_s(&F1, source_name, "r");
    bool ret = (err == 0);

    if (ret)
    {
        err = fopen_s(&F2, target_name, "w");
        ret = (err == 0);
    }
#else
    bool ret;
    F1 = fopen(source_name, "r");
    ret = (F1 != NULL);
    if (ret)
    {
        F2 = fopen(target_name, "w");
        ret = (F2 != NULL);
    }
#endif

    if (ret && F1 != NULL && F2 != NULL)
    {
        char buffer[512];

        while (fgets(buffer, sizeof(buffer), F1) != NULL)
        {
            if (fputs(buffer, F2) == EOF)
            {
                /* Could not write on F2 */
                ret = false;
                break;
            }
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
