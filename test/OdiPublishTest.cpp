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

#include <stdint.h>
#include <string.h>
#ifdef _WINDOWS
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif
#include <errno.h>
#include "ithimetrics.h"
#include "OdiPublisher.h"
#include "MetricTest.h"
#include "OdiPublishTest.h"



OdiPublishTest::OdiPublishTest()
{
}


OdiPublishTest::~OdiPublishTest()
{
}

typedef struct st_odi_update_time_test_t {
    time_t unix_time;
    char const * iso_time_string;
} odi_update_time_test_t;

static time_t test_publish_time = 0x5b73baf4;

static const odi_update_time_test_t update_time_test[] = {
    { 0, "1970-01-01T00:00:00UTC" },
    { static_cast<time_t>(0x5b73baf4), "2018-08-15T05:32:36UTC"},
    { static_cast<time_t>(0xdeadbeef), "2088-05-20T21:55:59UTC"}
};

static const size_t nb_testupdate_time_test = sizeof(update_time_test) / sizeof(odi_update_time_test_t);

typedef struct st_odi_publish_test_t {
    char const * metric_file_in;
    char const * json_file_ref;
    char const * odi_folder;
    char const * odi_metric_file;
    char const * odi_json_file;
} odi_publish_test_t;

static const odi_publish_test_t odi_publish_test[] = {
    { "M1-2017-01-31.csv", "Ref-Odi-M1.json", "ITHI-M1", "20170131-2359.csv", "20170131-2359.csv-metadata.json" },
    { "M2-2017-02-28.csv", "Ref-Odi-M2.json", "ITHI-M2", "20170228-2359.csv", "20170228-2359.csv-metadata.json" },
    { "M3-2017-01-31.csv", "Ref-Odi-M3.json", "ITHI-M3", "20170131-2359.csv", "20170131-2359.csv-metadata.json" },
    { "M4-2017-02-28.csv", "Ref-Odi-M4.json", "ITHI-M4", "20170228-2359.csv", "20170228-2359.csv-metadata.json" },
    { "M5-2017-01-31.csv", "Ref-Odi-M5.json", "ITHI-M5", "20170131-2359.csv", "20170131-2359.csv-metadata.json" },
    { "M6-2017-02-28.csv", "Ref-Odi-M6.json", "ITHI-M6", "20170228-2359.csv", "20170228-2359.csv-metadata.json" },
    { "M7-2017-01-31.csv", "Ref-Odi-M7.json", "ITHI-M7", "20170131-2359.csv", "20170131-2359.csv-metadata.json" },
    { "M8-2017-02-28.csv", "Ref-Odi-M8.json", "ITHI-M8", "20170228-2359.csv", "20170228-2359.csv-metadata.json" }
};

static const size_t nb_odi_publish_test = sizeof(odi_publish_test) / sizeof(odi_publish_test_t);

bool OdiPublishTest::UpdateTimeTest()
{
    char buffer[256];
    bool ret = true;


    TEST_LOG("Current time: %x\n", time(0));

    for (size_t i = 0; i < nb_testupdate_time_test; i++) {
        bool x = OdiPublisher::GetUpdateTime(buffer, sizeof(buffer), update_time_test[i].unix_time);

        if (!x) {
            TEST_LOG("Cannot GetUpdateTime([%d]:%x)\n", i, (int)update_time_test[i].unix_time);
            ret = x;
        } else {
            if (strcmp(buffer, update_time_test[i].iso_time_string) != 0) {
                TEST_LOG("GetUpdateTime([%d]:%x) returns %s instead of %s\n", i,
                    (int)update_time_test[0].unix_time, buffer, update_time_test[i].iso_time_string);
                ret = false;
            }
        }
    }

    return ret;
}

bool OdiPublishTest::UpdatePublishTestOne(int test_id)
{
    bool ret = true;
    char input_file_name[512];
    char target_file_name[512];
    char ref_file_name[512];

    /* Compose the folder name, and ensure that it exists */
    if (ret) {
        ret = snprintf(target_file_name, sizeof(target_file_name), ".%s%s",
            ITHI_FILE_PATH_SEP, odi_publish_test[test_id].odi_folder);
    }

    if (ret) {
#ifdef _WINDOWS
        if (_mkdir(target_file_name) != 0) {
            int err = errno;
            ret = (err == EEXIST);
        }
#else
        if (mkdir(target_file_name, 0777) != 0) {
            ret = errno == EEXIST;
        }
#endif
    }

    /* Compose the input file name */
    if (ret) {
        ret = (snprintf(input_file_name, sizeof(input_file_name), "%s%s%s",
            ITHI_DEFAULT_DATA_FOLDER, ITHI_FILE_PATH_SEP, odi_publish_test[test_id].metric_file_in) > 0);
    }
    /* Publish at test time */
    if (ret) {
        ret = OdiPublisher::PublishMetricFile(input_file_name, ".", ITHI_DEFAULT_DATA_FOLDER, test_publish_time);
        if (!ret) {
            TEST_LOG("Cannot publish ODI files for input: %s\n", input_file_name);
        }
    }
    /* Compose the metric file name */
    if (ret) {
        ret = snprintf(target_file_name, sizeof(target_file_name), ".%s%s%s%s",
            ITHI_FILE_PATH_SEP, odi_publish_test[test_id].odi_folder,
            ITHI_FILE_PATH_SEP, odi_publish_test[test_id].odi_metric_file);
    }
    /* Verify that the metric file is present and matches the input */
    if (ret)
    {
        ret = MetricTest::compare_files(input_file_name, target_file_name);
    }
    /* Compose the JSON file name */
    if (ret) {
        ret = snprintf(target_file_name, sizeof(target_file_name), ".%s%s%s%s",
            ITHI_FILE_PATH_SEP, odi_publish_test[test_id].odi_folder,
            ITHI_FILE_PATH_SEP, odi_publish_test[test_id].odi_json_file);
    }
    /* Compose the JSON ref name */
    if (ret) {
        ret = snprintf(ref_file_name, sizeof(ref_file_name), "%s%s%s",
            ITHI_DEFAULT_DATA_FOLDER, ITHI_FILE_PATH_SEP, odi_publish_test[test_id].json_file_ref);
    }
    /* Verify that the JSON file is present and matches the input */
    if (ret)
    {
        ret = MetricTest::compare_files(ref_file_name, target_file_name);
    }

    return ret;
}

bool OdiPublishTest::DoTest()
{
    bool ret = true;

    /* First, test the update time computation */
    ret = UpdateTimeTest();

    /* Then check the different files */
    for (size_t test_id = 0; test_id < nb_odi_publish_test; test_id++) {
        ret &= UpdatePublishTestOne((int)test_id);
    }
    return ret;
}
