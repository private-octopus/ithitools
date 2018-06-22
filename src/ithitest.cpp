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
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include "pcap_reader.h"
#include "DnsStats.h"
#ifndef HAVE_GETOPT
#include "getopt.h"
#endif
#include "CaptureSummary.h"
#include "ithimetrics.h"

#include "ithi_test_class.h"

bool do_one_test(int i, FILE* f_log)
{
    bool ret = true;
    ithi_test_class * test = ithi_test_class::TestByNumber(i);

    if (test == NULL)
    {
        fprintf(f_log, "Cannot instantiate test %d (%s)\n",
            i, ithi_test_class::GetTestName(i));
        ret = false;
    }
    else
    {
        fprintf(f_log, "Starting test %d (%s)\n",
            i, ithi_test_class::GetTestName(i));

        if (test->DoTest())
        {
            fprintf(f_log, "    Pass.\n");
        }
        else
        {
            fprintf(f_log, "    Test %d (%s) FAILS.\n",
                i, ithi_test_class::GetTestName(i));
            ret = false;
        }

        delete test;
    }

    return ret;
}

void Usage(int argc, char ** argv, FILE* f_log)
{
    fprintf(stderr, "Usage: %s [test_name]\n", argv[0]);
    fprintf(stderr, "   Possible test names:\n");
    for (int j = 0; j < ithi_test_class::get_number_of_tests();)
    {
        fprintf(stderr, "       ");
        for (int k = 0; k < 6 && j < ithi_test_class::get_number_of_tests(); k++, j++)
        {
            fprintf(stderr, "%s", ithi_test_class::GetTestName(j));
            if (j < ithi_test_class::get_number_of_tests())
            {
                fprintf(stderr, ", ");
            }
        }
        fprintf(stderr, "\n");
    }
}

int main(int argc, char ** argv)
{
    bool ret = true;
    int nb_success = 0;
    int nb_fails = 0;

    SET_LOG_FILE(stderr);

    if (argc < 2)
    {
        for (int i = 0; i < ithi_test_class::get_number_of_tests(); i++)
        {
            if (do_one_test(i, stdout))
            {
                nb_success++;
            }
            else
            {
                nb_fails++;
                ret = false;
            }
        }
    }
    else
    {
        for (int i = 1; i < argc; i++)
        {
            int test_num = ithi_test_class::GetTestNumberByName(argv[i]);

            if (test_num < ithi_test_class::get_number_of_tests())
            {
                if (do_one_test(test_num, stdout))
                {
                    nb_success++;
                }
                else
                {
                    nb_fails++;
                    ret = false;
                }
            }
            else
            {
                fprintf(stderr, "Unknow test name: %s\n", argv[i]);
                Usage(argc, argv, stderr);
                break;
            }
        }
    }

    if ((nb_success + nb_fails) > 1)
    {
        if (nb_fails == 0)
        {
            fprintf(stdout, "All %d tests passed.\n", nb_success);
        }
        else
        {
            fprintf(stdout, "Out of %d tests, %d FAILED.\n",
                nb_success + nb_fails, nb_fails);
        }
    }

    return (ret) ? 0 : -1;
}
