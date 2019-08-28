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
#else
#include <dirent.h>
#endif
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <algorithm>
#include "DnsStats.h"
#include "CsvHelper.h"
#include "CaptureSummary.h"
#include "ComputeM6.h"
#include "ithimetrics.h"
#include "ithiutil.h"
#include "ithipublisher.h"

// Place holder data, if metrics do not provide actual number
#define NB_GTLD 1212
#define NB_REGISTRARS 2463

ithipublisher::ithipublisher(char const * ithi_folder, int metric_id)
    :
    ithi_folder(ithi_folder),
    metric_id(metric_id),
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
        first_year = file_list[0]->year;
        first_month = file_list[0]->month;

        nb_months = 1 + last_month - first_month + 12 * (last_year - first_year);

        /* Load the selected data */
        for (size_t i = 0; ret &&  i < file_list.size(); i++)
        {
            ret = LoadFileData((int)i, dir_met_name);
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
 * This is tested by the name parser in "ithimetrics".
 */
bool ithipublisher::ParseFileName(MetricFileHolder * file, const char * name, int metric_id)
{
    int candidate_metric = 0;
    size_t name_offset = 0;
    size_t name_len = strlen(name);
    bool ret = name_len < sizeof(file->file_name);

    if (ret) 
    {
        ret = ithimetrics::ParseMetricFileName(name, &candidate_metric, &file->year, &file->month, &file->day, &name_offset);
        ret &= (candidate_metric == metric_id);
    }


    if (ret)
    {
        memcpy(file->file_name, name, name_len + 1);
    }

    return ret;
}

bool ithipublisher::LoadFileData(int file_index, char const * dir_met_name)
{
    FILE* F = NULL;
    MetricLine * line;
    char buffer[512];
    char file_name[512];
    bool ret = snprintf(file_name, sizeof(file_name), "%s%s", dir_met_name, file_list[file_index]->file_name) > 0;

    if (ret) {
        F = ithi_file_open(file_name, "r");
        ret = (F != NULL);
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
        CsvHelper::read_double(&line->frequency, start, buffer, sizeof(buffer));
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

bool ithipublisher::MetricNameLineIsBigger(MetricNameLine l1, MetricNameLine l2)
{
    bool ret = false;

    if (l1.current > l2.current)
    {
        ret = true;
    }
    else if (l1.current == l2.current)
    {
        if (l1.average > l2.average)
        {
            ret = true;
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
        F = ithi_file_open(file_name, "w");
        ret = (F != NULL);
    }

    if (ret)
    {
        /* Opening braces and date */
        ret = fprintf(F, "{\n\"date\" : \"%04d/%02d\",\n", last_year, last_month) > 0;
        ret &= fprintf(F, "\"year\" : %d,\n", last_year) > 0;
        ret &= fprintf(F, "\"month\" : %d,\n", last_month) > 0;
        /* Data */
        if (ret)
        {
            switch (metric_id)
            {
            case 1:
                ret = PublishDataM1(F);
                break;
            case 2:
                ret = PublishDataM2(F);
                break;
            case 3:
                ret = PublishDataM3(F);
                break;
            case 4:
                ret = PublishDataM4(F);
                break;
            case 5:
                ret = PublishDataM5(F);
                break;
            case 6:
                ret = PublishDataM6(F);
                break;
            case 7:
                ret = PublishDataM7(F);
                break;
            case 8:
                ret = PublishDataM8(F);
                break;
            default:
                ret = fprintf(F, "\"error\" : \"No data yet for metric M%d\"\n", metric_id) > 0;
                break;
            }
        }
        /* Closing braces */
        ret &= (fprintf(F, "}\n") > 0);
    }

    /* Close the file */
    if (F != NULL)
    {
        fclose(F);
    }

    return ret;
}

bool ithipublisher::GetVector(char const * metric_name, char const * key_value, std::vector<double> * metric)
{
    size_t line_index = 0;
    int current_year = first_year;
    int current_month = first_month;

    metric->clear();

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
                if (cmp >= 0)
                {
                    break;
                }
            }
        }
        line_index++;
    }
        
    for (int i = 0; i < nb_months; i++)
    {
        double m;

        if (line_index >= line_list.size() ||
            strcmp(line_list[line_index]->metric_name, metric_name) != 0 ||
            (key_value != NULL && strcmp(line_list[line_index]->key_value, key_value) != 0) ||
            line_list[line_index]->year > current_year ||
            (line_list[line_index]->year == current_year &&
                line_list[line_index]->month > current_month))
        {
            m = 0;
        }
        else
        {
            m = line_list[line_index]->frequency;
            line_index++;
        }
        metric->push_back(m);

        current_month++;
        if (current_month > 12)
        {
            current_month = 1;
            current_year++;
        }
    }

    return true;
}

bool ithipublisher::GetCurrent(char const * metric_name, char const * key_value, double * current)
{
    std::vector<double> val;
    bool ret = GetVector(metric_name, key_value, &val);

    *current = val[(size_t)nb_months - 1];

    return ret;
}

bool ithipublisher::GetAverageAndCurrent(char const * metric_name, char const * key_value, double * average, double * current)
{
    std::vector<double>  val;
    bool ret = GetVector(metric_name, key_value, &val);
    double sum = 0;

    *average = 0;
    *current = 0;

    if (ret)
    {
        if (nb_months > 1)
        {
            for (int i = 0; i < nb_months - 1; i++)
            {
                sum += val[i];
            }
            *average = sum / (double)(nb_months - 1);
        }

        *current = val[(size_t)nb_months - 1];
    }
    
    return ret;
}

bool ithipublisher::GetNameList(char const * metric_name, std::vector<MetricNameLine>* name_list)
{
    size_t line_index = 0;
    double sum = 0;
    MetricNameLine current_name = { NULL, 0, 0 };

    /* Skip to beginning of the metric */
    while (line_index < line_list.size())
    {
        int cmp = strcmp(line_list[line_index]->metric_name, metric_name);

        if (cmp > 0)
        {
            break;
        }
        else if (cmp == 0)
        {
            current_name.name = line_list[line_index]->key_value;
            break;
        }
        line_index++;
    }

    /* Extract the lines and compute the averages */
    while (line_index < line_list.size() && strcmp(line_list[line_index]->metric_name, metric_name) == 0)
    {
        if (current_name.name != NULL && strcmp(line_list[line_index]->key_value, current_name.name) != 0)
        {
            if (nb_months > 1)
            {
                current_name.average = sum / (nb_months - 1);
            }
            name_list->push_back(current_name);
            current_name.name = line_list[line_index]->key_value;
            current_name.average = 0;
            current_name.current = 0;
            sum = 0;
        }
        
        if (line_list[line_index]->year == last_year &&
            line_list[line_index]->month == last_month)
        {
            current_name.current = line_list[line_index]->frequency;
        }
        else
        {
            sum += line_list[line_index]->frequency;
        }

        line_index++;
    }
    /* Push the last entry if it was filled up */
    if (current_name.name != NULL)
    {
        if (nb_months > 1)
        {
            current_name.average = sum / (nb_months - 1);
        }
        name_list->push_back(current_name);
    }

    /* Sort from bigger to lower */
    std::sort(name_list->begin(), name_list->end(), ithipublisher::MetricNameLineIsBigger);

    return true;
}

bool ithipublisher::PrintVector(FILE * F, std::vector<double> * vx, double mult)
{
    bool ret = (fprintf(F, "[") > 0);
    for (size_t i = 0; i < vx->size(); i++)
    {
        if (i != 0)
        {
            ret &= fprintf(F, ", ") > 0;
        }

        ret &= fprintf(F, "%8f", mult*(*vx)[i]) > 0;
    }

    ret &= fprintf(F, "]") > 0;

    return ret;
}

bool ithipublisher::PrintNameVectorMetric(FILE * F, char const * sub_met_name, char const * metric_name, double mult)
{
    std::vector<double>  mvec;
    std::vector<MetricNameLine> name_list;
    bool ret = GetNameList(sub_met_name, &name_list);


    ret &= fprintf(F, "\"%s\" : [", metric_name) > 0;

    for (size_t i = 0; ret && i < name_list.size(); i++) {
        if (i == 0) {
            ret = fprintf(F, "\n") > 0;
        } else {
            ret = fprintf(F, ",\n") > 0;
        }
        ret &= (fprintf(F, "[\"%s\", ", name_list[i].name) > 0);

        if (ret)
        {
            ret = GetVector(sub_met_name, name_list[i].name, &mvec);

            if (ret)
            {
                ret = PrintVector(F, &mvec, mult);
            }
        }
        ret &= fprintf(F, "]") > 0;
    }

    ret &= fprintf(F, "],\n") > 0;

    return ret;
}

bool ithipublisher::PrintNameList(FILE * F, std::vector<MetricNameLine>* name_list, double mult)
{
    bool ret = true;

    for (size_t i = 0; ret && i < name_list->size(); i++)
    {
        if (i != 0)
        {
            ret = fprintf(F, ",\n") > 0;
        }
        ret &= (fprintf(F, "[\"%s\", %8f, %8f]", 
            (*name_list)[i].name, mult * (*name_list)[i].current, mult * (*name_list)[i].average) > 0);
    }

    return ret;
}

bool ithipublisher::PublishDataM1(FILE * F)
{
    std::vector<double>  m1x;
    bool ret = true;
    char const * subMet[3] = { "M1.1", "M1.2", "M1.3"};
    char const * M1Metric_nbRegistrars = "M1.4";
    double d_registrars = 0;

    if (ret && !GetCurrent(M1Metric_nbRegistrars, NULL, &d_registrars)) {
        d_registrars = NB_REGISTRARS;
    }

    if (ret) {
        ret &= fprintf(F, "\"NbRegistrars\" : %d,\n", (int)d_registrars) > 0;
    }

    ret &= fprintf(F, "\"m1Val\" : [") > 0;
    for (int m = 0; ret && m < 3; m++)
    {
        ret &= fprintf(F, "%s", (m == 0) ? "\n" : ",\n") > 0;

        if (ret) {
            if ((ret = GetVector(subMet[m], NULL, &m1x))) {
                ret = PrintVector(F, &m1x, 1.0);
            }
        }
    }

    ret &= fprintf(F, "]\n") > 0;

    return ret;
}

bool ithipublisher::PublishDataM2(FILE * F)
{
    std::vector<double> m2x;
    bool ret = true;
    char const * subMet[24] = {
        "M2.1.1.1", "M2.1.2.1", "M2.1.3.1", "M2.1.4.1",
        "M2.1.1.2", "M2.1.2.2", "M2.1.3.2", "M2.1.4.2",
        "M2.1.1.3", "M2.1.2.3", "M2.1.3.3", "M2.1.4.3",
        "M2.2.1.1", "M2.2.2.1", "M2.2.3.1", "M2.2.4.1",
        "M2.2.1.2", "M2.2.2.2", "M2.2.3.2", "M2.2.4.2",
        "M2.2.1.3", "M2.2.2.3", "M2.2.3.3", "M2.2.4.3" };
    char const * M2Metric_nbGtld = "M2.1.5";
    char const * M2Metric_nbRegistrars = "M2.2.5";
    double d_gtld = 0;
    double d_registrars = 0;

    if (ret && !GetCurrent(M2Metric_nbGtld, NULL, &d_gtld)) {
        d_gtld = NB_GTLD;
    }
    if (ret && !GetCurrent(M2Metric_nbRegistrars, NULL, &d_registrars)) {
        d_registrars = NB_REGISTRARS;
    }

    if (ret) {
        ret &= fprintf(F, "\"NbGtld\" : %d,\n", (int)d_gtld) > 0;
        ret &= fprintf(F, "\"NbRegistrars\" : %d,\n", (int)d_registrars) > 0;
    }

    ret &= fprintf(F, "\"m2Val\" : [") > 0;

    for (int m = 0; ret && m < 24; m++)
    {
        ret = GetVector(subMet[m], NULL, &m2x);
        if (ret) {
            ret = fprintf(F, "%s", (m==0)?"\n":",\n") > 0;
            ret &= PrintVector(F, &m2x, 1.0);
        }
    }
    
    ret &= fprintf(F, "]\n") > 0;

    return ret;
}

bool ithipublisher::PublishDataM3(FILE * F)
{
    bool ret = true;
    const char * sub_met[8] = { "M3.1", "M3.2", "M3.3.1", "M3.3.2", "M3.3.3", "M3.4", "M3.5", "M3.6" };
    const char * met_data_name[8] = { "M31", "M32", "m331Set", "m332Set", "m333Set", "M34", "M35", "M36" };
    std::vector<double> m31, m32, mvec;
    
    ret = fprintf(F, "\"%s\" : ", met_data_name[0]) > 0;

    if (ret)
    {
        ret = GetVector(sub_met[0], NULL, &m31);

        if (ret)
        {
            ret = PrintVector(F, &m31, 100.0);
        }
    }
    ret &= fprintf(F, ",\n") > 0;

    ret &= fprintf(F, "\"%s\" : ", met_data_name[1]) > 0;

    if (ret)
    {
        ret = GetVector(sub_met[1], NULL, &m32);

        for (int i = 0; ret && i < nb_months; i++)
        {
            m32[i] *= (1 - m31[i]);
            if (m32[i] < 0)
            {
                m32[i] = 0;
            }
        }

        if (ret)
        {
            ret = PrintVector(F, &m32, 100.0);
        }
    }
    ret &= fprintf(F, ",\n") > 0;

    for (int m = 2; ret && m<5; m++)
    {
        ret = PrintNameVectorMetric(F, sub_met[m], met_data_name[m], 100.0);
    }

    /* Add M3.4 data */
    if (ret) {
        ret &= fprintf(F, "\"%s\" : [", met_data_name[5]) > 0;
        ret &= GetVector("M3.4.1", NULL, &mvec);
        ret &= PrintVector(F, &mvec, 100.0);
        ret &= fprintf(F, ",\n") > 0;
        ret &= PublishOptTable(F, "M3.4.2");
        ret &= fprintf(F, "]") > 0;
    }

    /* Add M3.5 and M3.6 */
    for (int m = 6; ret && m<8; m++)
    {
        ret &= GetVector(sub_met[m], NULL, &mvec);
        ret &= fprintf(F, ",\n") > 0;
        ret &= fprintf(F, "\"%s\" : ", met_data_name[m]) > 0;
        ret &= PrintVector(F, &mvec, 100.0);
    }
    ret &= fprintf(F, "\n") > 0;

    return ret;
}

bool ithipublisher::PublishDataM4(FILE * F)
{
    bool ret = true;
    std::vector<double> mvec;
    const char * sub_met[5] = { "M4.1", "M4.2", "M4.3", "M4.5", "M4.6" };
    const char * met_data_name[5] = { "M41Data", "M42DataSet", "M43DataSet", "M45Data", "M46Data" };

    ret = GetVector(sub_met[0], NULL, &mvec);
    if (ret)
    {
        ret = fprintf(F, "\"%s\" :", met_data_name[0]) > 0;
        ret &= PrintVector(F, &mvec, 100);
        ret &= fprintf(F, ",\n") > 0;
    }

    for (int m=1; ret && m<3; m++)
    {
        ret = PrintNameVectorMetric(F, sub_met[m], met_data_name[m], 100.0);
    }

    /* Only publish M4.5, not M4.6, until we clarify M4.6 computation */
    for (int m = 3; ret && m < 4; m++)
    {
        ret = fprintf(F, "\"%s\" : ", met_data_name[m]);

        if (GetVector(sub_met[m], NULL, &mvec))
        {
            /* M7.x is present */
            ret = PrintVector(F, &mvec, 100.0);

            if (m == 3) {
                ret &= (fprintf(F, "\n") > 0);
            }
            else {
                ret &= (fprintf(F, ",\n") > 0);
            }
        }
    }


    return ret;
}

bool ithipublisher::PublishDataM5(FILE * F)
{
    bool ret = true;
    const char * subMet[] = { "M5.1.1", "M5.1.2", "M5.1.3", "M5.1.4", "M5.1.5", "M5.1.6",
        "M5.2.1", "M5.2.2", "M5.2.3", "M5.3.1", "M5.3.2", "M5.4.1", "M5.4.2", "M5.5" };
    const size_t nbSubMet = sizeof(subMet) / sizeof(const char *);

    ret &= fprintf(F, "\"M5\" : [") > 0;

    for (size_t i=0; i<nbSubMet; i++) {
        std::vector<double> mvec;

        ret = GetVector(subMet[i], NULL, &mvec);

        if (i != 0) {
            ret &= fprintf(F, ",\n") > 0;
        }

        ret &= fprintf(F, "{ \"v\" : ") > 0;

        if (ret)
        {
            ret = PrintVector(F, &mvec, 1.0);
        }

        ret &= fprintf(F, "}") > 0;
    }
    ret &= fprintf(F, "]\n") > 0;

    return ret;
}


bool ithipublisher::PublishDataM6(FILE * F)
{
    bool ret = true;
    char subMetX[64];
    char const * subMet[18] = {
        "M6.DNS.01", "M6.DNS.02", "M6.DNS.03", "M6.DNS.04", "M6.DNS.05", "M6.DNS.06", 
        "M6.DNS.07", "M6.DNS.08", "M6.DNS.09", "M6.DNS.10", "M6.DNS.11", "M6.DNS.12", 
        "M6.DNSSEC.1", "M6.DNSSEC.2", "M6.DNSSEC.3", "M6.DANE.1", "M6.DANE.2", "M6.DANE.3"
    };
    char const * subName[18] = {
        "DNS CLASSes",
        "Resource Record (RR) TYPEs",
        "DNS OpCodes",
        "DNS RCODEs",
        "AFSDB RR Subtype",
        "DHCID RR Identifier Type Codes",
        "DNS Label Types",
        "DNS EDNS0 Option Codes (OPT)",
        "DNS Header Flags",
        "EDNS Header Flags (16 bits)",
        "EDNS version Number (8 bits)",
        "Child Synchronization (CSYNC) Flags ",
        "DNS KEY Record Diffie-Hellman Prime Lengths",
        "DNS KEY Record Diffie-Hellman Well-Known Prime/Generator Pairs",
        "DNS Security Algorithm Numbers",
        "TLSA Certificate Usages",
        "TLSA Selectors",
        "TLSA Matching Types"
    };

    ret = fprintf(F, "\"m6Val\" : [\n") > 0;
    for (int m = 0; ret && m < 18; m++)
    {
        ret = fprintf(F, "[ \"%s\", \"%s\"", subMet[m], subName[m]) > 0;

        for (int i = 1; ret && i < 3; i++)
        {
            ret = snprintf(subMetX, sizeof(subMetX), "%s.%d", subMet[m], i) > 0;

            if (ret)
            {
                double average, current;

                ret = GetAverageAndCurrent(subMetX, NULL, &average, &current);

                /* Multiply metric value by 100, since we want to display percentages */
                ret &= fprintf(F, ", %8f, %8f", 100 * current, 100 * average) > 0;
            }
        }

        if (ret) {
            metric6_def_t const * table = ComputeM6::GetTable(subMet[m]);
            std::vector<MetricNameLine> name_list;
            double total_current = 0;
            double total_average = 0;
            double current_divider;
            double average_divider;

            ret = snprintf(subMetX, sizeof(subMetX), "%s.3", subMet[m]) > 0;

            ret &= fprintf(F, ",[") > 0;

            if (ret)
            {
                ret = GetNameList(subMetX, &name_list);
            }

            for (size_t l = 0; ret && l < name_list.size(); l++)
            {
                total_current += name_list[l].current;
                total_average += name_list[l].average;
            }
            current_divider = (total_current > 0) ? 100.0 / total_current : 1.0;
            average_divider = (total_average > 0) ? 100.0 / total_average : 1.0;

            for (size_t l = 0; ret && l < name_list.size(); l++)
            {
                char const * key_name = "";

                if (table != NULL) {
                    uint32_t key_val = (uint32_t)atoi(name_list[l].name);

                    key_name = "???";

                    for (size_t t = 0; t < table->nb_registered; t++) {
                        if (table->registry[t].key == key_val) {
                            key_name = table->registry[t].key_name;
                            break;
                        }
                    }
                }

                if (l == 0) {
                    ret = fprintf(F, "\n") > 0;
                } else {
                    ret = fprintf(F, ",\n") > 0;
                }

                ret &= (fprintf(F, "[%s, \"%s\", %1f, %1f]",
                    name_list[l].name, key_name, 
                    current_divider*name_list[l].current, average_divider*name_list[l].average) > 0);
            }

            ret &= fprintf(F, "]") > 0;
        }

        if (m == 17)
        {
            ret &= (fprintf(F, "]]\n") > 0);
        }
        else
        {
            ret &= (fprintf(F, "],\n") > 0);
        }
    }

    return ret;
}

bool ithipublisher::PublishDataM7(FILE * F)
{
    std::vector<double> m7x;
    char subMetX[16];
    bool ret = true;

    fprintf(F, "\"M7DataSet\" : [\n");

    for (int i = 1; ret && i <= 2; i++) {

        ret = snprintf(subMetX, sizeof(subMetX), "M7.%d", i) > 0;

        if (ret) {
            if (GetVector(subMetX, NULL, &m7x))
            {
                /* M7.x is present */
                ret = PrintVector(F, &m7x, 100.0);

                if (i == 2) {
                    ret &= (fprintf(F, "\n") > 0);
                }
                else {
                    ret &= (fprintf(F, ",\n") > 0);
                }
            }
        }
    }
    fprintf(F, "]\n");

    return ret;
}

bool ithipublisher::PublishOptTable(FILE * F, char const * metric_name)
{
    std::vector<MetricNameLine> name_list;
    const metric6_def_t * opt_def = ComputeM6::GetTable("M6.DNS.08");
    bool ret = GetNameList(metric_name, &name_list);
    std::vector<double>  mvec;

    ret &= (opt_def != NULL);

    ret &= fprintf(F, "[") > 0;

    for (size_t i = 0; ret && i < name_list.size(); i++) {
        /* Parse name to code */
        char const * opt_long_name = NULL;
        uint32_t opt_code = (uint32_t)atoi(name_list[i].name);

        for (size_t j = 0; j < opt_def->nb_registered; j++) {
            if (opt_def->registry[j].key == opt_code) {
                opt_long_name = opt_def->registry[j].key_name;
                break;
            }
        }

        if (i > 0) {
            ret &= fprintf(F, ",\n") > 0;
        }

        if (opt_long_name != NULL) {
            ret &= fprintf(F, "[ \"%s(%s)\",", opt_long_name, name_list[i].name) > 0;
        }
        else {
            ret &= fprintf(F, "[ \"%s\",", name_list[i].name) > 0;
        }

        if (ret) {
            ret = GetVector(metric_name, name_list[i].name, &mvec);

            if (ret) {
                ret = PrintVector(F, &mvec, 100);
            }
        }

        ret &= fprintf(F, "]") > 0;
    }

    ret &= fprintf(F, "]\n") > 0;

    return ret;
}

bool ithipublisher::PublishDataM8(FILE * F)
{
    std::vector<double> mvec;
    bool ret = true;
    char const * met_data_name[4] = { "M81", "M82", "M83", "M84" };
    char const * sub_met[4] = { "M8.1", "M8.2", "M8.3", "M8.4" };


    /* Publish M8.1 */
    ret &= GetVector(sub_met[0], NULL, &mvec);
    ret &= fprintf(F, "\"%s\" : ", met_data_name[0]) > 0;
    ret &= PrintVector(F, &mvec, 100.0);
    ret &= fprintf(F, ",\n") > 0;

    /* Add M8.2 data */
    if (ret) {
        ret &= fprintf(F, "\"%s\" : [", met_data_name[1]) > 0;
        ret &= GetVector("M8.2.1", NULL, &mvec);
        ret &= PrintVector(F, &mvec, 100.0);
        ret &= fprintf(F, ",\n") > 0;
        ret &= PublishOptTable(F, "M8.2.2");
        ret &= fprintf(F, "]") > 0;
    }

    /* Add M8.3 and M8.4 */
    for (int m = 2; ret && m<4; m++)
    {
        ret &= GetVector(sub_met[m], NULL, &mvec);
        ret &= fprintf(F, ",\n") > 0;
        ret &= fprintf(F, "\"%s\" : ", met_data_name[m]) > 0;
        ret &= PrintVector(F, &mvec, 100.0);
    }
    ret &= fprintf(F, "\n") > 0;

    return ret;
}


static const int ithiIndexPublisherInputMetrics[4] = { 2, 3, 4, 5 };

ithiIndexPublisher::ithiIndexPublisher(char const * ithi_folder) :
    ithi_folder(ithi_folder)
{
    for (int i = 0; i < 4; i++) {
        MData[i] = NULL;
    }
    for (int i = 0; i < 3; i++) {
        threeRootNames[i] = NULL;
        threeStubNames[i] = NULL;
    }
}

ithiIndexPublisher::~ithiIndexPublisher()
{
    for (int i = 0; i < 4; i++) {
        if (MData[i] != NULL) {
            delete MData[i];
            MData[i] = NULL;
        }
    }


    for (int i = 0; i < 3; i++) {
        if (threeRootNames[i] != NULL) {
            delete[] threeRootNames[i];
            threeRootNames[i] = NULL;
        }

        if (threeStubNames[i] != NULL) {
            delete[] threeStubNames[i];
            threeStubNames[i] = NULL;
        }
    }
}

bool ithiIndexPublisher::CollectMetricFiles()
{
    bool ret = true;

    for (int i = 0; ret && i < 4; i++) {
        MData[i] = new ithipublisher(ithi_folder, ithiIndexPublisherInputMetrics[i]);
        if (MData[i] == NULL) {
            ret = false;
        }
    }

    if (ret) {
        for (int i = 0; ret && i < 4; i++) {
            ret = MData[i]->CollectMetricFiles();
        }
    }

    return ret;
}

bool ithiIndexPublisher::Publish(char const * web_folder)
{
    std::vector<double> mvec;
    /* Find the date */
    int year = 0;
    int month = 0;
    for (int i = 0; i < 4; i++) {
        if (MData[i]->last_year > year ||
            (MData[i]->last_year == year &&
                MData[i]->last_month > month)) {
            year = MData[i]->last_year;
            month = MData[i]->last_month;
        }
    }

    /* Create file name for the metric */
    FILE * F = NULL;
    char file_name[512];
    bool ret = snprintf(file_name, sizeof(file_name), "%s%sIndexData.txt",
        web_folder, ITHI_FILE_PATH_SEP) > 0;

    if (ret)
    {
        /* Create the file */
        F = ithi_file_open(file_name, "w");
        ret = (F != NULL);
    }

    if (ret)
    {
        /* Opening braces and date */
        ret = fprintf(F, "{\n\"date\" : \"%04d/%02d\",\n", year, month) > 0;
        ret &= fprintf(F, "\"year\" : %d,\n", year) > 0;
        ret &= fprintf(F, "\"month\" : %d,\n", month) > 0;
        ret &= fprintf(F, "\"mDate\" : [") > 0;
        for (int i = 0; ret && i < 4; i++) {
            if (i != 0) {
                ret &= fprintf(F, ", ") > 0;
            }
            ret &= fprintf(F, "[%d, %d]", MData[i]->last_year, MData[i]->last_month) > 0;
        }
        ret &= fprintf(F, "],\n\"mData\" : [") > 0;

        /* First line: Metric M3.1 */
        if (ret)
        {
            ret = MData[1]->GetVector("M3.1", NULL, &mvec);
            if (ret) {
                ret = MData[1]->PrintVector(F, &mvec, 100.0);
            }
        }
        /* Second line: Metric M5.4.2 */
        if (ret)
        {
            ret = fprintf(F, ",\n") > 0;
            ret &= MData[3]->GetVector("M5.4.2", NULL, &mvec);
            if (ret) {
                ret = MData[3]->PrintVector(F, &mvec, 1.0);
            }
        }
        /* Next lines: first three names from metric M3. This is interesting,
         * because we need to pick the largest used from reserved and frequent names */
        if (ret) {
            ret = PickThreeNames(F, threeRootNames, MData[1], "M3.3.1", "M3.3.2");
        }

         /* Next lines: first three names from metric M4. This is interesting,
          * because we need to pick the largest used from reserved and frequent names */

        if (ret) {
            ret = PickThreeNames(F, threeStubNames, MData[2], "M4.2", "M4.3");
        }

        /* Then a set of submetrics from M2 */
        char const * M2Metrics[12] = { 
            "M2.2.1.1", "M2.2.2.1", "M2.2.3.1", "M2.2.4.1",
            "M2.1.1.2", "M2.1.2.2", "M2.1.3.2", "M2.1.4.2",
            "M2.1.1.3", "M2.1.2.3", "M2.1.3.3", "M2.1.4.3" };

        for (int i = 0; ret && i < 12; i++) {
            ret = fprintf(F, ",\n") > 0;
            ret &= MData[0]->GetVector(M2Metrics[i], NULL, &mvec);
            if (ret) {
                ret = MData[0]->PrintVector(F, &mvec, 1.0);
            }
        }
        
        /* Closing brackets, this is the end of the data part */
        ret &= (fprintf(F, "],\n") > 0);

        char const * M2Metric_nbGtld = "M2.1.5";
        char const * M2Metric_nbRegistrars = "M2.2.5";
        double d_gtld = 0;
        double d_registrars = 0;

        if (ret && !MData[0]->GetCurrent(M2Metric_nbGtld, NULL, &d_gtld)) {
            d_gtld = NB_GTLD;
        }
        if (ret && !MData[0]->GetCurrent(M2Metric_nbRegistrars, NULL, &d_registrars)) {
            d_registrars = NB_REGISTRARS;
        }

        if (ret) {
            ret &= fprintf(F, "\"NbGtld\" : %d,\n", (int)d_gtld) > 0;
            ret &= fprintf(F, "\"NbRegistrars\" : %d,\n", (int)d_registrars) > 0;
        }

        /* Now, write the 3 names selected in M3, and also the 3 names from M4. */
        if (ret) {
            ret &= fprintf(F, "\"RootNames\" : [") > 0;
            for (int i = 0; i < 3; i++) {
                if (i != 0) {
                    ret &= fprintf(F, ", ") > 0;
                }
                ret &= fprintf(F, "\"%s\"", (threeRootNames[i] == NULL) ? "-" : threeRootNames[i]) > 0;
            }
            ret &= fprintf(F, "],\n") > 0;
        }

        if (ret) {
            ret &= fprintf(F, "\"StubNames\" : [") > 0;
            for (int i = 0; i < 3; i++) {
                if (i != 0) {
                    ret &= fprintf(F, ", ") > 0;
                }
                ret &= fprintf(F, "\"%s\"", (threeRootNames[i] == NULL) ? "-" : threeStubNames[i]) > 0;
            }
            ret &= fprintf(F, "]\n") > 0;
        }

        /* Closing brackets, this is the end of the data part */
        ret &= (fprintf(F, "}\n") > 0);
    }

    /* Close the file */
    if (F != NULL)
    {
        fclose(F);
    }

    return ret;
}

bool ithiIndexPublisher::PickThreeNames(FILE * F, char ** threeNames, ithipublisher * mdata, char const * subMet1, char const * subMet2)
{
    bool ret = true;
    std::vector<MetricNameLine> name_list1;
    std::vector<MetricNameLine> name_list2;

    ret = mdata->GetNameList(subMet1, &name_list1);
    ret &= mdata->GetNameList(subMet2, &name_list2);

    if (ret) {
        int nb_published = 0;
        size_t index_1 = 0;
        size_t index_2 = 0;
        std::vector<double> mvec;

        while (ret && nb_published < 3 && (index_1 < name_list1.size() || index_2 < name_list2.size())){
            double frequency = 0;
            char const * name;

            if (index_1 < name_list1.size()) {
                frequency = name_list1[index_1].current;
            }

            if (index_2 < name_list1.size() && name_list2[index_2].current > frequency) {
                name = name_list2[index_2].name;
                index_2++;
                ret = mdata->GetVector(subMet2, name, &mvec);
            }
            else {
                name = name_list1[index_1].name;
                index_1++;
                ret = mdata->GetVector(subMet1, name, &mvec);
            }

            if (ret && DnsStats::IsValidTldSyntax((uint8_t*) name, strlen(name))) {
                size_t name_len;
                name_len = strlen(name) + 1;
                threeNames[nb_published] = new char[name_len];

                if (threeNames[nb_published] == NULL) {
                    ret = false;
                }
                else {
                    ret = fprintf(F, ",\n") > 0;
                    ret &= mdata->PrintVector(F, &mvec, 100);
                    if (ret) {
                        memcpy(threeNames[nb_published], name, name_len);
                        nb_published++;
                    }
                    else {
                        delete[] threeNames[nb_published];
                        threeNames[nb_published] = NULL;
                    }
                }
            }
        }

        if (ret && nb_published < 3) {
            for (int i = nb_published; i < 3; i++) {
                ret &= fprintf(F, ",[0, 0]\n") > 0;
            }
        }
    }

    return ret;
}

