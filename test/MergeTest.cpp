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

#include "ithiutil.h"
#include "CaptureSummary.h"
#include "MergeTest.h"

#ifdef _WINDOWS
#ifndef _WINDOWS64
static char const * summary1 = "..\\data\\summary1.csv";
static char const * summary2 = "..\\data\\summary2.csv";
static char const * target = "..\\data\\merge-1-2.csv";
#else
static char const * summary1 = "..\\..\\data\\summary1.csv";
static char const * summary2 = "..\\..\\data\\summary2.csv";
static char const * target = "..\\..\\data\\merge-1-2.csv";
#endif
#else
static char const * summary1 = "data/summary1.csv";
static char const * summary2 = "data/summary2.csv";
static char const * target = "data/merge-1-2.csv";
#endif

MergeTest::MergeTest()
{
}


MergeTest::~MergeTest()
{
}

bool MergeTest::DoTest()
{
    CaptureSummary cs;
    char const * list[2] = { summary1, summary2 };

    bool ret = cs.Merge(2, list);

    if (ret)
    {
        CaptureSummary tcs;

        ret = tcs.Load(target);

        if (ret)
        {
            tcs.Sort();
            ret = ithi_test_class::CompareCS(&cs, &tcs);
        }
    }

    return ret;
}

char const * capture_list_name = "capture_list.txt";

MergeListTest::MergeListTest()
{
}

MergeListTest::~MergeListTest()
{
}

bool MergeListTest::DoTest()
{
    bool ret = true;
    FILE * F;
    char const * list[2] = { summary1, summary2 };
    int nb_files = 0;
    unsigned int multiplier = 257;
    CaptureSummary cs;

    /* create the capture list */
    F = ithi_file_open(capture_list_name, "w");
    ret &= (F != NULL);

    if (ret)
    {
        for (size_t j = 0; j < multiplier; j++)
        {
            for (size_t i = 0; i < 2; i++)
            {
                ret = fputs(list[i], F) != EOF;
                if (ret)
                {
                    ret = fputs("\n", F) != EOF;
                }
                nb_files++;
                if (!ret)
                {
                    TEST_LOG("Cannot write file name %d on capture file\n", nb_files);
                }
            }
        }
        fclose(F);
    }
    else {
        TEST_LOG("Cannot create capture file \n");
    }

    if (ret)
    {
        ret = cs.Merge(capture_list_name);
        if (!ret) {
            TEST_LOG("Merge of %d capture files failed.\n", nb_files);
        }
    }

    if (ret)
    {
        CaptureSummary tcs;

        ret = tcs.Load(target);

        if (ret)
        {
            tcs.MultiplyByConstantForTest(multiplier);
            tcs.Sort();

            ret = ithi_test_class::CompareCS(&cs, &tcs);

            if (!ret)
            {
                TEST_LOG("Merge of %d capture files does not match prediction.\n", nb_files);
            }
        }
    }

    return ret;
}

char const* capture_empty_list_name = "capture_empty_list.txt";

MergeEmptyListTest::MergeEmptyListTest()
{
}

MergeEmptyListTest::~MergeEmptyListTest()
{
}

bool MergeEmptyListTest::DoTest()
{
    bool ret = true;
    FILE* F;
    int nb_files = 0;
    CaptureSummary cs;

    /* create the capture list */
    F = ithi_file_open(capture_empty_list_name, "w");
    ret &= (F != NULL);

    if (ret)
    {
        fclose(F);
    }
    else {
        TEST_LOG("Cannot create empty capture file \n");
    }

    if (ret)
    {
        ret = cs.Merge(capture_empty_list_name);
        if (!ret) {
            TEST_LOG("Merge of %d capture files failed.\n", nb_files);
        }
    }

    return ret;
}
