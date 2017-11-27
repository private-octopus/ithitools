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

            ret = cs.Compare(&tcs);
        }
    }

    return ret;
}
