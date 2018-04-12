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

#include "M1Data.h"
#include "M1DataTest.h"

#ifdef _WINDOWS
#ifndef _WINDOWS64
static char const * compliance_test = "..\\data\\M1-2017-01-31-compliance.csv";
#else
static char const * compliance_test = "..\\..\\data\\M1-2017-01-31-compliance.csv";
#endif
#else
static char const * compliance_test = "data/M1-2017-01-31-compliance.csv";
#endif

static const size_t compliance_test_lines = 34;
static const size_t compliance_test_notices = 19;
static const uint64_t compliance_test_total_domains = 9579309;
static const uint64_t compliance_test_total_1st = 809;
static const int compliance_test_nbRegistrars50pc = 1;
static const int compliance_test_nbRegistrars90pc = 5;

M1DataTest::M1DataTest()
{
}

M1DataTest::~M1DataTest()
{
}

bool M1DataTest::DoTest()
{
    bool ret = true;
    M1Data x;

    if (!x.Load(compliance_test))
    {
        TEST_LOG("M1Data test, could not open file %s\n", compliance_test);
        ret = false;
    }
    else if (x.dataset.size() != compliance_test_lines)
    {
        TEST_LOG("M1Data test, got %d lines in files instead of %d\n", 
            (int)x.dataset.size(), (int)compliance_test_lines);
        ret = false;
    }
    else if (x.firstNotice.size() != compliance_test_notices)
    {
        TEST_LOG("M1Data test, got %d unique registrars instead of %d\n",
            (int)x.firstNotice.size(), (int)compliance_test_notices);
        ret = false;
    }
    else if (x.totalDomain != compliance_test_total_domains)
    {
        TEST_LOG("M1Data test, got %d domains instead of %d\n",
            (int)x.totalDomain, (int)compliance_test_total_domains);
        ret = false;
    }
    else if (x.total1stN != compliance_test_total_1st)
    {
        TEST_LOG("M1Data test, got %d 1st notices instead of %d\n",
            (int)x.total1stN, (int)compliance_test_total_1st);
        ret = false;
    }
    else if (x.nbRegistrars50pc != compliance_test_nbRegistrars50pc)
    {
        TEST_LOG("M1Data test, 50% mark at %d instead of %d\n",
            x.nbRegistrars50pc, compliance_test_nbRegistrars50pc);
        ret = false;
    }
    else if (x.nbRegistrars90pc != compliance_test_nbRegistrars90pc)
    {
        TEST_LOG("M1Data test, 90% mark at %d instead of %d\n",
            (int)x.nbRegistrars90pc, (int)compliance_test_nbRegistrars90pc);
        ret = false;
    }

    return ret;
}
