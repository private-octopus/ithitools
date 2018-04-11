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
#include "M2Data.h"
#include "M2DataTest.h"

#ifdef _WINDOWS
#ifndef _WINDOWS64
static char const * registrar_test = "..\\data\\2017-01-31_registrars.csv";
static char const * tlds_test = "..\\data\\2017-01-31_tlds.csv";
static char const * tlds_old_test = "..\\data\\2017-feb-tlds.csv";
#else
static char const * registrar_test = "..\\..\\data\\2017-01-31_registrars.csv";
static char const * tlds_test = "..\\..\\data\\2017-01-31_tlds.csv";
static char const * tlds_old_test = "..\\..\\data\\2017-feb-tlds.csv";
#endif
#else
static char const * registrar_test = "data/2017-01-31_registrars.csv";
static char const * tlds_test = "data/2017-01-31_tlds.csv";
static char const * tlds_old_test = "data/2017-feb-tlds.csv";
#endif

struct st_m2_data_test_case_result_t {
    char const * name;
    int Domains;
    int abuse_count[4];
};

static const struct st_m2_data_test_case_result_t tld_cases[] = {
    { ".FGH", 6789123,{ 234, 345, 21111, 123 } },
    { ".IJK", 432109,{ 34, 45, 1234, 23 } },
    { ".TEST", 1234567,{ 45, 34, 456, 12 } },
    { ".XN--A01B02", 234567,{ 4, 0, 345, 0 } },
    { ".WXYZT", 12345,{ 0, 1234, 4321, 1020 } }
};

static const struct st_m2_data_test_case_result_t tld_old_cases[] = {
    { ".ABCD", 6789123, { 234, 345, 21111, 123}},
    { ".EFGHIJ", 432109, { 34, 45, 1234, 23}},
    { ".EFGHIJK", 1234567, {45, 34, 456, 12}},
    { ".LMNO", 234567, { 4, 0, 345, 0}},
    { ".PQRSTU", 123456,{ 0, 12345, 43210, 10203 } }
};

static const struct st_m2_data_test_case_result_t registrar_cases[] = {
    { "Abcdef.xyzt, LLC", 6789123,{ 234, 345, 21111, 123 } },
    { "1&amp;1 BlaBlaBla XY", 432109,{ 34, 45, 1234, 23 } },
    { "Ghijkl Technologies, Inc. Foo - Subsidiary of Ghijkl, Inc. USA", 1234567,{ 45, 34, 456, 12 } },
    { "Absurdistan Economic &amp; Industry Co., Ltd", 234567,{ 4, 0, 345, 0 } }
};

struct st_m2_data_test_case_t {
    char const * file_name;
    M2DataType test_type;
    int year;
    int month;
    int day;
    size_t nb_lines;
    const struct st_m2_data_test_case_result_t * result;
};

static const struct st_m2_data_test_case_t test_cases[] = {
    { registrar_test, Registrar, 2017, 1, 31,
    sizeof(registrar_cases) / sizeof(struct st_m2_data_test_case_result_t), registrar_cases},
    { tlds_test, TLD, 2017, 1, 31,
    sizeof(tld_cases) / sizeof(struct st_m2_data_test_case_result_t), tld_cases },
    {tlds_old_test, TLD_old, 2017, 2, 28,
    sizeof(tld_old_cases) / sizeof(struct st_m2_data_test_case_result_t), tld_old_cases }
};

static const size_t nb_test_cases = sizeof(test_cases) / sizeof(struct st_m2_data_test_case_t);

M2DataTest::M2DataTest()
{
}


M2DataTest::~M2DataTest()
{
}

bool M2DataTest::DoTest()
{
    bool ret = true;

    for (size_t i = 0; ret && i < nb_test_cases; i++)
    {
        M2Data x;
        char const * file_name = NULL;

        if ((file_name = x.get_file_name(test_cases[i].file_name)) == NULL)
        {
            ret = false;
        }
        else if (!x.parse_file_name(file_name))
        {
            ret = false;
        }
        else if (x.M2Type != test_cases[i].test_type)
        {
            ret = false;
        }
        else if (x.year != test_cases[i].year)
        {
            ret = false;
        }
        else if (x.month != test_cases[i].month)
        {
            ret = false;
        }
        else if (x.day != test_cases[i].day)
        {
            ret = false;
        }
        else if (!x.Load(test_cases[i].file_name))
        {
            ret = false;
        }
        else if (x.dataset.size() != test_cases[i].nb_lines)
        {
            ret = false;
        }
        else
        {
            for (size_t j = 0; ret && j < x.dataset.size(); j++)
            {
                if (strcmp(x.dataset[j].name, test_cases[i].result[j].name) != 0)
                {
                    ret = false;
                }
                else
                {
                    for (int k = 0; k < 4; k++)
                    {
                        if (x.dataset[j].abuse_count[k] != test_cases[i].result[j].abuse_count[k])
                        {
                            ret = false;
                            break;
                        }
                    }
                }
            }
        }
    }

    return ret;
}
