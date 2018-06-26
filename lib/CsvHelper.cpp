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

#include <stdlib.h>

#include "CsvHelper.h"



CsvHelper::CsvHelper()
{
}


CsvHelper::~CsvHelper()
{
}


int CsvHelper::read_number(int * number, size_t start, char const * buffer, size_t buffer_max)
{
    int x = 0;
    char text[64];
    int text_max = sizeof(text);
    int has_comma = 0;
    int last_comma_index = -1;


    /* Skip blanks */
    while (start < buffer_max && (buffer[start] == ' ' || buffer[start] == '\t'))
    {
        start++;
    }

    /* If quoted, remove the quotes */
    if (start < buffer_max && buffer[start] == '"')
    {
        start++;
        while (start < buffer_max && buffer[start] != 0)
        {
            if (buffer[start] == '"')
            {
                /* Skip the quote and break. */
                start++;
                break;
            }
            else if (buffer[start] == ',')
            {
                /* Skip the comma unless there is a syntax error. */
                if (has_comma != 0)
                {
                    if ((x - last_comma_index) != 3)
                    {
                        text[x++] = ',';
                        break;
                    }
                }

                has_comma = 1;
                last_comma_index = x;
                start++;
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
        while (start < buffer_max && buffer[start] != 0 && buffer[start] != ',')
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
    while (start < buffer_max)
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

    return (int)start;
}


int CsvHelper::read_number64(uint64_t * number, size_t start, char const * buffer, size_t buffer_max)
{
    int x = 0;
    char text[64];
    int text_max = sizeof(text);
    int has_comma = 0;
    int last_comma_index = -1;
    char * end_ptr;


    /* Skip blanks */
    while (start < buffer_max && (buffer[start] == ' ' || buffer[start] == '\t'))
    {
        start++;
    }

    /* If quoted, remove the quotes */
    if (start < buffer_max && buffer[start] == '"')
    {
        start++;
        while (start < buffer_max && buffer[start] != 0)
        {
            if (buffer[start] == '"')
            {
                /* Skip the quote and break. */
                start++;
                break;
            }
            else if (buffer[start] == ',')
            {
                /* Skip the comma unless there is a syntax error. */
                if (has_comma != 0)
                {
                    if ((x - last_comma_index) != 3)
                    {
                        text[x++] = ',';
                        break;
                    }
                }

                has_comma = 1;
                last_comma_index = x;
                start++;
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
        while (start < buffer_max && buffer[start] != 0 && buffer[start] != ',')
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

    *number = strtoull(text, &end_ptr, 10);

    /* Skip comma */
    while (start < buffer_max)
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

    return (int)start;
}

int CsvHelper::read_string(char* text, int text_max, size_t start, char const * buffer, size_t buffer_max)
{
    int x = 0;

    /* Skip blanks */
    while (start < buffer_max && (buffer[start] == ' ' || buffer[start] == '\t'))
    {
        start++;
    }

    /* If quoted, copy quoted, else copy */
    if (start < buffer_max && buffer[start] == '"')
    {
#if 0
        if (x < (text_max - 1))
        {
            text[x] = buffer[start];
            x++;
        }
#endif
        start++;
        while (start < buffer_max && buffer[start] != 0)
        {
            if (buffer[start] == '"')
            {
#if 0
                /* Copy the quote and skip it. */
                if (x < (text_max - 1))
                {
                    text[x] = buffer[start];
                    x++;
                }
#endif
                start++;
                /* Test whether there is a double quote */
                if (start < buffer_max && buffer[start] == '"')
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
        while (start < buffer_max && buffer[start] != 0 && buffer[start] != ',')
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

    /* Skip comma */
    while (start < buffer_max)
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

    return (int)start;
}

int CsvHelper::read_double(double * number, size_t start, char const * buffer, size_t buffer_max)
{
    int x = 0;
    char text[64];
    int text_max = sizeof(text);

    /* Skip blanks */
    while (start < buffer_max && (buffer[start] == ' ' || buffer[start] == '\t'))
    {
        start++;
    }

    /* If quoted, remove the quotes */
    if (start < buffer_max && buffer[start] == '"')
    {
#if 0
        if (x < (text_max - 1))
        {
            text[x] = buffer[start];
            x++;
        }
#endif
        start++;
        while (start < buffer_max && buffer[start] != 0)
        {
            if (buffer[start] == '"')
            {
#if 0
                /* Copy the quote and break. */
                if (x < (text_max - 1))
                {
                    text[x] = buffer[start];
                    x++;
                }
#endif
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
        while (start < buffer_max && buffer[start] != 0 && buffer[start] != ',')
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

    *number = atof(text);

    /* Skip comma */
    while (start < buffer_max)
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

    return (int)start;
}