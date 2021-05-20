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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifndef _WINDOWS
#include <errno.h>
#endif
#include "ithiutil.h"
#include "ithimetrics.h"
#include "OdiPublisher.h"

static char const * ithi_json_modified_header = "\"modified\"";

OdiPublisher::OdiPublisher()
{
}


OdiPublisher::~OdiPublisher()
{
}

/* Publish a specific metric file.
 * The metric value and name are derived from the file name.
 * The updated time is always the current time.
 */


bool OdiPublisher::PublishMetricFile(const char * metric_file_name, char const * odi_dir, char const * data_dir, time_t current_time)
{
    char odi_file_name[512];
    char json_file_name[512];
    int metric_id = 0;
    int year = 0;
    int month = 0;
    int day = 0;
    size_t name_offset = 0;
    bool ret;

    /* Parse the file name, obtain metric and date */
    ret = ithimetrics::ParseMetricFileName(metric_file_name, &metric_id, &year, &month, &day, &name_offset);

    /* Get the oid metric name */
    if (ret) {
        ret = snprintf(odi_file_name, sizeof(odi_file_name), "%s%sITHI-M%1d%s%04d%02d%02d-2359.csv",
            odi_dir, ITHI_FILE_PATH_SEP, metric_id, ITHI_FILE_PATH_SEP, year, month, day) > 0;
    }

    /* Copy the metric file */
    if (ret) {
        ret = CopyFile(metric_file_name, odi_file_name);
    }

    /* Get the JSON file name and copy it */
    if (ret) {
        ret = (snprintf(json_file_name, sizeof(json_file_name), "%s-metadata.json", odi_file_name) > 0);

        if (ret) {
            ret = CopyUpdateJsonFile(metric_id, json_file_name, data_dir, current_time);
        }
    }


    return ret;
}

/* Get the update time. This is used to fill the "modified"
 * date, which should be a full ISO 8601 date:  YYYY-MM-DDThh:mm:ss.sTZD
 *
 * eg
 *
 * "modified" : "2018-05-17T18:23:12.9UTC", 
 *
 */

bool OdiPublisher::GetUpdateTime(char * time_value, size_t time_value_size, time_t current_time)
{
    bool ret = true;
    struct tm tm;

#ifdef _WINDOWS
    if (gmtime_s(&tm, &current_time) != 0)
    {
        ret = false;
    }
#else
    tm = *gmtime(&current_time);
#endif


    if (ret)
    {
        ret = snprintf(time_value, time_value_size, "%04d-%02d-%02dT%02d:%02d:%02dUTC", tm.tm_year + 1900,
            tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec ) > 0;
    }

    return ret;
}

bool OdiPublisher::CopyFile(const char * source_file_name, const char * dest_file_name)
{
    bool ret = true;
    char line[512];
    FILE * F_out = NULL;
    FILE* F_in = ithi_file_open(source_file_name, "r");

    if (F_in != NULL)
    {
        F_out = ithi_file_open(dest_file_name, "w");
        if (F_out == NULL) {
            printf("Could not write file %s, err: %d\n", dest_file_name, errno);
            ret = false;
        }
    }
    else {
        printf("Could not read file %s, err: %d\n", source_file_name, errno);
        ret = false;
    }

    while (ret) {
        size_t nb_read = fread(line, 1, sizeof(line), F_in);

        if (nb_read > 0) {
            ret = fwrite(line, 1, nb_read, F_out) > 0;
        }
        else {
            ret = feof(F_in) != 0;
            break;
        }
    }

    if (F_in != NULL) {
        fclose(F_in);
    }

    if (F_out != NULL) {
        fclose(F_out);
    }

    return ret;
}

bool OdiPublisher::CopyUpdateJsonFile(int metric_id, const char * dest_file_name, char const * data_dir, time_t current_time)
{
    bool ret = true;
    bool updated = false;
    char updated_time[256];
    char source_file_name[512];
    char line[512] = { 0 };
    FILE * F_in = NULL;
    FILE * F_out = NULL;

    ret = snprintf(source_file_name, sizeof(source_file_name), "%s%sithi-m%1d.json",
        (data_dir == NULL)?ITHI_DEFAULT_DATA_FOLDER:data_dir,
        ITHI_FILE_PATH_SEP, metric_id);

    if (ret) {
        if (current_time == 0) {
            current_time = time(0);
        }
        ret = GetUpdateTime(updated_time, sizeof(updated_time), current_time);
    }

    if (ret) {
        F_in = ithi_file_open(source_file_name, "r");

        if (F_in != NULL)
        {
            F_out = ithi_file_open(dest_file_name, "w");
            if (F_out == NULL) {
                printf("Could not write file %s, err: %d\n", dest_file_name, errno);
                ret = false;
            }
        }
        else {
            printf("Could not read file %s, err: %d\n", source_file_name, errno);
            ret = false;
        }
    }

    while (ret) {
        if (fgets(line, sizeof(line), F_in) != NULL) {
            if (!updated) {
                if (strncmp(line, ithi_json_modified_header, strlen(ithi_json_modified_header)) == 0) {
                    ret = fprintf(F_out, "%s : \"%s\",\n", ithi_json_modified_header, updated_time) > 0;
                    updated = ret;
                    continue;
                }
            }
            ret = (fputs(line, F_out) != EOF);
        }
        else {
            ret = updated && feof(F_in) != 0;
            break;
        }
    }

    if (F_in != NULL) {
        fclose(F_in);
    }

    if (F_out != NULL) {
        fclose(F_out);
    }

    return ret;
}
