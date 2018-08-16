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
#ifndef ODI_PUBLISHER_H
#define ODI_PUBLISHER_H

#include <time.h>

#ifndef ITHI_DEFAULT_DATA_FOLDER
#ifdef _WINDOWS
#ifdef _WINDOWS64
#define ITHI_DEFAULT_DATA_FOLDER "..\\..\\data"
#else
#define ITHI_DEFAULT_DATA_FOLDER "..\\data"
#endif
#else
#define ITHI_DEFAULT_DATA_FOLDER "./data"
#endif
#endif

class OdiPublisher
{
public:
    OdiPublisher();
    ~OdiPublisher();

    static bool PublishMetricFile(const char * metric_file_name, char const * odi_dir, char const * data_dir, time_t current_time);
    static bool GetUpdateTime(char * time_value, size_t time_value_size, time_t current_time);
    static bool CopyFile(const char * source_file_name, const char * dest_file_name);
    static bool CopyUpdateJsonFile(int metric_id, const char * dest_file_name, char const * data_dir, time_t current_time);

};

#endif /* ODI_PUBLISHER_H */