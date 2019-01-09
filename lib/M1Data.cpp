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

#include <algorithm>
#include <string.h>
#include <stdio.h>
#include "CsvHelper.h"
#include "M1Data.h"

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(x) (void)(x)
#endif

M1Data::M1Data()
    :
    totalDomain(0),
    total1stN(0),
    nb_registrars(0)
{
}

M1Data::~M1Data()
{
    for (size_t i = 0; i < category_indices.size(); i++) {
        if (category_indices[i] != NULL) {
            delete[] category_indices[i];
            category_indices[i] = NULL;
        }
    }
}

bool M1Data::Load(char const * monthly_compliance_file_name)
{
    FILE* F;
    M1DataLine_t line;
    M1RegSummary_t reg_summary;
    char buffer[512];
    char category[512];
    char month[64];
    double half_sum = 0;
    double ninety_mark = 0;
    double current_sum = 0;
    size_t current_reg = 0;
    bool skip_month_column = false;

#ifdef _WINDOWS
    errno_t err = fopen_s(&F, monthly_compliance_file_name, "r");
    bool ret = (err == 0 && F != NULL);
#else
    bool ret;
    F = fopen(monthly_compliance_file_name, "r");
    ret = (F != NULL);
#endif


    /*
    MAYBE parse file name for date.
    if (M2Type == Unknown) {
    parse_file_name(monthly_csv_file_name);
    }
    */

    dataset.clear();
    firstNotice.clear();

    while (ret && fgets(buffer, sizeof(buffer), F))
    {
        int start = 0;
        memset(&line, 0, sizeof(M1DataLine_t));
        start = CsvHelper::read_string(category, sizeof(category), start, buffer, sizeof(buffer));
        start = CsvHelper::read_string(line.name, sizeof(line.name), start, buffer, sizeof(buffer));
        start = CsvHelper::read_number(&line.RegistrarId, start, buffer, sizeof(buffer));
        start = CsvHelper::read_string(line.complaint, sizeof(line.complaint), start, buffer, sizeof(buffer));
        if (skip_month_column) {
            start = CsvHelper::read_string(month, sizeof(month), start, buffer, sizeof(buffer));
        }
        start = CsvHelper::read_number(&line.Domains, start, buffer, sizeof(buffer));
        start = CsvHelper::read_number(&line.Complaints, start, buffer, sizeof(buffer));
        start = CsvHelper::read_number(&line.nb1stNotices, start, buffer, sizeof(buffer));
        start = CsvHelper::read_number(&line.nb3rdNotices, start, buffer, sizeof(buffer));
        start = CsvHelper::read_number(&line.nbBreaches, start, buffer, sizeof(buffer));
        start = CsvHelper::read_number(&line.nbSuspensions, start, buffer, sizeof(buffer));
        start = CsvHelper::read_number(&line.nbTerminations, start, buffer, sizeof(buffer));
        start = CsvHelper::read_number(&line.nbNonRenewals, start, buffer, sizeof(buffer));

        if (line.RegistrarId > 0) {
            /* Some files have an extra month column before the domain count */
            if (!skip_month_column && line.Domains == 0) {
                skip_month_column = true;
                line.Domains = line.Complaints;
                line.nb1stNotices = line.nb3rdNotices;
                line.nb3rdNotices = line.nbBreaches;
                line.nbBreaches = line.nbSuspensions;
                line.nbSuspensions = line.nbTerminations;
                line.nbTerminations = line.nbNonRenewals;
                CsvHelper::read_number(&line.nbNonRenewals, start, buffer, sizeof(buffer));
            }
            /* allocate data and add to vector */
            dataset.push_back(line);
        }
    }

    if (F != NULL)
    {
        fclose(F);
    }

    /* Sort by registrar ID */
    std::sort(dataset.begin(), dataset.end(), M1Data::RegistryIdIsSmaller);

    /* Compile the summaries, one line per registry */
    firstNotice.reserve(dataset.size());
    memset(&reg_summary, 0, sizeof(reg_summary));
    for (size_t i = 0; i < dataset.size(); i++) {
        if (i == 0 || dataset[i].RegistrarId != reg_summary.RegistrarId){
            if (i != 0) {
                /* Push the old value */
                firstNotice.push_back(reg_summary);
            }
            /* Push the new values */
            reg_summary.RegistrarId = dataset[i].RegistrarId;
            reg_summary.Domains = dataset[i].Domains;
            reg_summary.nb1stNotices = dataset[i].nb1stNotices;
            totalDomain += dataset[i].Domains;
            total1stN += dataset[i].nb1stNotices;
        } else {
            /* Count the notices */
            reg_summary.nb1stNotices += dataset[i].nb1stNotices;
            total1stN += dataset[i].nb1stNotices;
        }
    }
    if (reg_summary.Domains > 0) {
        /* Don't forget the last line */
        firstNotice.push_back(reg_summary);
    }

    /* Sort by bigger to smaller number of notices */
    std::sort(firstNotice.begin(), firstNotice.end(), M1Data::FirstNoticeIsBigger);

    /* Compute the statiscal summaries */
    half_sum = 0.5*(double)total1stN;
    ninety_mark = 0.9*(double)total1stN;
    current_sum = 0;

    while (current_sum < half_sum && current_reg < firstNotice.size()) {
        current_sum += firstNotice[current_reg++].nb1stNotices;
    }
    nbRegistrars50pc = (int)current_reg;

    while (current_sum < ninety_mark && current_reg < firstNotice.size()) {
        current_sum += firstNotice[current_reg++].nb1stNotices;
    }
    nbRegistrars90pc = (int)current_reg;

    nb_registrars = (uint32_t)firstNotice.size();

    return ret;
}

int M1Data::GetCategoryIndex(char const * category)
{
    int ret = -1;

    for (size_t i = 0; i < category_indices.size(); i++) {
        if (strcmp(category, category_indices[i]) == 0) {
            ret = (int)i;
            break;
        }
    }
    
    if (ret == -1) {
        size_t len = strlen(category);
        char * x = new char[len + 1];
        if (x != NULL) {
            memcpy(x, category, len + 1);
            ret = (int)category_indices.size();
            category_indices.push_back((char const *)x);
        }
    }

    return ret;
}

bool M1Data::ParseFileName(char const * monthly_compliance_file_name)
{
    UNREFERENCED_PARAMETER(monthly_compliance_file_name);

    return false;
}

bool M1Data::RegistryIdIsSmaller(M1DataLine_t x, M1DataLine_t y)
{
    return (x.RegistrarId < y.RegistrarId);
}

bool M1Data::FirstNoticeIsBigger(M1RegSummary_t x, M1RegSummary_t y)
{
    return (x.nb1stNotices > y.nb1stNotices);
}

ComputeM1::ComputeM1() :
    nb_registrars(0)
{
    for (int i = 0; i < 3; i++) {
        ithi_m1[1] = 0;
    }
}

ComputeM1::~ComputeM1()
{
}

bool ComputeM1::Load(char const * single_file_name)
{
    return m1Data.Load(single_file_name);
}

bool ComputeM1::Compute()
{
    bool ret = false;

    if (m1Data.totalDomain > 0) {
        ithi_m1[0] = (1000000*(double)m1Data.total1stN) / ((double)m1Data.totalDomain);
        ithi_m1[1] = (double)m1Data.nbRegistrars50pc;
        ithi_m1[2] = (double)m1Data.nbRegistrars90pc;
        nb_registrars = m1Data.nb_registrars;
        ret = true;
    }
    return ret;
}

bool ComputeM1::Write(FILE * F_out)
{
    bool ret = true;

    ret &= fprintf(F_out, "M1.1, , %8f,\n", ithi_m1[0]) > 0;
    ret &= fprintf(F_out, "M1.2, , %f,\n", ithi_m1[1]) > 0;
    ret &= fprintf(F_out, "M1.3, , %f,\n", ithi_m1[2]) > 0;
    ret &= fprintf(F_out, "M1.4, , %d,\n", nb_registrars) > 0;

    return ret;
}
