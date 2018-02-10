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
#include "CsvHelper.h"
#include "CaptureSummary.h"

CaptureSummary::CaptureSummary()
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
    FILE* F;
    CaptureLine line;
    char buffer[512];

#ifdef _WINDOWS
    errno_t err = fopen_s(&F, file_name, "r");
    bool ret = (err == 0 && F != NULL);
#else
    bool ret;
    F = fopen(file_name, "r");
    ret = (F != NULL);
#endif

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
        (void)CsvHelper::read_number(&line.count, start, buffer, sizeof(buffer));

        /* TODO: check that the parsing is good */

        /* allocate data and add to vector */
        ret = AddLine(&line, true);
    }

    if (F != NULL)
    {
        fclose(F);
    }
    return ret;
}

bool CaptureSummary::Save(char const * file_name)
{
    FILE* F;

#ifdef _WINDOWS
    errno_t err = fopen_s(&F, file_name, "w");
    bool ret = (err == 0 && F != NULL);
#else
    bool ret;
    F = fopen(file_name, "w");
    ret = (F != NULL);
#endif

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

        fprintf(F, """%d"",\n", line->count);
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

uint32_t CaptureSummary::GetCountByNumber(char const * table_name, uint32_t number)
{
    uint32_t count = 0;

    for (size_t i = 0; i < summary.size(); i++)
    {
        if (compare_string(table_name, summary[i]->registry_name) == 0 &&
            summary[i]->key_type == 0 &&
            summary[i]->key_number == number)
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

    /* Open the list file */
#ifdef _WINDOWS
    errno_t err = fopen_s(&F, list_file_name, "r");
    bool ret = (err == 0 && F != NULL);
#else
    bool ret;
    F = fopen(list_file_name, "r");
    ret = (F != NULL);
#endif

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
        ret = Merge(nb_files, (char const **)file_list);
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
    CaptureSummary ** list = new  CaptureSummary*[nb_files];
    CaptureSummary * baselist = new  CaptureSummary[nb_files];

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
    size_t * indx = new size_t[nb_summaries];
    CaptureLine current_line;

    ret = indx != NULL;

    /* Verify that all inputs are sorted, and compute a reasonable target size */
    for (size_t i = 0; ret && i < nb_summaries; i++)
    {
        cs[i]->Sort();

        if (cs[i]->Size() > max_size)
        {
            max_size = cs[i]->Size();
        }

        indx[i] = 0;
    }

    Reserve(max_size);

    while (ret)
    {
        bool at_least_one = false;

        for (size_t i = 0; ret && i < nb_summaries; i++)
        {
            if (indx[i] < cs[i]->Size())
            {
                if (!at_least_one)
                {
                    at_least_one = true;
                    current_line = *cs[i]->summary[indx[i]];
                }
                else
                {
                    if (CaptureLineIsLower(cs[i]->summary[indx[i]], &current_line))
                    {
                        current_line = *cs[i]->summary[indx[i]];
                    }
                }
            }
        }

        if (!at_least_one)
        {
            break;
        }
        else
        {
            // Compute the total count.
            uint32_t total_count = 0;

            for (size_t i = 0; ret && i < nb_summaries; i++)
            {
                if (indx[i] < cs[i]->Size() &&
                    CaptureLineIsSameKey(cs[i]->summary[indx[i]], &current_line))
                {
                    total_count += cs[i]->summary[indx[i]]->count;
                    indx[i] += 1;
                }
            }
            current_line.count = total_count;

            /* Add the merged line */
            AddLine(&current_line, true);
        }
    }

    if (indx != NULL)
    {
        delete[] indx;
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
