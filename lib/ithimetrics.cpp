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

#ifdef _WINDOWS
#include "wincompat/dirent.h"
#else
#include <dirent.h>
#endif
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>

#include "DnsStats.h"
#include "CaptureSummary.h"
#include "M7Getter.h"
#include "M2Data.h"
#include "ithimetrics.h"

ithimetrics::ithimetrics()
    :
    metric_date(NULL),
    ithi_folder(NULL),
    compliance_file_name(NULL),
    root_capture_file_name(NULL),
    recursive_capture_file_name(NULL),
    authoritative_capture_file_name(NULL),
    abuse_file_name_tlds(NULL),
    abuse_file_name_registrars(NULL),
    root_zone_file_name(NULL)
{
    for (int i = 0; i < ITHI_NUMBER_OF_METRICS; i++) {
        metric_file[i] = NULL;
        metric_is_available[i] = false;
    }
}

ithimetrics::~ithimetrics() {
    if (metric_date != NULL) {
        free(metric_date);
        metric_date = NULL;
    }

    if (ithi_folder != NULL) {
        free(ithi_folder);
        ithi_folder = NULL;
    }

    if (abuse_file_name_tlds == NULL) {
        free(abuse_file_name_tlds);
        abuse_file_name_tlds = NULL;
    }

    if (abuse_file_name_registrars == NULL) {
        free(abuse_file_name_registrars);
        abuse_file_name_registrars = NULL;
    }

    for (int i = 0; i < ITHI_NUMBER_OF_METRICS; i++) {
        if (metric_file[i] != NULL)
        {
            free(metric_file[i]);
            metric_file[i] = NULL;
        }
    }

    if (root_capture_file_name != NULL) {
        free(root_capture_file_name);
        root_capture_file_name = NULL;
    }

    if (recursive_capture_file_name != NULL) {
        free(recursive_capture_file_name);
        recursive_capture_file_name = NULL;
    }

    if (authoritative_capture_file_name != NULL) {
        free(authoritative_capture_file_name);
        authoritative_capture_file_name = NULL;
    }

    if (root_zone_file_name != NULL) {
        free(root_zone_file_name);
        root_zone_file_name = NULL;
    }

    if (compliance_file_name != NULL) {
        free(compliance_file_name);
        compliance_file_name = NULL;
    }
}

bool ithimetrics::SetIthiFolder(char const * folder)
{
    return copy_name(&ithi_folder, folder);
}

bool ithimetrics::SetDateString(char const * date_string)
{
    return copy_name(&metric_date, date_string);
}

bool ithimetrics::SetMetricFileNames(int metric_number, char const * metric_file_name)
{
    bool ret = false;

    if (metric_number >= 0 && metric_number < ITHI_NUMBER_OF_METRICS)
    {
        ret = copy_name(&metric_file[metric_number], metric_file_name);
    }

    return ret;
}

bool ithimetrics::SetDefaultDate(time_t current_time)
{
    bool ret = true;
    struct tm tm;
    char buffer[256];
    int last_day_of_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

#ifdef _WINDOWS
    if (localtime_s(&tm, &current_time) != 0)
    {
        ret = false;
    }
#else
    tm = *localtime(&current_time);
#endif

    int year4digit = tm.tm_year + 1900;
    if (year4digit % 4 == 0 && (year4digit % 100 != 0 || year4digit % 400 == 0))
    {
        last_day_of_month[1] = 29;
    }

    if (ret)
    {
        ret = snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, last_day_of_month[tm.tm_mon]) > 0;
    }

    if (ret)
    {
        ret = SetDateString(buffer);
    }

    return ret;
}

bool ithimetrics::SetDefaultAbuseFileName(M2DataType f_type)
{ 
    bool ret = (f_type == TLD || f_type == Registrar);
    bool is_set = (GetAbuseFileName(f_type) != NULL);
    char file_name_buffer[512];

    if (ret && ithi_folder == NULL)
    {
        ret = SetIthiFolder(ITHI_DEFAULT_FOLDER);
    }

    if (ret && metric_date == NULL)
    {
        ret = SetDefaultDate(time(0));
    }

    if (ret && !is_set)
    {
        ret = snprintf(file_name_buffer, sizeof(file_name_buffer),
            "%s%sinput%sM2%s%s%s", 
            ithi_folder, ITHI_FILE_PATH_SEP, ITHI_FILE_PATH_SEP, ITHI_FILE_PATH_SEP, 
            metric_date, M2Data::get_file_suffix(f_type)) > 0;

        if (ret)
        {
            ret = SetAbuseFileName(file_name_buffer, f_type);
        }
    }

    return ret;
}

bool ithimetrics::SetAbuseFileName(char const * abuse_file_name, M2DataType f_type)
{
    bool ret = false;

    switch (f_type) {
    case TLD:
        ret = copy_name(&this->abuse_file_name_tlds, abuse_file_name);
        break;
    case Registrar:
        ret = copy_name(&this->abuse_file_name_registrars, abuse_file_name);
        break;
    default:
        break;
    }

    return ret;
}

const char * ithimetrics::GetAbuseFileName(M2DataType f_type)
{
    const char * name = NULL;

    switch (f_type) {
    case TLD:
        name = abuse_file_name_tlds;
        break;
    case Registrar:
        name = abuse_file_name_registrars;
        break;
    default:
        break;
    }

    return name;
}

bool ithimetrics::SetComplianceFileName(char const * compliance_file_name)
{
    return copy_name(&this->compliance_file_name, compliance_file_name);
}

bool ithimetrics::SetDefaultComplianceFileName()
{
    return SetDefaultCaptureFiles("M1", "-compliance.csv", &compliance_file_name);
}

bool ithimetrics::SetRootCaptureFileName(char const * file_name)
{
    return copy_name(&root_capture_file_name, file_name);
}

bool ithimetrics::SetRecursiveCaptureFileName(char const * file_name)
{
    return copy_name(&recursive_capture_file_name, file_name);
}

bool ithimetrics::SetAuthoritativeCaptureFileName(char const * file_name)
{
    return copy_name(&authoritative_capture_file_name, file_name);
}


bool ithimetrics::SetRootZoneFileName(char const * file_name)
{
    return copy_name(&root_zone_file_name, file_name);
}

bool ithimetrics::SetDefaultRootCaptureFile()
{
    return SetDefaultCaptureFiles("M3", "-summary.csv", &root_capture_file_name);
}

bool ithimetrics::SetDefaultRecursiveCaptureFile()
{
    return SetDefaultCaptureFiles("M46", "-summary.csv", &recursive_capture_file_name);
}

bool ithimetrics::SetDefaultAuthoritativeCaptureFile()
{
    return SetDefaultCaptureFiles("M8", "-summary.csv", &authoritative_capture_file_name);
}

bool ithimetrics::SetDefaultRootZoneFile()
{
    return SetDefaultCaptureFiles("M7", ".zone", &root_zone_file_name);
}

bool ithimetrics::SetDefaultCaptureFiles(char const * metric_name, char const * suffix, char ** p_file_name)
{
    bool ret = true;
    char file_name[512];

    if (*p_file_name != NULL)
    {
        ret = false;
    }

    if (ret && ithi_folder == NULL)
    {
        ret = SetIthiFolder(ITHI_DEFAULT_FOLDER);
    }

    if (ret && metric_date == NULL)
    {
        ret = SetDefaultDate(time(0));
    }

    if (ret)
    {
        ret = snprintf(file_name, sizeof(file_name), "%s%sinput%s%s%s%s-%s%s", ithi_folder, ITHI_FILE_PATH_SEP, 
            ITHI_FILE_PATH_SEP, metric_name, ITHI_FILE_PATH_SEP, metric_name, metric_date, suffix) > 0;

        if (ret)
        {
            ret = copy_name(p_file_name, file_name);
        }
    }

    return ret;
}


bool ithimetrics::copy_name(char ** target, char const * name)
{
    bool ret = true;
    size_t len = strlen(name);

    if (*target != NULL)
    {
        free(*target);
        *target = NULL;
    }

    *target = (char *)malloc(len + 1);

    if (*target == NULL)
    {
        ret = false;
    }
    else
    {
        memcpy(*target, name, len);
        (*target)[len] = 0;
    }

    return ret;
}

bool ithimetrics::GetMetrics() {
    bool ret = false;


    if (compliance_file_name == NULL)
    {
        (void)SetDefaultComplianceFileName();
    }

    if (compliance_file_name != NULL && cm1.Load(compliance_file_name))
    {
        metric_is_available[0] = cm1.Compute();
        ret |= metric_is_available[0];
    }

    if (abuse_file_name_tlds == NULL)
    {
        (void)SetDefaultAbuseFileName(TLD);
    }

    if (abuse_file_name_registrars == NULL)
    {
        (void)SetDefaultAbuseFileName(Registrar);
    }

    if (abuse_file_name_tlds != NULL && abuse_file_name_registrars != NULL && cm2.LoadTwoFiles(abuse_file_name_tlds, abuse_file_name_registrars))
    {
        metric_is_available[1] = cm2.Compute();
        ret |= metric_is_available[1];
    }

    if (root_capture_file_name == NULL)
    {
        (void)SetDefaultRootCaptureFile();
    }

    if (root_capture_file_name != NULL && cm3.Load(root_capture_file_name))
    {
        metric_is_available[2] = cm3.Compute();
        ret |= metric_is_available[2];
    }

    if (recursive_capture_file_name == NULL)
    {
        (void)SetDefaultRecursiveCaptureFile();
    }

    if (recursive_capture_file_name != NULL && cm4.Load(recursive_capture_file_name))
    {
        metric_is_available[3] = cm4.Compute();
        ret |= metric_is_available[3];
    }


    if (recursive_capture_file_name != NULL && cm6.Load(recursive_capture_file_name))
    {
        metric_is_available[5] = cm6.Compute();
        ret |= metric_is_available[5];
    }

    if (root_zone_file_name == NULL)
    {
        (void)SetDefaultRootZoneFile();
    }

    if (root_zone_file_name != NULL && cm7.Load(root_zone_file_name))
    {
        metric_is_available[6] = cm7.Compute();
        ret |= metric_is_available[6];
    }


    if (authoritative_capture_file_name == NULL)
    {
        (void)SetDefaultAuthoritativeCaptureFile();
    }

    if (authoritative_capture_file_name != NULL && cm8.Load(authoritative_capture_file_name))
    {
        metric_is_available[7] = cm8.Compute();
        ret |= metric_is_available[7];
    }

    return ret;
}

bool ithimetrics::SaveMetricFiles()
{
    bool ret = true;
    char buffer[512];
    ComputeMetric * cm[ITHI_NUMBER_OF_METRICS] = { &cm1, &cm2, &cm3, &cm4, NULL, &cm6, &cm7, &cm8 };

    if (ret && ithi_folder == NULL)
    {
        ret = SetIthiFolder(ITHI_DEFAULT_FOLDER);
    }

    /* Set the date to default if not already done */
    if (ret && metric_date == NULL)
    {
        ret = SetDefaultDate(time(0));
    }

    for (int i = 0; ret && i < ITHI_NUMBER_OF_METRICS; i++)
    {
        if (!metric_is_available[i] || cm[i] == NULL)
        {
            continue;
        }

        if (metric_file[i] == NULL)
        {
            /* Set the metric file name to its default value*/
            if (ret)
            {
#ifdef _WINDOWS
                ret = snprintf(buffer, sizeof(buffer), "%s\\M%d\\M%d-%s.csv", ithi_folder, i + 1, i + 1, metric_date) > 0;
#else
                ret = snprintf(buffer, sizeof(buffer), "%s/M%d/M%d-%s.csv", ithi_folder, i + 1, i + 1, metric_date) > 0;
#endif
                ret &= SetMetricFileNames(i, buffer);
            }
        }

        if (ret)
        {
            ret = cm[i]->Save(metric_file[i]);
        }

    }

    return ret;

}

bool ithimetrics::Save(char const * file_name)
{

    ComputeMetric * cm[ITHI_NUMBER_OF_METRICS] = { &cm1, &cm2, &cm3, &cm4, NULL, &cm6, &cm7, &cm8 };
    FILE* F;
#ifdef _WINDOWS
    errno_t err = fopen_s(&F, file_name, "w");
    bool ret = (err == 0);
#else
    bool ret;
    F = fopen(file_name, "w");
    ret = (F != NULL);
#endif

    for (int i = 0; ret && i < ITHI_NUMBER_OF_METRICS; i++)
    {
        if (!metric_is_available[i] || cm[i] == NULL)
        {
            continue;
        }

        ret = cm[i]->Write(F);
    }

    if (F != NULL)
    {
        fclose(F);
    }

    return ret;
}

