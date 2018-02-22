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
static char const * publish_test_target = ".\\";
static char const * publish_test_target_m2 = ".\\M2Data.txt";
static char const * publish_test_target_m7 = ".\\M7Data.txt";
#ifndef ITHI_FILE_PATH_SEP
#define ITHI_FILE_PATH_SEP "\\"
#endif
#ifdef _WINDOWS64
static char const * publish_test_m21 = "..\\..\\data\\M7-2017-01-31.csv";
static char const * publish_test_m22 = "..\\..\\data\\M7-2017-02-28.csv";
static char const * publish_ref_m2 = "..\\..\\data\\M2Data-test-ref.txt";
static char const * publish_test_m71 = "..\\..\\data\\M7-2017-01-31.csv";
static char const * publish_test_m72 = "..\\..\\data\\M7-2017-02-28.csv";
static char const * publish_ref_m7 = "..\\..\\data\\M7Data-test-ref.txt";
#else
static char const * publish_test_m21 = "..\\data\\M2-2017-01-31.csv";
static char const * publish_test_m22 = "..\\data\\M2-2017-02-28.csv";
static char const * publish_ref_m2 = "..\\data\\M2Data-test-ref.txt";
static char const * publish_test_m71 = "..\\data\\M7-2017-01-31.csv";
static char const * publish_test_m72 = "..\\data\\M7-2017-02-28.csv";
static char const * publish_ref_m7 = "..\\data\\M7Data-test-ref.txt";
#endif
#else
static char const * metric_test_dir_ithi = "./ithi";
static char const * publish_test_target = "./";
static char const * publish_test_target_m2 = "./M2Data.txt";
static char const * publish_test_m21 = "./data/M2-2017-01-31.csv";
static char const * publish_test_m22 = "./data/M2-2017-02-28.csv";
static char const * publish_ref_m2 = "./data/M2Data-test-ref.txt";
static char const * publish_test_target_m7 = "./M7Data.txt";
static char const * publish_test_m71 = "./data/M7-2017-01-31.csv";
static char const * publish_test_m72 = "./data/M7-2017-02-28.csv";
static char const * publish_ref_m7 = "./data/M7Data-test-ref.txt";
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
        ithipublisher pub(publish_test_dir_ithi, metric_id, "2017-02-28");

        ret = pub.CollectMetricFiles();

        if (ret && pub.nb_months != nb_files)
        {
            ret = false;
        }

        if (ret && (pub.last_year != 2017 || pub.last_month != 2 || pub.last_day != 28))
        {
            ret = false;
        }

        if (ret)
        {
            ret = pub.Publish(publish_test_target);
        }

        if (ret)
        {
            ret = MetricTest::compare_files(target_file, ref_file);
        }
    }

    return ret;
}

bool PublishTest::DoTest()
{
    bool ret = true;
    /* M2 test */
    char const * m2_files[2] = { publish_test_m21, publish_test_m22 };

    if (ret)
    {
        ret = DoOneTest(2, m2_files, 2, publish_test_target_m2, publish_ref_m2);
    }

    /* M7 test */
    char const * m7_files[2] = { publish_test_m71, publish_test_m72 };

    if (ret)
    {
        ret = DoOneTest(7, m7_files, 2, publish_test_target_m7, publish_ref_m7);
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
