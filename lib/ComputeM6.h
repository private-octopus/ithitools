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

#ifndef COMPUTE_M6_H
#define COMPUTE_M6_H

#include <time.h>
#include "CaptureSummary.h"
#include "ComputeMetric.h"

typedef struct _st_metric6_registered_t {
    uint32_t key;
    char const * key_name;
} metric6_registered_t;

typedef struct _st_metric6_parameter {
    uint32_t parameter_value;
    uint32_t parameter_count;
} metric6_parameter_t;

typedef struct _st_metric6_def_t {
    char const * m6_prefix;
    uint32_t table_id;
    uint32_t nb_registered;
    metric6_registered_t * registry;
    char const * iana_file_name;
} metric6_def_t;

typedef struct _st_metric6_line_t {
    char const * m6_prefix;
    double m6_x_1;
    double m6_x_2;
    std::vector<metric6_parameter_t> m6_x_3;
} metric6_line_t;


class ComputeM6 : public ComputeMetric
{
public:
    ComputeM6();
    ~ComputeM6();

    bool Load(char const * single_file_name) override;
    bool LoadMultipleFiles(char const ** in_files, int nb_files) override;
    bool Compute() override;
    bool Write(FILE * F_out) override;

private:
    CaptureSummary cs;
    std::vector<metric6_line_t> m6;
};

#endif