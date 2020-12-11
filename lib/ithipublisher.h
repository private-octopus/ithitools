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

#ifndef ITHI_PUBLISHER_H
#define ITHI_PUBLISHER_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

typedef struct _metric_line {
    char metric_name[64];
    char metric_date[64];
    char metric_version[64];
    char key_value[64];
    int year;
    int month;
    double frequency;
} MetricLine;

typedef struct _MetricFileHolder {
    char file_name[256];
    int year;
    int month;
    int day;
} MetricFileHolder;

typedef struct _metric_name_line {
    char const * name;
    double current;
    double average;
} MetricNameLine;

class ithipublisher
{
public:
    ithipublisher(char const * ithi_folder, int metric_id);
    ~ithipublisher();

    bool CollectMetricFiles();

    bool Publish(char const * web_folder);

    static bool ParseFileName(MetricFileHolder * file, const char * name, int metric_id);
    bool LoadFileData(int file_index, char const * dir_met_name);

    char const * ithi_folder;
    int metric_id;
    std::vector<MetricFileHolder*> file_list;
    int last_year;
    int last_month;
    int last_day;
    int first_year;
    int first_month;
    int nb_months;
    std::vector<MetricLine*> line_list;

    static bool MetricFileIsEarlier(MetricFileHolder * f1, MetricFileHolder * f2);
    static bool MetricLineIsLower(MetricLine * l1, MetricLine * l2);
    static bool MetricNameLineIsBigger(MetricNameLine l1, MetricNameLine l2);
    static bool MetricNumberIsLower(MetricNameLine l1, MetricNameLine l2);

    /* Get the nb_months values of a specific metric */
    bool GetVector(char const * metric_name, char const * key_value, std::vector<double> * metric);
    bool GetCurrent(char const * metric_name, char const * key_value, double * current);
    bool GetAverageAndCurrent(char const * metric_name, char const * key_value, double * average, double * current);
    bool GetNameOrNumberList(char const* metric_name, std::vector<MetricNameLine>* name_list, bool is_number);
    bool GetNameList(char const * metric_name, std::vector<MetricNameLine> * name_list);
    bool GetNumberList(char const* metric_name, std::vector<MetricNameLine>* number_list);

    bool PrintVector(FILE * F, std::vector<double> * vx, double mult);
    bool PrintNameOrNumberVectorMetric(FILE* F, char const* sub_met_name, char const* metric_name, double mult, bool is_number);
    bool PrintNameVectorMetric(FILE * F, char const * sub_met_name, char const * metric_name, double mult);
    bool PrintNumberVectorMetric(FILE* F, char const* sub_met_name, char const* metric_name, double mult);
    bool PrintNameList(FILE * F, std::vector<MetricNameLine> * name_list, double mult);

    /* Metric specific publishers */
    bool PublishDataM1(FILE * F);
    bool PublishDataM2(FILE * F);
    bool PublishDataM3(FILE * F);
    bool PublishDataM4(FILE * F);
    bool PublishDataM5(FILE * F);
    bool PublishDataM6(FILE * F);
    bool PublishDataM7(FILE * F);
    bool PublishOptTable(FILE * F, char const * metricId);
    bool PublishDataM8(FILE * F);
};

class ithiIndexPublisher
{
public:
    ithiIndexPublisher(char const * ithi_folder);
    ~ithiIndexPublisher();

    bool CollectMetricFiles();

    bool Publish(char const * web_folder);

    bool PickThreeNames(FILE * F, char ** threeNames, ithipublisher *mdata, char const * subMet1, char const * subMet2);

    char const * ithi_folder;

private:
    ithipublisher * MData[4];
    char * threeRootNames[3];
    char * threeStubNames[3];
};
#endif /* ITHI_PUBLISHER_H */
