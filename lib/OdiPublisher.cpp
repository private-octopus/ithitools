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

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "ithimetrics.h"
#include "OdiPublisher.h"



OdiPublisher::OdiPublisher()
{
}


OdiPublisher::~OdiPublisher()
{
}

/* Publish a specific metric file, for the specified date.
 * If the date is not set, publish for the current data and year.
 * The updated time is always the current time.
 */

bool OdiPublisher::ParseMetricFileName(const char * name, int * metric_id, int * year, int * month, int * day, size_t * name_offset)
{
    size_t ch_index = 0;
    size_t char_after_sep_index = 0;
    size_t name_len = strlen(name);
    int val[4] = { 0, 0, 0, 0 };
    bool ret = true;

    /* Find the last separator in the file name */
    if (ret)
    {
        while (name[ch_index] != 0)
        {
            if (name[ch_index] == ITHI_FILE_PATH_SEP[0])
            {
                char_after_sep_index = ch_index + 1;
            }
            ch_index++;
        }

        /* Check that the name length matches expectation */
        ret &= (char_after_sep_index + 17u) <= name_len;
    }

    if (ret)
    {

        ret = name[char_after_sep_index] == 'M' &&
            name[char_after_sep_index + 2] == '-' &&
            name[char_after_sep_index + 7] == '-' &&
            name[char_after_sep_index + 10] == '-' &&
            name[char_after_sep_index + 13] == '.' &&
            name[char_after_sep_index + 14] == 'c' &&
            name[char_after_sep_index + 15] == 's' &&
            name[char_after_sep_index + 16] == 'v' &&
            name[char_after_sep_index + 17] == 0;
    }

    if (ret)
    {
        char digits[5];
        const int delta[4] = { 1, 3, 8, 11 };
        const int len[4] = { 1, 4, 2, 2 };

        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < len[i]; j++)
            {
                digits[j] = name[char_after_sep_index + delta[i] + j];
                ret &= (isdigit(digits[j]) != 0);
            }
            digits[len[i]] = 0;
            val[i] = atoi(digits);
        }
    }

    /* In case of error, return whatever value was parsed, or possibly zero.
     */
    
    *metric_id = val[0];
    *year = val[1];
    *month = val[2];
    *day = val[3];
    *name_offset = char_after_sep_index;

    return ret;
}