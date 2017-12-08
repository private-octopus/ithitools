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

#include <string.h>
#include "CsvHelper.h"
#include "CsvTest.h"

struct st_CsvTest_string_t
{
    char const * csv_text;
    char const * text;
    int next_start;
};

static st_CsvTest_string_t const string_test[] = {
    { "abc,", "abc", 4 },
    { "   abc,", "abc", 7 },
    { "\"abc\",", "abc", 6 },
    { "\"ab,c\",", "ab,c", 7 }
};

static size_t nb_string_test = sizeof(string_test) / sizeof(st_CsvTest_string_t);

struct st_CsvTest_int_t
{
    char const * csv_text;
    int const val;
    int next_start;
};

static st_CsvTest_int_t const int_test[] = {
    { "1,", 1, 2 },
    { "  17,", 17, 5 },
    { "  17 ,", 17, 6 },
    { "\"1000\",", 1000, 7 },
    { "\"1,000\",", 1000, 8 }
};

static size_t nb_int_test = sizeof(int_test) / sizeof(st_CsvTest_int_t);

struct st_CsvTest_double_t
{
    char const * csv_text;
    double const val;
    int next_start;
};

static st_CsvTest_double_t const double_test[] = {
    { "1.00,", 1.0, 5 },
    { "  16.125,", 16.125, 9 },
    { "  17 ,", 17, 6 },
    { "\"1000\",", 1000, 7 },
    { "\"1000.00\",", 1000, 10 }
};

static size_t nb_double_test = sizeof(double_test) / sizeof(st_CsvTest_double_t);

CsvTest::CsvTest()
{
}


CsvTest::~CsvTest()
{
}

bool CsvTest::DoTest()
{
    bool ret = true;
    char text[256];

    for (size_t i = 0; ret && i < nb_string_test; i++)
    {
        int start = CsvHelper::read_string(text, sizeof(text), 0, string_test[i].csv_text, strlen(string_test[i].csv_text));

        if (start != string_test[i].next_start)
        {
            ret = false;
        }
        else if (strcmp(text, string_test[i].text) != 0)
        {
            ret = false;
        }
    }

    for (size_t i = 0; ret && i < nb_int_test; i++)
    {
        int x = -1;
        int start = CsvHelper::read_number(&x, 0, int_test[i].csv_text, strlen(int_test[i].csv_text));

        if (start != int_test[i].next_start)
        {
            ret = false;
        }
        else if (x != int_test[i].val)
        {
            ret = false;
        }
    }

    for (size_t i = 0; ret && i < nb_double_test; i++)
    {
        double x = -1;
        int start = CsvHelper::read_double(&x, 0, double_test[i].csv_text, strlen(double_test[i].csv_text));

        if (start != double_test[i].next_start)
        {
            ret = false;
        }
        else if (x != double_test[i].val)
        {
            ret = false;
        }
    }

    return ret;
}
