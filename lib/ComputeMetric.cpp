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

#include "ithiutil.h"
#include "ComputeMetric.h"

ComputeMetric::ComputeMetric()
    :
    F_log(NULL)
{
}

ComputeMetric::~ComputeMetric()
{
}

bool ComputeMetric::LoadMultipleFiles(char const ** in_files, int nb_files)
{
    bool ret = true;

    if (nb_files == 1)
    {
        ret = Load(in_files[0]);
    }
    else
    {
        ret = false;
    }
    return ret;
}

bool ComputeMetric::Save(char const * out_file)
{
    bool ret;
    FILE * F = NULL;
    
    F = ithi_file_open(out_file, "w");
    ret = (F != NULL);

    if (ret)
    {
        ret = Write(F);
        fclose(F);
    }

    return ret;
}
