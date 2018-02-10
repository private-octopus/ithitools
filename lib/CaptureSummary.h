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

#ifndef CAPTURE_SUMMARY_H
#define CAPTURE_SUMMARY_H

#include <stdint.h>
#include <stdlib.h>
#include <vector>

typedef struct _capture_line
{
    char registry_name[64];
    int key_type;
    union
    {
        int key_number;
        char key_value[64];
    };
    int count;
} CaptureLine;

class CaptureSummary
{
public:
    CaptureSummary();
    ~CaptureSummary();

    bool Load(char const * file_name);

    bool Save(char const * file_name);

    bool Compare(CaptureSummary * x);

    bool AddLine(CaptureLine * line, bool need_allocation);

    void Reserve(size_t target_size);

    void Sort();

    size_t Size();

    uint32_t GetCountByNumber(char const * table_name, uint32_t number);

    void Extract(char const * table_name, std::vector<CaptureLine *> *extract);

    bool Merge(char const * list_file_name);
    bool Merge(size_t nb_files, char const ** file_name);
    bool Merge(size_t nb_summaries, CaptureSummary ** cs);
    static int compare_string(char const * x, char const * y);

    void MultiplyByConstantForTest(unsigned int multiplier);

private:
    std::vector<CaptureLine *> summary;

    static bool CaptureLineIsLower(CaptureLine * x, CaptureLine * y);
    static bool CaptureLineIsSameKey(CaptureLine * x, CaptureLine * y);
};

#endif

