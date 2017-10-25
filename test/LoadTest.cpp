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

#include <stdio.h>
#include "CaptureSummary.h"
#include "LoadTest.h"

LoadTest::LoadTest()
{
}

LoadTest::~LoadTest()
{
}

#ifdef _WINDOWS
static char const * no_such_file = "no_such_file.csv";
static char const * good_file = "..\\data\\summary1.csv";
#else
static char const * no_such_file = "no_such_file.csv";
static char const * good_file = "./data/summary1.csv";
#endif


bool LoadTest::DoTest()
{
    bool ret = DoNoSuchTest();

    if (ret)
    {
        ret = DoGoodTest();
    }

    return ret;
}

bool LoadTest::DoGoodTest()
{
    CaptureSummary cs;

    bool ret = cs.Load(good_file);

    if (ret)
    {
        /* Count the lines in the test file and verify that the count matches. */
        FILE* F = NULL;
        char buffer[512];
        size_t nb_lines = 0;

#ifdef _WINDOWS
        errno_t err = fopen_s(&F, good_file, "r");
        bool ret = (err == 0 && F != NULL);
#else
        bool ret;
        F = fopen(good_file, "r");
        ret = (F != NULL);
#endif

        while (ret && fgets(buffer, sizeof(buffer), F))
        {
            nb_lines++;
        }

        if (F != NULL)
        {
            fclose(F);
        }

        if (ret)
        {
            ret = nb_lines = cs.Size();
        }
    }

    return ret;
}

bool LoadTest::DoNoSuchTest()
{
    CaptureSummary cs;

    bool ret = !cs.Load(no_such_file);

    return ret;
}
