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

#ifndef M1DATA_H
#define M1DATA_H

#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include "ComputeMetric.h"

typedef struct st_M1DataLine_t {
    int category_index; /* ITHI Category */
    char name[128]; /* registrar_name */
    int RegistrarId; /* iana_id */
    char complaint[128]; /* complaint type */
    int Domains; /* Domain Count */
    int Complaints; /* Total Complaints Received in January 2017... */
    int nb1stNotices; /* Total 1st Notices */
    int nb3rdNotices; /* Total 3rd Notices */
    int nbBreaches; /* Total Breaches */
    int nbSuspensions; /* Total Suspensions */
    int nbTerminations; /* Total Terminations */
    int nbNonRenewals; /* Total Non-Renewals */
} M1DataLine_t;

typedef struct st_M1RegSummary_t {
    int RegistrarId; /* iana_id */
    int Domains; /* Domain Count */
    int nb1stNotices; /* Total 1st Notices */
} M1RegSummary_t;

class M1Data
{
public:
    M1Data();
    ~M1Data();

    bool Load(char const * monthly_compliance_file_name);

    int GetCategoryIndex(char const * category, int lastCategoryIndex);

    bool ParseFileName(char const * monthly_compliance_file_name);

    static bool RegistryIdIsSmaller(M1DataLine_t x, M1DataLine_t y);
    static bool FirstNoticeIsBigger(M1RegSummary_t x, M1RegSummary_t y);

    std::vector<M1DataLine_t> dataset;
    std::vector<char const *> category_indices;
    std::vector<M1RegSummary_t> firstNotice;

    uint64_t totalDomain;
    uint64_t total1stN;
    int nbRegistrars50pc;
    int nbRegistrars90pc;
};

class ComputeM1 : public ComputeMetric
{
public:
    ComputeM1();
    ~ComputeM1();

    bool Load(char const * single_file_name) override;
    bool Compute() override;
    bool Write(FILE * F_out) override;

private:
    M1Data m1Data;

    double ithi_m1[3];
};


#endif /* M1DATA_H */