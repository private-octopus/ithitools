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

#ifndef METRIC_TEST_H
#define METRIC_TEST_H

class MetricTest
{
public:
    MetricTest();
    ~MetricTest();

    bool DoTest();

    static bool compare_files(char const * fname1, char const * fname2);
    static bool compare_lines(char const * b1, char const * b2);
};

class MetricDateTest
{
public:
    MetricDateTest();
    ~MetricDateTest();

    bool DoTest();
};

class MetricCaptureFileTest
{
public:
    MetricCaptureFileTest();
    ~MetricCaptureFileTest();

    bool DoTest();

private:
    bool CreateDirectoryIfAbsent(char const * dir_name);
    bool CopyFileToDestination(char const * target_name, char const * source_name);
};

#endif /* METRIC_TEST_H */