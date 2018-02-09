/*
* Author: Christian Huitema
* Copyright (c) 2018, Private Octopus, Inc.
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
#ifndef COMPUTE_M346_H
#define COMPUTE_M346_H

#include <time.h>
#include "CaptureSummary.h"
#include "ComputeMetric.h"

typedef struct _st_metric34_line_t {
    char domain[64];
    double frequency;
} metric34_line_t;

class ComputeM3 : public ComputeMetric
{
public:
    ComputeM3();
    ~ComputeM3();

    bool Load(char const * single_file_name) override;
    bool LoadMultipleFiles(char const ** in_files, int nb_files) override;
    bool Compute() override;
    bool Write(FILE * F_out) override;

private:
    uint32_t nb_rootqueries;
    double m3_1;
    double m3_2;
    std::vector<metric34_line_t> m33_1;
    std::vector<metric34_line_t> m33_2;
    std::vector<metric34_line_t> m33_3;
    double m33_4;
    CaptureSummary cs;

    void GetM3_X(uint32_t table_id,
        std::vector<metric34_line_t>* mstring_x, double min_share);
    bool GetM3_1();
    bool GetM3_2();
    bool GetM33_1();
    bool GetM33_2();
    bool GetM33_3();
};


class ComputeM4 : public ComputeMetric
{
public:
    ComputeM4();
    ~ComputeM4();

    bool Load(char const * single_file_name) override;
    bool LoadMultipleFiles(char const ** in_files, int nb_files) override;
    bool Compute() override;
    bool Write(FILE * F_out) override;

private:
    uint32_t nb_userqueries;
    uint32_t nb_nondelegated;
    uint32_t nb_delegated;
    CaptureSummary cs;

    double m4_1;
    std::vector<metric34_line_t> m4_2;
    std::vector<metric34_line_t> m4_3;
    double m4_4;

    bool GetM4_X(uint32_t table_id, std::vector<metric34_line_t>* mstring_x, double min_share);
    bool GetM4_1();
    bool GetM4_2();
    bool GetM4_3();
};

#endif /* COMPUTE_M346_H */