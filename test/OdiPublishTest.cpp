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
#include <OdiPublisher.h>
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

static const odi_update_time_test_t update_time_test[] = {
    {0, "1970-01-01T00:00:00UTC" },
    {0x5b73baf4, "2018-08-15T05:32:36UTC"},
    {0xdeadbeef, "2088-05-20T21:55:59UTC"}
};

static const size_t nb_testupdate_time_test = sizeof(update_time_test) / sizeof(odi_update_time_test_t);


typedef struct st_odi_publish_metric_t {
    time_t unix_time;
    char const * iso_time_string;
} st_odi_parse_file_t;

bool UpdateTimeTest()
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



bool OdiPublishTest::DoTest()
{
    bool ret = true;

    /* First, test the update time computation */
    ret = UpdateTimeTest();
    return ret;
}
