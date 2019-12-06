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

#include "CaptureSummary.h"
#include "SaveTest.h"

#ifdef _WINDOWS
#ifndef _WINDOWS64
static char const * good_file = "..\\data\\summary1.csv";
static char const * saved_file = "save_test.csv";
#else
static char const * good_file = "..\\..\\data\\summary1.csv";
static char const * saved_file = "save_test.csv";
#endif
#else
static char const * good_file = "data/summary1.csv";
static char const * saved_file = "save_test.csv";
#endif

SaveTest::SaveTest()
{
}


SaveTest::~SaveTest()
{
}

bool SaveTest::DoTest()
{
    CaptureSummary cs;

    bool ret = cs.Load(good_file);

    if (ret)
    {
        ret = cs.Save(saved_file);

        if (ret)
        {
            CaptureSummary cs2;

            ret = cs2.Load(saved_file);

            if (ret)
            {
                ret = ithi_test_class::CompareCS(&cs, &cs2);
            }
        }
    }

    return ret;
}
