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

#ifndef COMPUTE_METRIC_H
#define COMPUTE_METRIC_H


#if __cplusplus < 199711L
#ifndef override
#define override 
#endif
#endif

#include <stdio.h>

class ComputeMetric
{
public:
    ComputeMetric();
    virtual ~ComputeMetric();

    void SetLog(FILE * F_log) {
        this->F_log = F_log;
    };

    virtual bool Load(char const * single_file_name) = 0;
    virtual bool LoadMultipleFiles(char const ** in_files, int nb_files);
    virtual bool Compute() = 0;
    virtual bool Save(char const * out_file);
    virtual bool Write(FILE * F_out) = 0;

    FILE * F_log;
};

#endif /* COMPUTE_METRIC_H */