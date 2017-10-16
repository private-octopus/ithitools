/*
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

// PcapCsvMerge.cpp : Defines the entry point for the console application.
//
#include <stdint.h>
#include "PcapCsvReader.h"
#include "PcapCsvMerge.h"

PcapCsvMerge::PcapCsvMerge()
{

}

PcapCsvMerge::~PcapCsvMerge()
{

}

char * PcapCsvMerge::GetFileName(char * x)
{
    char* fname = x;

    while (*x)
    {
        if (*x == '/' || *x == '\\')
        {
            fname = x + 1;
        }
        x++;
    }

    return(fname);
}

bool PcapCsvMerge::DoMerge(int nb_readers, char ** fname, FILE* target)
{
    bool ret = true;
    PcapCsvReader * reader = NULL;
    pcap_csv_line current_line;

    if (nb_readers <= 0)
    {
        fprintf(stderr, "No file to merge");
        ret = false;
    }
    else
    {
        reader = new PcapCsvReader[nb_readers];
        if (reader == NULL)
        {
            fprintf(stderr, "Cannot allocate %d readers.\n", nb_readers);
            ret = false;
        }
    }

    if (ret)
    {
        for (int i = 0; ret && (i < nb_readers); i++)
        {
            ret = reader[i].Open(fname[i]);

            if (!ret)
            {
                fprintf(stderr, "Cannot open reader for <%s>.\n", fname[i]);
            }
        }
    }

    if (ret)
    {
#if 0
        fprintf(target, "R-ID, R-Name, K-Type, Key, Key name,");
        for (int i = 0; i < nb_readers; i++)
        {
            fprintf(target, "F-%d,", i + 1);
        }
        fprintf(target, "\n");

        for (int i = 0; i < nb_readers; i++)
        {
            fprintf(target, "0, Input File, 1, %s, F-%d,", GetFileName(fname[i]), i);
            for (int j = 0; j < nb_readers; j++)
            {
                if (j == i)
                {
                    fprintf(target, "1,");
                }
                else
                {
                    fprintf(target, ",");
                }
            }
            fprintf(target, "\n");
        }
#else
        fprintf(target, "R-ID, R-Name, K-Type, Key, Key name, Count\n");
#endif
    }

    while (ret)
    {
        bool at_least_one = false;

        for (int i = 0; ret && i < nb_readers; i++)
        {
            if (!reader[i].is_finished)
            {
                if (!at_least_one)
                {
                    at_least_one = true;
                    current_line = reader[i].line;
                }
                else
                {
                    if (reader[i].IsLower(&current_line))
                    {
                        current_line = reader[i].line;
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

            for (int i = 0; ret && i < nb_readers; i++)
            {
                if (!reader[i].is_finished &&
                    reader[i].IsEqual(&current_line))
                {
                    total_count += reader[i].line.count;
                    reader[i].ReadNext();
                }
            }

            // Print the header of the current line
            fprintf(target, "%d, ""%s"", ", current_line.registry_id, current_line.registry_name);

            fprintf(target, "%d,", current_line.key_type);
            if (current_line.key_type == 0)
            {
                fprintf(target, "%d,", current_line.key_number);
            }
            else
            {
                fprintf(target, """%s"",", current_line.key_value);
            }

            fprintf(target, """%s"",", current_line.key_name);
#if 0
            // Print the count for each line present
            for (int i = 0; ret && i < nb_readers; i++)
            {
                if (!reader[i].is_finished &&
                    reader[i].IsEqual(&current_line))
                {
                    fprintf(target, "%d,", reader[i].line.count);
                    reader[i].ReadNext();
                }
                else
                {
                    fprintf(target, "0,");
                }
            }
#else
            fprintf(target, """%d"",", total_count);
#endif
            // And finish the csv line...
            fprintf(target, "\n");
        }
    }

    return ret;
}

