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

#ifndef M2DATA_H
#define M2DATA_H
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include "ComputeMetric.h"

typedef enum M2DataType {
    Unknown = 0,
    TLD,
    Registrar,
    TLD_old
} M2DataType;

typedef enum abuseType {
    Phishing = 0,
    Malware = 1,
    Botnet = 2,
    Spam = 3
} abuseType;

typedef struct st_M2DataLine_t {
    char name[128];
    union {
        int RegistrarId;
        char TldType[64];
    };
    int Domains;
    int AbusiveDomains;
    double AbuseScore;
    int abuse_count[4];
    int NewlyAbusiveDomains;
    int LastMonthAbusiveDomains;
    double LastMonthAbuseScore;
} M2DataLine_t;

class M2Data
{
public:
    M2Data();
    ~M2Data();

    static bool IsSooner(M2Data * x, M2Data * y);
    static bool TldIsSmaller(M2DataLine_t x, M2DataLine_t y);
    static bool IsReservedRegistrarId(int registrar_id);

    bool Load(char const * monthly_csv_file_name);

    void ComputeMetrics(double ithi_m2[4], double ithi_media[4], double ithi_ninety[4]);

    void Sort();

    bool Save();

    char const * get_file_name(char const * monthly_csv_file_path);
    bool parse_file_name(char const * monthly_csv_file_name);
    static char const * get_file_suffix(M2DataType f_type);

    std::vector<M2DataLine_t> dataset;
    int year;
    int month;
    int day;
    M2DataType M2Type;
};


class ComputeM2 : public ComputeMetric
{
public:
    ComputeM2();
    ~ComputeM2();

    bool Load(char const * single_file_name) override;
    bool LoadMultipleFiles(char const ** in_files, int nb_files) override;
    bool LoadTwoFiles(char const * tld_file_name, char const * registrars_file_name);
    bool Compute() override;
    bool Write(FILE * F_out) override;

private:
    M2Data m2Data_tlds;
    M2Data m2Data_registrars;

    bool LoadSingleFile(char const * single_file_name, M2Data * f_data);

    double ithi_m2_tlds[4];
    double ithi_median_tlds[4];
    double ithi_ninety_tlds[4];
    double ithi_m2_registrars[4];
    double ithi_median_registrars[4];
    double ithi_ninety_registrars[4];
};

#endif /* M2DATA_H */