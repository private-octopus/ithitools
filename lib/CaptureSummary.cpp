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

#include <stdio.h>
#include <algorithm>
#include <string.h>
#include "Version.h"
#include "CsvHelper.h"
#include "DnsStats.h"
#include "Version.h"
#include "ithiutil.h"
#include "CaptureSummary.h"

CaptureSummary::CaptureSummary()
    :
    capture_version(ITHITOOLS_VERSION)
{
}

CaptureSummary::~CaptureSummary()
{
    for (size_t i = 0; i < summary.size(); i++)
    {
        if (summary[i] != NULL)
        {
            delete summary[i];
            summary[i] = NULL;
        }
    }
}

bool CaptureSummary::Load(char const * file_name)
{
    CaptureLine line;
    char buffer[512];
    FILE * F = ithi_file_open(file_name, "r");
    bool ret = (F != NULL);

    while ( ret && fgets(buffer, sizeof(buffer), F))
    {
        int start = 0;
        start = CsvHelper::read_string(line.registry_name, sizeof(line.registry_name), start, buffer, sizeof(buffer));
        start = CsvHelper::read_number(&line.key_type, start, buffer, sizeof(buffer));
        if (line.key_type == 0)
        {
            start = CsvHelper::read_number(&line.key_number, start, buffer, sizeof(buffer));
        }
        else
        {
            start = CsvHelper::read_string(line.key_value, sizeof(line.key_value), start, buffer, sizeof(buffer));
        }
        (void)CsvHelper::read_number64(&line.count, start, buffer, sizeof(buffer));


        /* TODO: check that the parsing is good */

        /* Notice the version number */
        if (strcmp(line.registry_name, DnsStats::GetTableName(REGISTRY_ITHITOOLS_VERSION)) == 0) {
            capture_version = (int) line.count;
        } else {
            /* allocate data and add to vector */
            ret = AddLine(&line, true);
        }
    }

    if (F != NULL)
    {
        fclose(F);
    }
    return ret;
}

bool CaptureSummary::Save(char const * file_name)
{
    FILE* F = ithi_file_open(file_name, "w");
    bool ret = (F != NULL);

    if (ret) {
        /* TODO: write the version number */
        fprintf(F, "\"%s\",0,0,%d,\n", DnsStats::GetTableName(REGISTRY_ITHITOOLS_VERSION), capture_version);
    }

    for (size_t i = 0; ret && i < summary.size(); i++)
    {
        CaptureLine * line = summary[i];

        // Print current line
        fprintf(F, """%s"",%d,", line->registry_name, line->key_type);

        if (line->key_type == 0)
        {
            fprintf(F, "%d,", line->key_number);
        }
        else
        {
            fprintf(F, """%s"",", line->key_value);
        }

        fprintf(F, """%lld"",\n", (long long)line->count);
    }

    if (F != NULL)
    {
        fclose(F);
    }

    return ret;
}

bool CaptureSummary::Compare(CaptureSummary * x)
{
    bool ret = x->Size() == Size();

    for (size_t i = 0; ret && i < Size(); i++)
    {
        ret = CaptureLineIsSameKey(summary[i], x->summary[i]);
        if (ret)
        {
            if (summary[i]->count != x->summary[i]->count)
            {
                ret = false;
            }
        }
        else
        {
            break;
        }
    }

    return ret;
}

bool CaptureSummary::AddLine(CaptureLine * line, bool need_allocation)
{
    bool ret = true;
    CaptureLine * new_line;

    if (need_allocation)
    {
        new_line = new CaptureLine;

        if (new_line == NULL)
        {
            ret = false;
        }
        else
        {
            memcpy(new_line, line, sizeof(CaptureLine));
        }
    }
    else
    {
        new_line = line;
    }

    if (ret)
    {
        summary.push_back(new_line);
    }

    return ret;
}

void CaptureSummary::Reserve(size_t target_size)
{
    summary.reserve(target_size);
}

bool CaptureSummary::CaptureLineIsLower(CaptureLine * x, CaptureLine * y)
{
    bool ret = false;
    int cmp;

    cmp = CaptureSummary::compare_string(x->registry_name, y->registry_name);

    if (cmp > 0)
    {
        ret = true;
    }
    else if (cmp == 0)
    {
        if (x->key_type < y->key_type)
        {
            ret = true;
        }
        else if (x->key_type == y->key_type)
        {
            if (x->key_type == 0)
            {
                ret = (x->key_number < y->key_number);
            }
            else
            {
                cmp = compare_string(x->key_value, y->key_value);

                ret = cmp > 0;
            }
        }
    }

    return ret;
}

bool CaptureSummary::CaptureLineIsLargerCount(CaptureLine * x, CaptureLine * y)
{
    bool ret = false;

    if (x->count > y->count)
    {
        ret = true;
    }

    return ret;
}

bool CaptureSummary::CaptureLineIsSameKey(CaptureLine * x, CaptureLine * y)
{
    bool ret = false;
    int cmp;

    cmp = CaptureSummary::compare_string(x->registry_name, y->registry_name);

    if (cmp != 0)
    {
        ret = false;
    }
    else if (x->key_type != y->key_type)
    {
        ret = false;
    }
    else
    {
        if (x->key_type == 0)
        {
            ret = (x->key_number == y->key_number);
        }
        else
        {
            cmp = compare_string(x->key_value, y->key_value);

            ret = cmp == 0;
        }
    }

    return ret;
}


void CaptureSummary::Sort()
{
    std::sort(summary.begin(), summary.end(), CaptureLineIsLower);
}

size_t CaptureSummary::Size()
{
    return summary.size();
}

uint64_t CaptureSummary::GetCountByNumber(char const * table_name, uint32_t number)
{
    uint64_t count = 0;

    for (size_t i = 0; i < summary.size(); i++)
    {
        if (compare_string(table_name, summary[i]->registry_name) == 0 &&
            summary[i]->key_type == 0 &&
            summary[i]->key_number == (int)number)
        {
            count = summary[i]->count;
            break;
        }
    }

    return count;
}

void CaptureSummary::Extract(char const * table_name, std::vector<CaptureLine*>* extract)
{
    for (size_t i = 0; i < summary.size(); i++)
    {
        if (compare_string(table_name, summary[i]->registry_name) == 0)
        {
            extract->push_back(summary[i]);
        }
    }
}

bool CaptureSummary::Merge(char const * list_file_name)
{
    size_t nb_alloc = 0;
    char ** file_list = NULL;
    size_t nb_files = 0;
    FILE* F;
    char buffer[512];
    const size_t batch_size = 128;

    /* Open the list file */
    bool ret;
    F = ithi_file_open(list_file_name, "r");
    ret = (F != NULL);

    /* Read each file name and add it to the list */
    while (ret && fgets(buffer, sizeof(buffer), F) != NULL) {
        /* Clean up the end of the buffer */
        size_t len = strlen(buffer);

        while (len > 0 && (buffer[len-1] == ' ' || buffer[len-1] == '\t' || buffer[len-1] == '\r' || buffer[len-1] == '\n')) {
            buffer[len--] = 0;
        }

        if (len == 0)
        {
            continue;
        }

        /* Allocate memory if necessary */
        if (nb_files >= nb_alloc) {
            size_t new_len = (nb_alloc == 0) ? 512 : 2 * nb_alloc;
            char ** new_list = new char*[new_len];

            if (new_list == NULL) {
                ret = false;
            }
            else
            {
                size_t i = 0;
                while (i < nb_alloc){
                    new_list[i] = file_list[i];
                    file_list[i++] = NULL;
                }
                while (i < new_len) {
                    new_list[i++] = NULL;
                }

                if (file_list != NULL) {
                    delete[] file_list;
                }
                file_list = new_list;
                nb_alloc = new_len;
            }
        }

        /* Record the name */
        if (ret)
        {
            file_list[nb_files] = new char[len + 1];
            if (file_list[nb_files] == NULL)
            {
                ret = false;
            }
            else
            {
                memcpy(file_list[nb_files], buffer, len);
                file_list[nb_files][len] = 0;
                nb_files++;
            }
        }
    }


    /* Merge the list */
    if (ret)
    {
        /* Create intermediate merge files if there are more than 100 summaries */
        if (nb_files > batch_size) {
            size_t nb_compiled = 0;
            size_t nb_intermediate_summaries = (nb_files + batch_size - 1) / batch_size;
            size_t nb_created = 0;
            CaptureSummary** intermediate_list = new  CaptureSummary * [nb_intermediate_summaries];

            if (intermediate_list == NULL) {
                ret = false;
            }
            else {
                for (size_t i = 0; ret && i < nb_intermediate_summaries; i++) {
                    intermediate_list[i] = new CaptureSummary();
                    if (intermediate_list[i] == NULL) {
                        ret = false;
                        for (size_t j = nb_created; j < nb_intermediate_summaries; j++) {
                            intermediate_list[j] = NULL;
                        }
                    }
                    else {
                        nb_created++;
                        size_t first_file = nb_compiled;
                        nb_compiled += batch_size;
                        if (nb_compiled > nb_files) {
                            nb_compiled = nb_files;
                        }
                        ret = intermediate_list[i]->Merge(nb_compiled - first_file, (char const**)(file_list + first_file));
                    }
                }

                if (ret) {
                    /* Merge all the summaries */
                    ret = Merge(nb_intermediate_summaries, intermediate_list);
                }
                /* Clean up all the intermediate summaries */
                for (size_t i = 0; i < nb_intermediate_summaries; i++) {
                    if (intermediate_list[i] != NULL) {
                        delete intermediate_list[i];
                        intermediate_list[i] = NULL;
                    }
                }
                delete[] intermediate_list;
            }
        }
        else {
            ret = Merge(nb_files, (char const**)file_list);
        }
    }

    /* Clean up */
    if (file_list != NULL)
    {
        for (size_t i = 0; i < nb_files; i++)
        {
            delete[] file_list[i];
            file_list[i] = NULL;
        }

        delete[] file_list;
    }

    if (F != NULL)
    {
        fclose(F);
    }

    return ret;
}

bool CaptureSummary::Merge(size_t nb_files, char const ** file_name)
{
    CaptureSummary ** list = new  CaptureSummary*[nb_files + 1];
    CaptureSummary * baselist = new  CaptureSummary[nb_files + 1];

    bool ret = list != NULL && baselist != NULL;

    if (ret)
    {
        for (size_t i = 0; ret && i < nb_files; i++)
        {
            list[i] = &baselist[i];
        }
    }

    for (size_t i=0; ret && i < nb_files; i++)
    {
        ret = list[i]->Load(file_name[i]);
    }

    if (ret)
    {
        ret = Merge(nb_files, list);
    }

    if (baselist != NULL)
    {
        delete[] baselist;
    }

    if (list != NULL)
    {
        delete[] list;
    }

    return ret;
}

bool CaptureSummary::Merge(size_t nb_summaries, CaptureSummary ** cs)
{
    bool ret = true;
    size_t max_size = 0;
    size_t complete_size = 0;
    std::vector<CaptureLine *> complete;
    std::vector<CaptureLine *> leaked_tld;
    std::vector<CaptureLine*> leaked_2ld;
    std::vector<CaptureLine *> frequent_tld;
    char const * leak_tld_id = NULL;
    char const * leak_2ld_id = NULL;
    char const * frequent_tld_id = NULL;
    char const * local_tld_id = NULL;
    char const * leak_by_length_id = NULL;
    char const* name_list_id = NULL;
    char const* addr_list_id = NULL;
    CaptureLine current_line;
    uint64_t nb_locally_leaked_tld = 0;
    CaptureLine *local_leak = new CaptureLine;
    uint64_t skipped_tld_length[64];

    memset(skipped_tld_length, 0, sizeof(skipped_tld_length));

    if (ret)
    {
        leak_tld_id = DnsStats::GetTableName(REGISTRY_DNS_LeakedTLD);
        leak_2ld_id = DnsStats::GetTableName(REGISTRY_DNS_LEAK_2NDLEVEL);
        frequent_tld_id = DnsStats::GetTableName(REGISTRY_DNS_Frequent_TLD_Usage);
        local_tld_id = DnsStats::GetTableName(REGISTRY_DNS_Local_TLD_Usage_Count);
        leak_by_length_id = DnsStats::GetTableName(REGISTRY_DNS_LeakByLength);
        name_list_id = DnsStats::GetTableName(REGISTRY_DNS_ERRONEOUS_NAME_LIST);
        addr_list_id = DnsStats::GetTableName(REGISTRY_DNS_ADDRESS_LIST);

        ret = (leak_tld_id != NULL && frequent_tld_id != NULL && local_tld_id != NULL && leak_by_length_id != NULL && local_leak != NULL
            && name_list_id != NULL && addr_list_id != NULL);
    }

    /* Compute the plausible max size and the complete size */
    for (size_t i = 0; ret && i < nb_summaries; i++)
    {
        if (cs[i]->Size() > max_size)
        {
            max_size = cs[i]->Size();
        }

        complete_size += cs[i]->Size();
    }

    /* Create a complete list that contains all the summaries */
    if (ret)
    {
        complete.reserve(complete_size);
        leaked_tld.reserve(48 * nb_summaries);
        leaked_2ld.reserve(48 * nb_summaries);
        frequent_tld.reserve(16 * nb_summaries);
    }

    for (size_t i = 0; ret && i < nb_summaries; i++) {
        for (size_t j = 0; j < cs[i]->Size(); j++) {
            if (strcmp(cs[i]->summary[j]->registry_name, frequent_tld_id) == 0)
            {
                /* This is a TLD "frequently used" by a client. We will only consider it in the summary
                 * if it is part of the TLD frequently leaked to the root.
                 */  
                if (DnsStats::IsFrequentLeakTld((uint8_t *)cs[i]->summary[j]->key_value,
                    strlen(cs[i]->summary[j]->key_value))) {
                    complete.push_back(cs[i]->summary[j]);
                }
                else
                {
                    nb_locally_leaked_tld += cs[i]->summary[j]->count;
                }
            }
            else if (strcmp(cs[i]->summary[j]->registry_name, leak_tld_id) == 0)
            {
                /* This is a TLD "frequently leaked" to the root */
                if (cs[i]->summary[j]->count <= 4)
                {
                    size_t len = strlen(cs[i]->summary[j]->key_value);

                    if (len < 64)
                    {
                        skipped_tld_length[len]++;
                    }
                }
                else
                {
                    complete.push_back(cs[i]->summary[j]);
                }
            }
            /* TODO: if local leak present, add to total */
            else
            {
                complete.push_back(cs[i]->summary[j]);
            }
        }
    }

    if (ret && nb_locally_leaked_tld != 0)
    {
        local_leak->count = nb_locally_leaked_tld;
        local_leak->key_type = 0;
        local_leak->key_number = 0; 
        
        memcpy(local_leak->registry_name, local_tld_id, strlen(local_tld_id) + 1);

        complete.push_back(local_leak);
    }

    /* Sort the complete list */
    std::sort(complete.begin(), complete.end(), CaptureLineIsLower);

    /* Now go through the list and perform the summations.
     * Put the frequent tld, 2nd ld and tls usage in separate lists */
    Reserve(max_size + 64* nb_summaries);

    for (size_t i = 0; ret && i < complete.size();) {
        /* Read the next capture line */
        current_line = *complete[i++];

        /* Summarize all matching lines */
        while (i < complete.size() && CaptureLineIsSameKey(complete[i], &current_line)) {
            current_line.count += complete[i]->count;
            i++;
        }

        /* Add line to summary, or to one of the special registries */

        if (strcmp(current_line.registry_name, leak_tld_id) == 0) {
            CaptureLine *tld_line = new CaptureLine;

            if (tld_line != NULL)
            {
                memcpy(tld_line, &current_line, sizeof(CaptureLine));
                leaked_tld.push_back(tld_line);
            }
            else
            {
                ret = false;
            }
        }
        else if (strcmp(current_line.registry_name, leak_2ld_id) == 0) {
            CaptureLine* second_ld_line = new CaptureLine;

            if (second_ld_line != NULL)
            {
                memcpy(second_ld_line, &current_line, sizeof(CaptureLine));
                leaked_2ld.push_back(second_ld_line);
            }
            else
            {
                ret = false;
            }
        }
        else if (strcmp(current_line.registry_name, frequent_tld_id) == 0) {
            CaptureLine *tld_line = new CaptureLine;

            if (tld_line != NULL)
            {
                memcpy(tld_line, &current_line, sizeof(CaptureLine));
                frequent_tld.push_back(tld_line);
            }
            else
            {
                ret = false;
            }
        }
        else {
            AddLine(&current_line, true);
        }
    }

    /* Sort and tabulate the most used TLD. */
    if (ret) {
        std::sort(frequent_tld.begin(), frequent_tld.end(), CaptureLineIsLargerCount);
        for (size_t i = 0; i < frequent_tld.size() && i < 256; i++) {
            AddLine(frequent_tld[i], true);
        }
    }

    /* Sort and tabulate the most used 2LD. */
    if (ret) {
        std::sort(leaked_2ld.begin(), leaked_2ld.end(), CaptureLineIsLargerCount);
        for (size_t i = 0; i < leaked_2ld.size() && i < 256; i++) {
            AddLine(leaked_2ld[i], true);
        }
    }

    /* Sort and tabulate the most leaked TLD. 
     * TODO: tabulate the length of the discarded TLD, and correct the captured values */
    if (ret) {
        std::sort(leaked_tld.begin(), leaked_tld.end(), CaptureLineIsLargerCount);
        for (size_t i = 0; i < leaked_tld.size() && i < 256; i++) {
            AddLine(leaked_tld[i], true);
        }

        for (size_t i = 256; i < leaked_tld.size() && i < 256; i++) {
            size_t len = strlen(leaked_tld[i]->key_value);

            if (len < 64)
            {
                skipped_tld_length[len]++;
            }
        }
    }

    if (ret) {
        /* Add the leaked length to the current counts */
        for (size_t i = 0; i < summary.size(); i++)
        {
            if (strcmp(summary[i]->registry_name, leak_by_length_id) == 0 &&
                summary[i]->key_number > 0 &&
                summary[i]->key_number < 64)
            {
                summary[i]->count += skipped_tld_length[summary[i]->key_number];
                skipped_tld_length[summary[i]->key_number] = 0;
            }
        }

        for (int i = 0; i < 64; i++)
        {
            if (skipped_tld_length[i] != 0)
            {
                CaptureLine length_pattern;

                length_pattern.count = skipped_tld_length[i];
                length_pattern.key_type = 0;
                length_pattern.key_number = i;

                memcpy(length_pattern.registry_name, leak_by_length_id, strlen(leak_by_length_id) + 1);

                AddLine(&length_pattern, true);
                skipped_tld_length[i] = 0;
            }
        }
    }

    if (ret) {
        /* Sort the whole thing again */
        Sort();
    }

    /* Clean up */

    for (size_t i = 0; i < frequent_tld.size(); i++)
    {
        if (frequent_tld[i] != NULL)
        {
            delete frequent_tld[i];
            frequent_tld[i] = NULL;
        }
    }

    for (size_t i = 0; i < leaked_tld.size(); i++)
    {
        if (leaked_tld[i] != NULL)
        {
            delete leaked_tld[i];
            leaked_tld[i] = NULL;
        }
    }

    for (size_t i = 0; i < leaked_2ld.size(); i++)
    {
        if (leaked_2ld[i] != NULL)
        {
            delete leaked_2ld[i];
            leaked_2ld[i] = NULL;
        }
    }

    if (local_leak != NULL)
    {
        delete local_leak;
        local_leak = NULL;
    }

    return ret;
}

int CaptureSummary::compare_string(char const * x, char const * y)
{
    int ret = 0;
    uint8_t ux, uy;

    while (*x == *y && *x != 0)
    {
        x++;
        y++;
    }

    ux = (uint8_t)*x;
    uy = (uint8_t)*y;

    if (ux < uy)
    {
        ret = 1;
    }
    else if (ux > uy)
    {
        ret = -1;
    }

    return ret;
}

void CaptureSummary::MultiplyByConstantForTest(unsigned int multiplier)
{
    for (size_t i = 0; i < summary.size(); i++)
    {
        summary[i]->count *= multiplier;
    }
}
