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

#include <stdlib.h>
#include "PcapCsvReader.h"

PcapCsvReader::PcapCsvReader()
    :
    F(NULL),
    is_finished(true)
{
}


PcapCsvReader::~PcapCsvReader()
{
    if (F != NULL)
    {
        fclose(F);
    }
}

bool PcapCsvReader::Open(char * filename)
{
#ifdef _WINDOWS
    errno_t err = fopen_s(&F, filename, "r");
    bool ret = (err == 0);
#else
    bool ret;
    F = fopen(filename, "r");
    ret = (F != NULL);
#endif

    if (ret)
    {
        is_finished = false;
        ReadNext();
    }

    return ret;
}

void PcapCsvReader::ReadNext()
{
    if (is_finished || !fgets(buffer, sizeof(buffer), F))
    {
        is_finished = true;
    }
    else
    {
        int start = 0;
        start = read_number(&line.registry_id, start);
        start = read_string(line.registry_name, sizeof(line.registry_name), start);
        start = read_number(&line.key_type, start);
        if (line.key_type == 0)
        {
            start = read_number(&line.key_number, start);
        }
        else
        {
            start = read_string(line.key_value, sizeof(line.key_value), start);
        }
#if 0
        start = read_string(line.key_name, sizeof(line.key_name), start);
#endif
        (void) read_number(&line.count, start);
    }
}

bool PcapCsvReader::IsLower(pcap_csv_line * y)
{
    bool ret = false;

    if (line.registry_id < y->registry_id)
    {
        ret = true;
    }
    else if (line.registry_id == y->registry_id)
    {
        if (line.key_type < y->key_type)
        {
            ret = true;
        }
        else if (line.key_type == y->key_type)
        {
            if (line.key_type == 0)
            {
                ret = (line.key_number < y->key_number);
            }
            else
            {
                for (int i = 0; i < sizeof(line.key_value); i++)
                {
                    if (line.key_value[i] < y->key_value[i])
                    {
                        ret = true;
                        break;
                    }
                    else if (line.key_value[i] != y->key_value[i])
                    {
                        break;
                    }
                }
            }
        }
    }

    return ret;
}

bool PcapCsvReader::IsEqual(pcap_csv_line * y)
{
    bool ret = true;

    if (line.registry_id != y->registry_id)
    {
        ret = false;
    }
    else if (line.registry_id != y->registry_id)
    {
        ret = false;
    }
    else if (line.key_type != y->key_type)
    {
        ret = false;
    }
    else if (line.key_type == y->key_type)
    {
        if (line.key_type == 0)
        {
            ret = (line.key_number == y->key_number);
        }
        else
        {
            for (int i = 0; i < sizeof(line.key_value); i++)
            {
                if (line.key_value[i] != y->key_value[i])
                {
                    ret = false;
                    break;
                }
                else if (line.key_value[i] == 0)
                {
                    ret = true;
                    break;
                }
            }
        }
    }

    return ret;
}

int PcapCsvReader::read_number(int * number, int start)
{
    int x = 0;
    char text[64];
    int text_max = sizeof(text);


    /* Skip blanks */
    while (start < sizeof(buffer) && (buffer[start] == ' ' || buffer[start] == '\t'))
    {
        start++;
    }

    /* If quoted, remove the quotes */
    if (start < sizeof(buffer) && buffer[start] == '"')
    {
        if (x < (text_max - 1))
        {
            text[x] = buffer[start];
            x++;
        }
        start++;
        while (start < sizeof(buffer) && buffer[start] != 0)
        {
            if (buffer[start] == '"')
            {
                /* Copy the quote and break. */
                if (x < (text_max - 1))
                {
                    text[x] = buffer[start];
                    x++;
                }
                start++;
                break;
            }
            else
            {
                if (x < (text_max - 1))
                {
                    text[x] = buffer[start];
                    x++;
                }
                start++;
            }
        }
    }
    else
    {
        while (start < sizeof(buffer) && buffer[start] != 0 && buffer[start] != ',')
        {
            if (x < (text_max - 1))
            {
                text[x] = buffer[start];
                x++;
            }
            start++;
        }
    }
    text[x] = 0;

    *number = atoi(text);

    /* Skip comma */
    while (start < sizeof(buffer))
    {
        if (buffer[start] == ',')
        {
            start++;
            break;
        }
        else if (buffer[start] == ' ' || buffer[start] == '\t')
        {
            start++;
        }
        else
        {
            break;
        }
    }

    return start;
}

int PcapCsvReader::read_string(char* text, int text_max, int start)
{
    int x = 0;

    /* Skip blanks */
    while (start < sizeof(buffer) && (buffer[start] == ' ' || buffer[start] == '\t'))
    {
        start++;
    }

    /* If quoted, copy quoted, else copy */
    if (start < sizeof(buffer) && buffer[start] == '"')
    {
        if (x < (text_max - 1))
        {
            text[x] = buffer[start];
            x++;
        }
        start++;
        while (start < sizeof(buffer) && buffer[start] != 0)
        {
            if (buffer[start] == '"')
            {
                /* Copy the quote and skip it. */
                if (x < (text_max - 1))
                {
                    text[x] = buffer[start];
                    x++;
                }
                start++;
                /* Test whether there is a double quote */
                if (start < sizeof(buffer) && buffer[start] == '"')
                {
                    /* copy the double quote and continue */
                    if (x < (text_max - 1))
                    {
                        text[x] = buffer[start];
                        x++;
                    }
                    start++;
                }
                else
                {
                    /* This was the final quote */
                    break;
                }
            }
            else
            {
                if (x < (text_max - 1))
                {
                    text[x] = buffer[start];
                    x++;
                }
                start++;
            }
        }
    }
    else
    {
        while (start < sizeof(buffer) && buffer[start] != 0 && buffer[start] != ',')
        {
            if (x < (text_max-1))
            {
                text[x] = buffer[start];
                x++;
            }
            start++;
        }
    }
    text[x] = 0;

    /* Skip comma */
    while (start < sizeof(buffer))
    {
        if (buffer[start] == ',')
        {
            start++;
            break;
        }
        else if (buffer[start] == ' ' || buffer[start] == '\t')
        {
            start++;
        }
        else
        {
            break;
        }
    }

    return start;
}

