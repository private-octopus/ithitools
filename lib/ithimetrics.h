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

#ifndef ITHI_METRICS_H
#define ITHI_METRICS_H

#include <time.h>
#include "CaptureSummary.h"
#include "M1Data.h"
#include "M2Data.h"
#include "ComputeM34.h"
#include "ComputeM6.h"
#include "M7Getter.h"


#ifdef _WINDOWS
#ifndef ITHI_DEFAULT_FOLDER
#define ITHI_DEFAULT_FOLDER ".\\"
#endif
#ifndef ITHI_FILE_PATH_SEP
#define ITHI_FILE_PATH_SEP "\\"
#endif
#else
#ifndef ITHI_DEFAULT_FOLDER
#define ITHI_DEFAULT_FOLDER "./"
#endif
#ifndef ITHI_FILE_PATH_SEP
#define ITHI_FILE_PATH_SEP "/"
#endif
#endif

#define ITHI_NUMBER_OF_METRICS 8

class ithimetrics
{
public:
    ithimetrics();
    ~ithimetrics();

    bool GetMetrics();

    bool SetIthiFolder(char const * folder);
    const char * GetIthiFolderName() {
        return ithi_folder;
    };

    bool SetMetricFileNames(int metric_number, char const * metric_file_name);

    bool SetRootCaptureFileName(char const * file_name);
    bool SetRecursiveCaptureFileName(char const * file_name);
    bool SetAuthoritativeCaptureFileName(char const * file_name);

    bool SetRootZoneFileName(char const * file_name);

    bool SetDefaultRootCaptureFile();
    bool SetDefaultRecursiveCaptureFile();
    bool SetDefaultAuthoritativeCaptureFile();
    bool SetDefaultRootZoneFile();

    const char * GetRootCaptureFileName() {
        return root_capture_file_name;
    };
    const char * GetRecursiveCaptureFileName() {
        return recursive_capture_file_name;
    };
    const char * GetAuthoritativeCaptureFileName() {
        return recursive_capture_file_name;
    };
    const char * GetRootZoneFileName() {
        return root_zone_file_name;
    };

    bool SetDateString(char const * date_string);
    bool SetDefaultDate(time_t current_time);
    const char * GetMetricDate() { return (const char *)metric_date; };

    bool SetAbuseFileName(char const * abuse_file_name, M2DataType f_type);
    bool SetDefaultAbuseFileName(M2DataType f_type);
    const char * GetAbuseFileName(M2DataType f_type);

    bool SetComplianceFileName(char const * compliance_file_name);
    bool SetDefaultComplianceFileName();
    const char * GetComplianceFileName() { return (const char *)compliance_file_name; };

    bool SaveMetricFiles();

    bool Save(char const * file_name);

    static bool ParseMetricFileName(const char * name, int * metric_id, int * year, int * month, int * day, size_t * name_offset);

private:
    ComputeM1 cm1;
    ComputeM2 cm2;
    ComputeM3 cm3;
    ComputeM4 cm4;
    ComputeM6 cm6;
    ComputeM7 cm7;
    ComputeM8 cm8;

    char * metric_date;
    char * ithi_folder;
    char * metric_file[ITHI_NUMBER_OF_METRICS];
    bool metric_is_available[ITHI_NUMBER_OF_METRICS];
    char * compliance_file_name;
    char * root_capture_file_name;
    char * recursive_capture_file_name;
    char * authoritative_capture_file_name;
    char * abuse_file_name_tlds;
    char * abuse_file_name_registrars;
    char * root_zone_file_name;

    bool SetDefaultCaptureFiles(char const * metric_name, char const * suffix, char ** p_file_name);

    static bool copy_name(char ** target, char const * name);
};

#endif /* ITHI_METRICS_H */
