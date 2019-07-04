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

#include <algorithm>
#include <string.h>
#include <stdio.h>
#include "CsvHelper.h"
#include "M2Data.h"

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(x) (void)(x)
#endif

/*
 * The Metrics are in a different order as the columns in the capture file. 
 * The following tables help converting between the two.
 */

static const abuseType M2CaptureOrder[] = {
    Phishing,
    Malware,
    Spam,
    Botnet
};

M2Data::M2Data()
    :
    year(0),
    month(0),
    M2Type(Unknown)
{
}

M2Data::~M2Data()
{
}

bool M2Data::IsSooner(M2Data * x, M2Data * y)
{
    return (x->year < y->year || (x->year == y->year && x->month == y->month));
}

bool M2Data::TldIsSmaller(M2DataLine_t x, M2DataLine_t y)
{
    return (strcmp(x.name, y.name) < 0);
}

/*
 * IANA has reserved a set of registry IDs for special purpose. Data from these registries should
 * not be summed in the total number of abuse domains.
 * We add to this reserved list the registrar #2482, "Stichting Registrar of Last Resort Foundation",
 * which is used to park malware domains while taking down botnets and the like.
 */

static const int m2data_reserved_registrar_id[] = {
    1, 3, 8, 119, 376, 2482, 9995, 9996, 9997, 9998, 9999, 10009, 4000001, 8888888
};

static const size_t nb_m2data_reserved_registrar_id = sizeof(m2data_reserved_registrar_id) / sizeof(int);

bool M2Data::IsReservedRegistrarId(int registrar_id)
{
    bool ret = false;

    for (size_t i = 0; i < nb_m2data_reserved_registrar_id; i++)
    {
        if (registrar_id == m2data_reserved_registrar_id[i])
        {
            ret = true;
            break;
        }
        else if (registrar_id < m2data_reserved_registrar_id[i])
        {
            break;
        }
    }

    return ret;
}

bool M2Data::Load(char const * monthly_csv_file_name)
{
    FILE* F;
    M2DataLine_t line;
    char buffer[512];

    if (M2Type == Unknown) {
        parse_file_name(monthly_csv_file_name);
    }

#ifdef _WINDOWS
    errno_t err = fopen_s(&F, monthly_csv_file_name, "r");
    bool ret = (err == 0 && F != NULL);
#else
    bool ret;
    F = fopen(monthly_csv_file_name, "r");
    ret = (F != NULL);
#endif

    while (ret && fgets(buffer, sizeof(buffer), F))
    {
        int start = 0;
        memset(&line, 0, sizeof(M2DataLine_t));

        if (M2Type == TLD_old)
        {
            start = CsvHelper::read_string(line.name + 1, sizeof(line.name) - 1, start, buffer, sizeof(buffer));
            if (line.name[1] == 0)
            {
                line.name[0] = 0;
            }
            else
            {
                line.name[0] = '.';
            }
        }
        else
        {
            start = CsvHelper::read_string(line.name, sizeof(line.name), start, buffer, sizeof(buffer));
        }
        switch (M2Type)
        {
        case TLD:
            start = CsvHelper::read_string(line.TldType, sizeof(line.TldType), start, buffer, sizeof(buffer));
            break;
        case Registrar:
            start = CsvHelper::read_number(&line.RegistrarId, start, buffer, sizeof(buffer));
            break;
        case TLD_old:
            /* Nothing there */
            break;
        default:
            /* Assume TLD_old if default */
            break;
        }
        start = CsvHelper::read_number(&line.Domains, start, buffer, sizeof(buffer));
        start = CsvHelper::read_number(&line.AbusiveDomains, start, buffer, sizeof(buffer));
        start = CsvHelper::read_double(&line.AbuseScore, start, buffer, sizeof(buffer));

        for (int i = 0; i < 4; i++)
        {
            start = CsvHelper::read_number(&line.abuse_count[i], start, buffer, sizeof(buffer));
        }
        start = CsvHelper::read_number(&line.NewlyAbusiveDomains, start, buffer, sizeof(buffer));
        start = CsvHelper::read_number(&line.LastMonthAbusiveDomains, start, buffer, sizeof(buffer));
        CsvHelper::read_double(&line.LastMonthAbuseScore, start, buffer, sizeof(buffer));

        if (M2Type == Registrar && IsReservedRegistrarId(line.RegistrarId)) {
            /* Ignore data relative to parking registries */
            continue;
        } else if (line.name[0] != 0 && line.Domains != 0) {
            /* allocate data and add to vector */
            dataset.push_back(line);
        }
    }

    if (F != NULL)
    {
        fclose(F);
    }
    return ret;
}

void M2Data::ComputeMetrics(double ithi_m2[4], double ithi_median[4], double ithi_ninety[4], uint32_t * nb_entries)
{
    int totals[4] = { 0,0,0,0 };
    std::vector<double> point_list[4];
    int total_domains = 0;

    *nb_entries = (uint32_t) dataset.size();

    for (int i = 0; i < 4; i++) {
        point_list[i].reserve(dataset.size());
    }

    for (size_t i = 0; i < dataset.size(); i++)
    {
        for (int j = 0; j < 4; j++)
        {
            totals[j] += dataset[i].abuse_count[j];
            /* Push negative value so sort will start from biggest negative to smallest*/
            point_list[j].push_back(-1.0*(double)dataset[i].abuse_count[j]);
        }

        total_domains += dataset[i].Domains;
    }

    for (int j = 0; j < 4; j++)
    {
        abuseType x = M2CaptureOrder[j];

        ithi_m2[x] = (double)totals[j];
        if (total_domains > 0) {
            ithi_m2[x] /= (double)total_domains;
        }
        ithi_m2[x] *= 10000;

        if (totals[j] > 0 && point_list[j].size() > 0) {
            double abuse_median = -0.5 * (double)totals[j];
            double abuse_ninety = -0.9 * (double)totals[j];
            double running_total = 0;
            size_t abuse_rank = 0;
            std::sort(point_list[j].begin(), point_list[j].end());

            while (running_total > abuse_median && abuse_rank < point_list[j].size()) {
                running_total += point_list[j][abuse_rank++];
            }
            ithi_median[x] = (double)abuse_rank;

            while (running_total > abuse_ninety && abuse_rank < point_list[j].size()) {
                running_total += point_list[j][abuse_rank++];
            }
            ithi_ninety[x] = (double)abuse_rank;
        } else {
            ithi_median[x] = 0;
            ithi_ninety[x] = 0;
        }
    }
}

void M2Data::Sort()
{
    std::sort(dataset.begin(), dataset.end(), M2Data::TldIsSmaller);
}

bool M2Data::Save()
{
    char file_name[256];
    FILE* F = NULL;
    bool ret = true;

    if (0 > snprintf(file_name, sizeof(file_name), "M2-%4d-%2d-%2d-%s.csv",
        year, month, day, (M2Type == Registrar) ? "-registrars" : "tlds"))
    {
        ret = false;
    }

    if (ret)
    {
#ifdef _WINDOWS
        errno_t err = fopen_s(&F, file_name, "w");
        ret = (err == 0 && F != NULL);
#else
        F = fopen(file_name, "w");
        ret = (F != NULL);
#endif
    }

    if (!ret)
    {
        printf("Cannot save <%s>\n", file_name);
    }

    for (size_t i = 0; ret && i < dataset.size(); i++)
    {
        fprintf(F, "%s,", (M2Type == Registrar) ? "Registrar" : "TLD");
        fprintf(F, "\"%s\",", dataset[i].name);
        fprintf(F, "%d,", dataset[i].Domains);
        for (int j = 0; j < 4; j++)
        {
            fprintf(F, "%d,", dataset[i].abuse_count[j]);
        }

        fprintf(F, "\n");
    }

    if (F != NULL)
    {
        fclose(F);
    }

    if (ret)
    {
        printf("Saved <%s>, %d lines.\n", file_name, (int)dataset.size() );
    }

    return ret;
}

/*
 * Expect names of the form:
 *      2017-sept-tlds.csv
 *      2017-10-31_registrars.csv
 *      2017-10-30_tlds.csv
 */
static char const * month_names[12] = {
    "jan", "feb", "mar", "apr", "may", "june", "july", "aug", "sept", "oct", "nov", "dec"
};

static const int month_day[12] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static char const * file_suffix[3] = { "_tlds.csv", "_registrars.csv", "-tlds.csv" };
static M2DataType const file_type[3] = { TLD, Registrar, TLD_old };

#ifdef _WINDOWS
static int file_sep = '\\';
#else
static int file_sep = '/';
#endif


char const * M2Data::get_file_name(char const * monthly_csv_file_path)
{
    char const * start = monthly_csv_file_path;
    char const * x = monthly_csv_file_path;

    while (*x)
    {
        if (*x == file_sep)
        {
            start = x + 1;
        }
        x++;
    }

    return start;
}

bool M2Data::parse_file_name(char const * monthly_csv_file_name)
{
    bool ret = true;
    size_t len = strlen(monthly_csv_file_name);
    size_t char_index = 0;
    int y = 0;
    int m = 0;
    int d = 0;
    int suffix_len = 0;

    if (len < 4)
    {
        ret = false;
    }
    else
    {
        for (int i = 0; i < 3; i++)
        {
            size_t sl = strlen(file_suffix[i]);
            if (sl < len && strcmp(monthly_csv_file_name + len - sl, file_suffix[i]) == 0)
            {
                M2Type = file_type[i];
                suffix_len = (int) sl;
                break;
            }
        }

        if (M2Type == Unknown)
        {
            ret = false;
        }

        for (int i = 0; ret && i < 4; i++)
        {
            char c = monthly_csv_file_name[char_index++];

            if (c < '0' || c > '9')
            {
                ret = false;
            }
            else
            {
                y *= 10;
                y += c - '0';
            }
        }

        if (y < 2017 || y > 2057)
        {
            ret = false;
        }
    }

    if (ret && monthly_csv_file_name[char_index++] != '-')
    {
        ret = false;
    }

    if (ret)
    {
        if (M2Type == TLD_old)
        {
            for (int i = 0; i < 12; i++)
            {
                size_t ml = strlen(month_names[i]);
                if ((char_index + ml) < len &&
                    strncmp(&monthly_csv_file_name[char_index], month_names[i], ml) == 0)
                {
                    m = i+1;
                    char_index += ml;
                    break;
                }
            }


            if (m <= 0)
            {
                ret = false;
            }
            else
            {
                d = month_day[m-1];
            }
        }
        else
        {
            for (int i = 0; ret && i < 2; i++)
            {
                char c = monthly_csv_file_name[char_index++];

                if (c < '0' || c > '9')
                {
                    ret = false;
                }
                else
                {
                    m *= 10;
                    m += c - '0';
                }
            }

            if (m <= 0 || m > 12)
            {
                ret = false;
            }

            if (ret && monthly_csv_file_name[char_index++] != '-')
            {
                ret = false;
            }

            for (int i = 0; ret && i < 2; i++)
            {
                char c = monthly_csv_file_name[char_index++];

                if (c < '0' || c > '9')
                {
                    ret = false;
                }
                else
                {
                    d *= 10;
                    d += c - '0';
                }
            }

            if (d <= 0 || d > 31)
            {
                ret = false;
            }
        }
    }

    if (ret && char_index+suffix_len != len)
    {
        ret = false;
    }

    if (ret)
    {
        year = y;
        month = m;
        day = d;
    }

    return ret;
}

char const * M2Data::get_file_suffix(M2DataType f_type)
{
    char const * suffix = NULL;

    switch (f_type) {
    case Registrar:
        suffix = file_suffix[1];
        break;
    case TLD:
        suffix = file_suffix[0];
        break;
    default:
        break;
    }
    return suffix;
}

ComputeM2::ComputeM2() :
    nb_registrars(0),
    nb_gtld(0)
{
    for (int i = 0; i < 4; i++)
    {
        ithi_m2_tlds[i] = 0;
        ithi_median_tlds[i] = 0;
        ithi_ninety_tlds[i] = 0;
        ithi_m2_registrars[i] = 0;
        ithi_median_registrars[i] = 0; 
        ithi_ninety_registrars[i] = 0;
    }
}

ComputeM2::~ComputeM2()
{
}

bool ComputeM2::LoadSingleFile(char const * single_file_name, M2Data * f_data)
{
    bool ret = true;
    char const * file_name = NULL;

    if ((file_name = f_data->get_file_name(single_file_name)) == NULL)
    {
        ret = false;
    }
    else if (!f_data->parse_file_name(file_name))
    {
        ret = false;
    }
    else
    {
        ret = f_data->Load(single_file_name);
    }
    return ret;
}

bool ComputeM2::Load(char const * single_file_name)
{
    UNREFERENCED_PARAMETER(single_file_name);

    return false;
}

bool ComputeM2::LoadMultipleFiles(char const ** in_files, int nb_files)
{
    UNREFERENCED_PARAMETER(in_files);
    UNREFERENCED_PARAMETER(nb_files);

    return false;
}

bool ComputeM2::LoadTwoFiles(char const * tld_file_name, char const * registrars_file_name)
{
    bool ret = m2Data_tlds.Load(tld_file_name);
    ret &= m2Data_registrars.Load(registrars_file_name);

    return ret;
}

bool ComputeM2::Compute()
{
    m2Data_tlds.ComputeMetrics(ithi_m2_tlds, ithi_median_tlds, ithi_ninety_tlds, &nb_gtld);
    m2Data_registrars.ComputeMetrics(ithi_m2_registrars, ithi_median_registrars, ithi_ninety_registrars, &nb_registrars);

    return true;
}

bool ComputeM2::Write(FILE * F_out)
{
    bool ret = true;

    for (int m = 1; m < 3; m++) {
        double * average, *median, *ninety;
        uint32_t count;

        if (m == 1) {
            average = ithi_m2_tlds;
            median = ithi_median_tlds;
            ninety = ithi_ninety_tlds;
            count = nb_gtld;
        } else {
            average = ithi_m2_registrars;
            median = ithi_median_registrars;
            ninety = ithi_ninety_registrars;
            count = nb_registrars;
        }

        for (int i = 0; i < 4; i++)
        {
            ret &= (fprintf(F_out, "M2.%d.%d.1, , %6f,\n", m, i + 1, average[i]) > 0);
            ret &= (fprintf(F_out, "M2.%d.%d.2, , %6f,\n", m, i + 1, median[i]) > 0);
            ret &= (fprintf(F_out, "M2.%d.%d.3, , %6f,\n", m, i + 1, ninety[i]) > 0);
        }
        ret &= (fprintf(F_out, "M2.%d.5, , %d,\n", m, count) > 0);
    }
    return ret;
}

void ComputeM2::SetNbRegistrars(uint32_t nb_registrars)
{
    this->nb_registrars = nb_registrars;
}

void ComputeM2::SetNbGtld(uint32_t nb_gtld)
{
    this->nb_gtld = nb_gtld;
}
