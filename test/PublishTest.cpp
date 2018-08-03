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

#include "ithipublisher.h"
#include "MetricTest.h"

#include "PublishTest.h"

#ifdef _WINDOWS
static char const * publish_test_dir_ithi = ".\\ithi";
static char const * publish_test_target = ".";
static char const * publish_test_target_m1 = ".\\M1Data.txt";
static char const * publish_test_target_m2 = ".\\M2Data.txt";
static char const * publish_test_target_m3 = ".\\M3Data.txt";
static char const * publish_test_target_m4 = ".\\M4Data.txt";
static char const * publish_test_target_m5 = ".\\M5Data.txt";
static char const * publish_test_target_m6 = ".\\M6Data.txt";
static char const * publish_test_target_m7 = ".\\M7Data.txt";
static char const * publish_test_target_m8 = ".\\M8Data.txt";
#ifndef ITHI_FILE_PATH_SEP
#define ITHI_FILE_PATH_SEP "\\"
#endif
#ifdef _WINDOWS64
static char const * publish_test_m11 = "..\\..\\data\\M1-2017-01-31.csv";
static char const * publish_test_m12 = "..\\..\\data\\M1-2017-02-28.csv";
static char const * publish_ref_m1 = "..\\..\\data\\M1Data-test-ref.txt";
static char const * publish_test_m21 = "..\\..\\data\\M7-2017-01-31.csv";
static char const * publish_test_m22 = "..\\..\\data\\M7-2017-02-28.csv";
static char const * publish_ref_m2 = "..\\..\\data\\M2Data-test-ref.txt";
static char const * publish_test_m31 = "..\\..\\data\\M3-2017-01-31.csv";
static char const * publish_test_m32 = "..\\..\\data\\M3-2017-02-28.csv";
static char const * publish_ref_m3 = "..\\..\\data\\M3Data-test-ref.txt";
static char const * publish_test_m41 = "..\\..\\data\\M4-2017-01-31.csv";
static char const * publish_test_m42 = "..\\..\\data\\M4-2017-02-28.csv";
static char const * publish_ref_m4 = "..\\..\\data\\M4Data-test-ref.txt";
static char const * publish_test_m51 = "..\\..\\data\\M5-2017-01-31.csv";
static char const * publish_test_m52 = "..\\..\\data\\M5-2017-02-28.csv";
static char const * publish_ref_m5 = "..\\..\\data\\M5Data-test-ref.txt";
static char const * publish_test_m61 = "..\\..\\data\\M6-2017-01-31.csv";
static char const * publish_test_m62 = "..\\..\\data\\M6-2017-02-28.csv";
static char const * publish_ref_m6 = "..\\..\\data\\M6Data-test-ref.txt";
static char const * publish_test_m71 = "..\\..\\data\\M7-2017-01-31.csv";
static char const * publish_test_m72 = "..\\..\\data\\M7-2017-02-28.csv";
static char const * publish_ref_m7 = "..\\..\\data\\M7Data-test-ref.txt";
static char const * publish_test_m81 = "..\\..\\data\\M8-2017-01-31.csv";
static char const * publish_test_m82 = "..\\..\\data\\M8-2017-02-28.csv";
static char const * publish_ref_m8 = "..\\..\\data\\M8Data-test-ref.txt";
#else
static char const * publish_test_m11 = "..\\data\\M1-2017-01-31.csv";
static char const * publish_test_m12 = "..\\data\\M1-2017-02-28.csv";
static char const * publish_ref_m1 = "..\\data\\M1Data-test-ref.txt";
static char const * publish_test_m21 = "..\\data\\M2-2017-01-31.csv";
static char const * publish_test_m22 = "..\\data\\M2-2017-02-28.csv";
static char const * publish_ref_m2 = "..\\data\\M2Data-test-ref.txt";
static char const * publish_test_m31 = "..\\data\\M3-2017-01-31.csv";
static char const * publish_test_m32 = "..\\data\\M3-2017-02-28.csv";
static char const * publish_ref_m3 = "..\\data\\M3Data-test-ref.txt";
static char const * publish_test_m41 = "..\\data\\M4-2017-01-31.csv";
static char const * publish_test_m42 = "..\\data\\M4-2017-02-28.csv";
static char const * publish_ref_m4 = "..\\data\\M4Data-test-ref.txt";
static char const * publish_test_m51 = "..\\data\\M5-2017-01-31.csv";
static char const * publish_test_m52 = "..\\data\\M5-2017-02-28.csv";
static char const * publish_ref_m5 = "..\\data\\M5Data-test-ref.txt";
static char const * publish_test_m61 = "..\\data\\M6-2017-01-31.csv";
static char const * publish_test_m62 = "..\\data\\M6-2017-02-28.csv";
static char const * publish_ref_m6 = "..\\data\\M6Data-test-ref.txt";
static char const * publish_test_m71 = "..\\data\\M7-2017-01-31.csv";
static char const * publish_test_m72 = "..\\data\\M7-2017-02-28.csv";
static char const * publish_ref_m7 = "..\\data\\M7Data-test-ref.txt";
static char const * publish_test_m81 = "..\\data\\M8-2017-01-31.csv";
static char const * publish_test_m82 = "..\\data\\M8-2017-02-28.csv";
static char const * publish_ref_m8 = "..\\data\\M8Data-test-ref.txt";
#endif
#else
static char const * publish_test_dir_ithi = "./ithi";
static char const * publish_test_target = ".";
static char const * publish_test_target_m1 = "./M1Data.txt";
static char const * publish_test_m11 = "./data/M1-2017-01-31.csv";
static char const * publish_test_m12 = "./data/M1-2017-02-28.csv";
static char const * publish_ref_m1 = "./data/M1Data-test-ref.txt";
static char const * publish_test_target_m2 = "./M2Data.txt";
static char const * publish_test_m21 = "./data/M2-2017-01-31.csv";
static char const * publish_test_m22 = "./data/M2-2017-02-28.csv";
static char const * publish_ref_m2 = "./data/M2Data-test-ref.txt";
static char const * publish_test_target_m3 = "./M3Data.txt";
static char const * publish_test_m31 = "./data/M3-2017-01-31.csv";
static char const * publish_test_m32 = "./data/M3-2017-02-28.csv";
static char const * publish_ref_m3 = "./data/M3Data-test-ref.txt";
static char const * publish_test_target_m4 = "./M4Data.txt";
static char const * publish_test_m41 = "./data/M4-2017-01-31.csv";
static char const * publish_test_m42 = "./data/M4-2017-02-28.csv";
static char const * publish_ref_m4 = "./data/M4Data-test-ref.txt";
static char const * publish_test_target_m5 = "./M5Data.txt";
static char const * publish_test_m51 = "./data/M5-2017-01-31.csv";
static char const * publish_test_m52 = "./data/M5-2017-02-28.csv";
static char const * publish_ref_m5 = "./data/M5Data-test-ref.txt";
static char const * publish_test_target_m6 = "./M6Data.txt";
static char const * publish_test_m61 = "./data/M6-2017-01-31.csv";
static char const * publish_test_m62 = "./data/M6-2017-02-28.csv";
static char const * publish_ref_m6 = "./data/M6Data-test-ref.txt";
static char const * publish_test_target_m7 = "./M7Data.txt";
static char const * publish_test_m71 = "./data/M7-2017-01-31.csv";
static char const * publish_test_m72 = "./data/M7-2017-02-28.csv";
static char const * publish_ref_m7 = "./data/M7Data-test-ref.txt";
static char const * publish_test_m81 = "./data/M8-2017-01-31.csv";
static char const * publish_test_m82 = "./data/M8-2017-02-28.csv";
static char const * publish_ref_m8 = "./data/M8Data-test-ref.txt";
#ifndef ITHI_FILE_PATH_SEP
#define ITHI_FILE_PATH_SEP "/"
#endif
#endif

PublishTest::PublishTest()
{
}


PublishTest::~PublishTest()
{
}

bool PublishTest::DoOneTest(int metric_id, char const ** metric_files, size_t nb_files,
    char const * target_file, char const * ref_file)
{

    bool ret = CreateTestDirectory(metric_id, metric_files, nb_files);

    if (ret) {
        ithipublisher pub(publish_test_dir_ithi, metric_id);

        ret = pub.CollectMetricFiles();
        if (!ret) 
        {
            TEST_LOG("For metric M%d, cannot collect metric files\n", metric_id);
        }

        if (ret && pub.nb_months != (int)nb_files)
        {
            ret = false;
            TEST_LOG("For metric M%d, got %d metric files instead of %d\n", metric_id,
                pub.nb_months, nb_files);
        }

        if (ret && (pub.last_year != 2017 || pub.last_month != 2 || pub.last_day != 28))
        {
            ret = false;
            TEST_LOG("For metric M%d, dates do not match, %d/%d/%d instead of 2017/02/28\n", metric_id,
                target_file, pub.last_year, pub.last_month, pub.last_day);
        }

        if (ret)
        {
            ret = pub.Publish(publish_test_target);
            if (!ret)
            {
                TEST_LOG("For metric M%d, cannot publish %s in %s\n", metric_id, 
                    target_file, publish_test_target);
            }
        }

        if (ret)
        {
            ret = MetricTest::compare_files(target_file, ref_file);
            if (!ret)
            {
                TEST_LOG("For metric M%d, %s != %s\n", metric_id, target_file, ref_file);
            }
        }
    }


    return ret;
}

bool PublishTest::DoTest()
{
    bool ret = true;

    /* M1 test */
    char const * m1_files[2] = { publish_test_m11, publish_test_m12 };

    if (ret)
    {
        ret = DoOneTest(1, m1_files, 2, publish_test_target_m1, publish_ref_m1);
    }

    /* M2 test */
    char const * m2_files[2] = { publish_test_m21, publish_test_m22 };

    if (ret)
    {
        ret = DoOneTest(2, m2_files, 2, publish_test_target_m2, publish_ref_m2);
    }

    /* M3 test */
    char const * m3_files[2] = { publish_test_m31, publish_test_m32 };

    if (ret)
    {
        ret = DoOneTest(3, m3_files, 2, publish_test_target_m3, publish_ref_m3);
    }

    /* M4 test */
    char const * m4_files[2] = { publish_test_m41, publish_test_m42 };

    if (ret)
    {
        ret = DoOneTest(4, m4_files, 2, publish_test_target_m4, publish_ref_m4);
    }


    /* M4 test */
    char const * m5_files[2] = { publish_test_m51, publish_test_m52 };

    if (ret)
    {
        ret = DoOneTest(5, m5_files, 2, publish_test_target_m5, publish_ref_m5);
    }

    /* M6 test */
    char const * m6_files[2] = { publish_test_m61, publish_test_m62 };

    if (ret)
    {
        ret = DoOneTest(6, m6_files, 2, publish_test_target_m6, publish_ref_m6);
    }

    /* M7 test */
    char const * m7_files[2] = { publish_test_m71, publish_test_m72 };

    if (ret)
    {
        ret = DoOneTest(7, m7_files, 2, publish_test_target_m7, publish_ref_m7);
    }

    /* M7 test */
    char const * m8_files[2] = { publish_test_m81, publish_test_m82 };

    if (ret)
    {
        ret = DoOneTest(8, m8_files, 2, publish_test_target_m8, publish_ref_m8);
    }

    return ret;
}


bool PublishTest::CreateTestDirectory(int metric_id, char const ** file_names, int nb_files)
{
    char dir_met_name[512];
    bool ret = snprintf(dir_met_name, sizeof(dir_met_name), "%s%sM%d%s", publish_test_dir_ithi, ITHI_FILE_PATH_SEP, metric_id, ITHI_FILE_PATH_SEP) > 0;
    
    if (ret)
    {
        ret = MetricCaptureFileTest::CreateDirectoryIfAbsent(dir_met_name);
    }

    for (int i = 0; i < nb_files; i++)
    {
        ret &= CopyFileToDirectory(file_names[i], dir_met_name);
    }

    return ret;
}

bool PublishTest::CopyFileToDirectory(char const * file_name, char const * dir_name)
{
    int file_name_index = 0;
    char dest_file_name[512];
    bool ret = true; 

    for (int i = 0; file_name[i] != 0; i++)
    {
        if (file_name[i] == ITHI_FILE_PATH_SEP[0])
        {
            file_name_index = i + 1;
        }
    }
    
    ret = snprintf(dest_file_name, sizeof(dest_file_name), "%s%s", dir_name,file_name + file_name_index) > 0;

    if (ret)
    {
        ret = MetricCaptureFileTest::CopyFileToDestination(dest_file_name, file_name);
    }

    return ret;
}
