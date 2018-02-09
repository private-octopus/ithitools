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
#ifndef ITHI_DEFAULT_FOLDER
#define ITHI_DEFAULT_FOLDER ".\\"
#endif
#ifndef ITHI_FILE_PATH_SEP
#define ITHI_FILE_PATH_SEP "\\"
#endif
#else
#include <dirent.h>
#ifndef ITHI_DEFAULT_FOLDER
#define ITHI_DEFAULT_FOLDER "./"
#endif
#ifndef ITHI_FILE_PATH_SEP
#define ITHI_FILE_PATH_SEP "/"
#endif
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
    nb_capture_files(0),
    capture_file(NULL),
    abuse_file_name(NULL)
{
    for (int i = 0; i < 7; i++) {
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

    if (abuse_file_name == NULL) {
        free(abuse_file_name);
        abuse_file_name = NULL;
    }

    for (int i = 0; i < 7; i++) {
        if (metric_file[i] != NULL)
        {
            free(metric_file[i]);
            metric_file[i] = NULL;
        }
    }

    if (capture_file != NULL) {
        for (uint32_t i = 0; i < nb_capture_files; i++)
        {
            if (capture_file[i] != NULL)
            {
                free(capture_file[i]);
                capture_file[i] = NULL;
            }
        }

        free(capture_file);
        capture_file = NULL;
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

    if (metric_number >= 0 && metric_number < 7)
    {
        ret = copy_name(&metric_file[metric_number], metric_file_name);
    }

    return ret;
}

bool ithimetrics::SetCaptureFileNames(int nb_files, char const ** file_names)
{
    bool ret = true;

    if (nb_files > 0)
    {
        capture_file = (char **)malloc(nb_files * sizeof(char *));
        if (capture_file == NULL)
        {
            ret = false;
        }
        else
        {
            memset(capture_file, 0, nb_files * sizeof(char *));
            this->nb_capture_files = nb_files;

            for (int i = 0; ret && i < nb_files; i++)
            {
                ret = copy_name(&capture_file[i], file_names[i]);
            }
        }
    }

    return ret;
}

bool ithimetrics::SetDefaultDate(time_t current_time)
{
    bool ret = true;
    struct tm tm;
    char buffer[256];

#ifdef _WINDOWS
    if (localtime_s(&tm, &current_time) != 0)
    {
        ret = false;
    }
#else
    tm = *localtime(&current_time);
#endif
    if (ret)
    {
        ret = snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday) > 0;
    }

    if (ret)
    {
        ret = SetDateString(buffer);
    }

    return ret;
}

bool ithimetrics::SetDefaultAbuseFileName(time_t current_time)
{
    bool ret = (abuse_file_name == NULL);
    char file_name_buffer[512];

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
        ret = snprintf(file_name_buffer, sizeof(file_name_buffer),
            "%s%sinput%sM2%s_tlds.csv", 
            ithi_folder, ITHI_FILE_PATH_SEP, ITHI_FILE_PATH_SEP, metric_date) > 0;

        if (ret)
        {
            ret = SetAbuseFileName(file_name_buffer);
        }
    }

    return ret;
}

bool ithimetrics::SetDefaultCaptureFiles()
{
    bool ret = true;
    char dir_name_346[512];
    char dir_name_loc[512];
    char expected_name[256];
    char file_name[512];
    size_t max_file_number = 128;

    if (capture_file != NULL)
    {
        ret = false;
    }
    else
    {
        capture_file = (char **)malloc(max_file_number * sizeof(char *));
        memset(capture_file, 0, max_file_number * sizeof(char *));
        nb_capture_files = 0;

        ret = capture_file != NULL;
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
        ret = snprintf(dir_name_346, sizeof(dir_name_346), "%s%sinput%sM346", ithi_folder, ITHI_FILE_PATH_SEP, ITHI_FILE_PATH_SEP);
    }

    if (ret)
    {
        DIR           *d346;
        struct dirent *dir_loc;
        d346 = opendir(dir_name_346);
        if (d346 == NULL)
        {
            ret = false;
        }
        else
        {
            while ((dir_loc = readdir(d346)) != NULL)
            {
                if (strcmp(dir_loc->d_name, ".") == 0 ||
                    strcmp(dir_loc->d_name, "..") == 0)
                {
                    continue;
                }

                ret = snprintf(dir_name_loc, sizeof(dir_name_loc), "%s%s%s%s",
                    dir_name_346, ITHI_FILE_PATH_SEP, dir_loc->d_name, ITHI_FILE_PATH_SEP) > 0;

                if (ret)
                {
                    ret = snprintf(expected_name, sizeof(expected_name), "M346-%s-%s.csv",
                        metric_date, dir_loc->d_name) > 0;
                }

                if (ret)
                {
                    DIR           *d_loc;
                    struct dirent *file_ent;
                    d_loc = opendir(dir_name_loc);
                    if (d_loc == NULL)
                    {
                        /* Probably not a directory */
                        continue;
                    }
                    else
                    {
                        while (ret && (file_ent = readdir(d_loc)) != NULL)
                        {
                            /* Check whether the name matches the expected pattern */
                            if (strcmp(expected_name, file_ent->d_name) == 0)
                            {
                                if (nb_capture_files >= max_file_number)
                                {
                                    char ** new_capture_file = NULL;
                                    max_file_number *= 2;
                                    new_capture_file = (char **)malloc(max_file_number * sizeof(char*));

                                    if (new_capture_file == NULL)
                                    {
                                        ret = false;
                                    }
                                    else
                                    {
                                        memset(new_capture_file, 0, max_file_number * sizeof(char*));
                                        memcpy(new_capture_file, capture_file, nb_capture_files * sizeof(char*));
                                        free(capture_file);
                                        capture_file = new_capture_file;
                                    }
                                }

                                if (ret)
                                {
                                    ret = snprintf(file_name, sizeof(file_name), "%s%s",
                                        dir_name_loc, expected_name) > 0;
                                    if (ret)
                                    {
                                        ret = copy_name(&capture_file[nb_capture_files], file_name);
                                        nb_capture_files++;
                                        break;
                                    }
                                }
                            }
                        }

                        closedir(d_loc);
                    }
                }
            }

            closedir(d346);
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

    if (abuse_file_name == NULL)
    {
        (void)SetDefaultAbuseFileName(time(0));
    }

    if (abuse_file_name != NULL)
    {
        if (cm2.Load(abuse_file_name))
        {
            metric_is_available[1] = cm2.Compute();
            ret |= metric_is_available[1];
        }
    }

    if (cm3.LoadMultipleFiles((char const **)capture_file, nb_capture_files))
    {
        metric_is_available[2] = cm3.Compute();
        ret |= metric_is_available[2];
    }

    if (cm4.LoadMultipleFiles((char const **)capture_file, nb_capture_files))
    {
        metric_is_available[3] = cm4.Compute();
        ret |= metric_is_available[3];
    }


    if (cm6.LoadMultipleFiles((char const **)capture_file, nb_capture_files))
    {
        metric_is_available[5] = cm6.Compute();
        ret |= metric_is_available[5];
    }

    return ret;
}

bool ithimetrics::GetM7(char const * zone_file_name)
{
    bool ret = false;

    if (cm7.Load(zone_file_name))
    {
        metric_is_available[6] = cm7.Compute();
        ret = metric_is_available[6];
    }

    return ret;
}


bool ithimetrics::SaveMetricFiles()
{
    bool ret = true;
    char buffer[512];
    ComputeMetric * cm[7] = { NULL, &cm2, &cm3, &cm4, NULL, &cm6, &cm7 };


    if (ret && ithi_folder == NULL)
    {
        ret = SetIthiFolder(ITHI_DEFAULT_FOLDER);
    }

    /* Set the date to default if not already done */
    if (ret && metric_date == NULL)
    {
        ret = SetDefaultDate(time(0));
    }

    for (int i = 0; ret && i < 7; i++)
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
                snprintf(buffer, sizeof(buffer), "%s\\M%d\\M%d-%s.csv", ithi_folder, i + 1, i + 1, metric_date);
#else
                snprintf(buffer, sizeof(buffer), "%s/M%d/M%d-%s.csv", ithi_folder, i + 1, i + 1, metric_date);
#endif
                ret = SetMetricFileNames(i, buffer);
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

    ComputeMetric * cm[7] = { NULL, &cm2, &cm3, &cm4, NULL, &cm6, &cm7 };
    FILE* F;
#ifdef _WINDOWS
    errno_t err = fopen_s(&F, file_name, "w");
    bool ret = (err == 0);
#else
    bool ret;
    F = fopen(file_name, "w");
    ret = (F != NULL);
#endif

    for (int i = 0; ret && i < 7; i++)
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

bool ithimetrics::SetAbuseFileName(char const * abuse_file_name)
{
    bool ret = copy_name(&this->abuse_file_name, abuse_file_name);

    return ret;
}

