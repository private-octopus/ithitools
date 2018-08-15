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

/* Publish a specific metric file.
 * The metric value and name are derived from the file name.
 * The updated time is always the current time.
 */


/* Get the update time. This is used to fill the "modified"
 * date, which should be a full ISO 8601 date:  YYYY-MM-DDThh:mm:ss.sTZD
 *
 * eg
 *
 * "modified" : "2018-05-17T18:23:12.9UTC", 
 *
 */

bool OdiPublisher::GetUpdateTime(char * time_value, size_t time_value_size, time_t current_time)
{
    bool ret = true;
    struct tm tm;

#ifdef _WINDOWS
    if (gmtime_s(&tm, &current_time) != 0)
    {
        ret = false;
    }
#else
    tm = *gmtime(&current_time);
#endif


    if (ret)
    {
        ret = snprintf(time_value, time_value_size, "%04d-%02d-%02dT%02d:%02d:%02dUTC", tm.tm_year + 1900,
            tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec ) > 0;
    }

    return ret;
}