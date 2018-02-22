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
#include <ctype.h>
#include <algorithm>
#include "CsvHelper.h"
#include "CaptureSummary.h"
#include "ithipublisher.h"

ithipublisher::ithipublisher(char const * ithi_folder, int metric_id, char const * date_string)
    :
    ithi_folder(ithi_folder),
    metric_id(metric_id),
    date_string(date_string),
    last_year(0),
    last_month(0),
    first_year(0),
    first_month(0),
    nb_months(0)
{
}

ithipublisher::~ithipublisher()
{
    for (size_t i = 0; i < file_list.size(); i++)
    {
        MetricFileHolder * pmfh = file_list[i];
        delete pmfh;
        file_list[i] = NULL;
    }

    for (size_t i = 0; i < line_list.size(); i++)
    {
        MetricLine * pml = line_list[i];
        delete pml;
        line_list[i] = NULL;
    }
}

bool ithipublisher::CollectMetricFiles()
{
    char dir_met_name[512];
    bool ret = snprintf(dir_met_name, sizeof(dir_met_name), "%s%sM%d%s", ithi_folder, ITHI_FILE_PATH_SEP, metric_id, ITHI_FILE_PATH_SEP) > 0;

    if (ret)
    {
        DIR *dir_met;

        dir_met = opendir(dir_met_name);
        if (dir_met == NULL)
        {
            ret = false;
        }
        else
        {
            struct dirent *file_ent;

            while (ret && (file_ent = readdir(dir_met)) != NULL)
            {
                MetricFileHolder met_file;

                if (ParseFileName(&met_file, file_ent->d_name, metric_id))
                {
                    /* Check whether this is the latest in list for the month */
                    bool found_already = false;

                    for (size_t i = 0; i < file_list.size(); i++)
                    {
                        if (file_list[i]->year == met_file.year &&
                            file_list[i]->month == met_file.month)
                        {
                            if (met_file.day >= file_list[i]->day)
                            {
                                *(file_list[i]) = met_file;
                            }
                            found_already = true;
                            break;
                        }
                    }

                    if (!found_already)
                    {
                        MetricFileHolder * pmfh = new MetricFileHolder();

                        if (pmfh == NULL)
                        {
                            ret = false;
                        }
                        else
                        {
                            *pmfh = met_file;
                            file_list.push_back(pmfh);
                        }
                    }
                }
            }

            closedir(dir_met);

            ret = file_list.size() > 0;
        }
    }

    if (ret)
    {
        /* Sort the file list from earlier to last */
        std::sort(file_list.begin(), file_list.end(), ithipublisher::MetricFileIsEarlier);

        /* Compute the first and last year and months */
        size_t last_index = file_list.size() - 1;
        last_year = file_list[last_index]->year;
        last_month = file_list[last_index]->month;
        last_day = file_list[last_index]->day;
        nb_months = 12;

        first_year = last_year;
        first_month = last_month - 11;
        if (first_month < 1)
        {
            first_month += 12;
            first_year -= 1;
        }

        /* Remove the list elements that are too old*/
        size_t to_erase = 0;
        while (file_list[to_erase]->year < first_year ||
            (file_list[to_erase]->year == first_year &&
                file_list[to_erase]->month < first_month))
        {
            to_erase++;
        }

        if (to_erase > 0)
        {
            for (size_t i = 0; i < to_erase; i++)
            {
                MetricFileHolder * pmf = file_list[i];
                delete pmf;
                file_list[i] = NULL;
            }
            file_list.erase(file_list.begin(), file_list.begin() + to_erase);
        }

        /* Adjust the first year and month if needed */
        if (first_year < file_list[0]->year)
        {
            first_year = file_list[0]->year;
            nb_months -= (file_list[0]->month + 12 - first_month);
            first_month = file_list[0]->month;
        }
        else if (first_year == file_list[0]->year &&
            first_month < file_list[0]->month)
        {
            nb_months -= (file_list[0]->month - first_month);
            first_month = file_list[0]->month;
        }

        /* Load the selected data */
        for (size_t i = 0; ret &&  i < file_list.size(); i++)
        {
            ret = LoadFileData(i, dir_met_name);
        }

        if (ret)
        {
            std::sort(line_list.begin(), line_list.end(), MetricLineIsLower);
        }
    }

    return ret;
}

/*
 * Metric files have the name format "MX-YYYY-MM-DD.csv".
 */

bool ithipublisher::ParseFileName(MetricFileHolder * file, const char * name, int metric_id)
{
    size_t ch_index = 0;
    size_t char_after_sep_index = 0;
    size_t name_len = strlen(name);
    bool ret = name_len < sizeof(file->file_name);

    /* Find the last separator in the file name */
    if (ret)
    {
        while (name[ch_index] != 0)
        {
            if (file->file_name[ch_index] == ITHI_FILE_PATH_SEP[0])
            {
                char_after_sep_index = ch_index + 1;
            }
            ch_index++;
        }

        /* Check that the name is well formed */
        ret &= (char_after_sep_index + 17) < sizeof(file->file_name);
    }

    if (ret)
    {
        ret = name[char_after_sep_index] == 'M' &&
            name[char_after_sep_index + 1] == '0' + metric_id &&
            name[char_after_sep_index + 2] == '-' &&
            name[char_after_sep_index + 7] == '-' &&
            name[char_after_sep_index + 10] == '-' &&
            name[char_after_sep_index + 13] == '.' &&
            name[char_after_sep_index + 14] == 'c' &&
            name[char_after_sep_index + 15] == 's' &&
            name[char_after_sep_index + 16] == 'v' &&
            name[char_after_sep_index + 17] == 0;
    }

    if (ret)
    {
        char digits[5];
        int val[3] = { 0, 0, 0 };
        const int delta[3] = { 3, 8, 11 };
        const int len[3] = { 4, 2, 2 };

        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < len[i]; j++)
            {
                digits[j] = name[char_after_sep_index + delta[i] + j];
                ret &= (isdigit(digits[j]) != 0);
            }
            digits[len[i]] = 0;
            val[i] = atoi(digits);
        }

        file->year = val[0];
        file->month = val[1];
        file->day = val[2];
    }

    if (ret)
    {
        memcpy(file->file_name, name, name_len + 1);
    }

    return ret;
}

bool ithipublisher::LoadFileData(int file_index, char const * dir_met_name)
{
    FILE* F;
    MetricLine * line;
    char buffer[512];
    char file_name[512];
    bool ret = snprintf(file_name, sizeof(file_name), "%s%s", dir_met_name, file_list[file_index]->file_name) > 0;

    if (ret) {
#ifdef _WINDOWS
        errno_t err = fopen_s(&F, file_name, "r");
        ret = (err == 0 && F != NULL);
#else
        F = fopen(file_name, "r");
        ret = (F != NULL);
#endif
    }

    while (ret && fgets(buffer, sizeof(buffer), F))
    {
        int start = 0;
        line = new MetricLine;

        if (line == NULL)
        {
            ret = false;
            break;
        }

        start = CsvHelper::read_string(line->metric_name, sizeof(line->metric_name), start, buffer, sizeof(buffer));
        start = CsvHelper::read_string(line->key_value, sizeof(line->key_value), start, buffer, sizeof(buffer));
        start = CsvHelper::read_double(&line->frequency, start, buffer, sizeof(buffer));
        line->year = file_list[file_index]->year;
        line->month = file_list[file_index]->month;

        /* TODO: check that the parsing is good */

        line_list.push_back(line);
    }

    if (F != NULL)
    {
        fclose(F);
    }
    return ret;
}


bool ithipublisher::MetricFileIsEarlier(MetricFileHolder * f1, MetricFileHolder * f2)
{
    return (f1->year < f2->year ||
        (f1->year == f2->year && f1->month < f2->month));
}

bool ithipublisher::MetricLineIsLower(MetricLine * x, MetricLine * y)
{
    bool ret = false;
    int cmp;

    cmp = CaptureSummary::compare_string(x->metric_name, y->metric_name);

    if (cmp > 0)
    {
        ret = true;
    }
    else if (cmp == 0)
    {
        cmp = CaptureSummary::compare_string(x->key_value, y->key_value);
        if (cmp > 0)
        {
            ret = true;
        }
        else if (cmp == 0)
        {
            if (x->year < y->year)
            {
                ret = true;
            }
            else if (x->year == y->year && x->month < y->month)
            {
                ret = true;
            }
        }
    }

    return ret;
}

bool ithipublisher::Publish(char const * web_folder)
{
    /* Create file name for the metric */
    FILE * F = NULL;
    char file_name[512];
    bool ret = snprintf(file_name, sizeof(file_name), "%s%sM%dData.txt", 
        web_folder, ITHI_FILE_PATH_SEP, metric_id) > 0;

    if (ret)
    {
        /* Create the file */
#ifdef _WINDOWS
        errno_t err = fopen_s(&F, file_name, "w");
        ret = (err == 0 && F != NULL);
#else
        F = fopen(file_name, "w");
        ret = (F != NULL);
#endif
    }

    if (ret)
    {
        /* Opening braces and date */
        ret = fprintf(F, "{\n\"date\" : \"%04d/%02d/%02d\",\n", last_year, last_month, last_day) > 0;
        /* Data */
        if (ret)
        {
            switch (metric_id)
            {
            case 7:
                ret = PublishDataM7(F);
                break;
            case 2:
                ret = PublishDataM2(F);
                break;
            case 3:
            case 4:
            case 6:
            default:
                ret = fprintf(F, "// No data yet for metric M%d\n", metric_id) > 0;
                break;
            }
        }
        /* Closing braces */
        if (ret)
        {
            fprintf(F, "}\n");
        }
    }

    /* Close the file */
    if (F != NULL)
    {
        fclose(F);
    }

    return ret;
}

bool ithipublisher::GetVector(char const * metric_name, char const * key_value, double * metric)
{
    size_t line_index = 0;
    int current_year = first_year;
    int current_month = first_month;

    while (line_index < line_list.size())
    {
        int cmp = strcmp(line_list[line_index]->metric_name, metric_name);

        if (cmp > 0)
        {
            break;
        }
        else if (cmp == 0)
        {
            if (key_value == NULL)
            {
                break;
            }
            else
            {
                cmp = strcmp(line_list[line_index]->key_value, key_value);
                if (cmp <= 0)
                {
                    break;
                }
            }
        }
        line_index++;
    }
        
    for (int i = 0; i < nb_months; i++)
    {

        if (line_index > line_list.size() ||
            strcmp(line_list[line_index]->metric_name, metric_name) != 0 ||
            (key_value != NULL && strcmp(line_list[line_index]->key_value, key_value) != 0) ||
            line_list[line_index]->year > current_year ||
            (line_list[line_index]->year == current_year &&
                line_list[line_index]->month > current_month))
        {
            metric[i] = 0;
        }
        else
        {
            metric[i] = line_list[line_index]->frequency;
            line_index++;
        }
        current_month++;
        if (current_month > 12)
        {
            current_month = 1;
            current_year++;
        }
    }

    return true;
}

bool ithipublisher::PublishDataM2(FILE * F)
{
    double m2x[12];
    bool ret = true;
    char const * subMet[4] = { "M2.1", "M2.2", "M2.3", "M2.4" };

    fprintf(F, "\"m2Val\" : [\n");
    for (int m = 0; m < 4; m++)
    {
        if (GetVector(subMet[m], NULL, m2x))
        {
            /* m2x is present */
            fprintf(F, "[");
            for (int i = 0; i < nb_months; i++)
            {
                if (i != 0)
                {
                    fprintf(F, ",");
                }

                ret &= fprintf(F, "%8f", m2x[i]) > 0;
            }
            if (m == 3) {
                fprintf(F, "]");
            } else {
                fprintf(F, "],\n");
            }
        }
        else
        {
            ret = false;
        }
    }
    fprintf(F, "]\n");

    return ret;
}


bool ithipublisher::PublishDataM7(FILE * F)
{
    double m7[12];
    bool ret = true;

    fprintf(F, "\"M7DataSet\" : [");
    if (GetVector("M7", NULL, m7))
    {
        /* M7 is present */
        for (int i = 0; i < nb_months; i++)
        {
            if (i != 0)
            {
                fprintf(F, ",");
            }

            /* Multiply metric value by 100, since we want to display percentages */
            ret &= fprintf(F, "%8f", 100*m7[i]) > 0;
        }
    }
    fprintf(F, "]\n");

    return ret;
}
