/*
* Author: Christian Huitema
* Copyright (c) 2019, Private Octopus, Inc.
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

/* Simple set of utilities */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef _WINDOWS
#include <fcntl.h>
#include <io.h>
#else
#include <errno.h>
#endif
#include "ithiutil.h"

/* Safely open files in a portable way */
FILE* ithi_file_open_ex(char const* file_name, char const* flags, int* last_err)
{
    FILE* F;

#ifdef _WINDOWS
    errno_t err = fopen_s(&F, file_name, flags);
    if (err != 0) {
        if (last_err != NULL) {
            *last_err = err;
        }
        if (F != NULL) {
            fclose(F);
            F = NULL;
        }
    }
#else
    F = fopen(file_name, flags);
    if (F == NULL && last_err != NULL) {
        *last_err = errno;
    }
#endif

    return F;
}

FILE* ithi_file_open(char const* file_name, char const* flags)
{
    return ithi_file_open_ex(file_name, flags, NULL);
}

FILE* ithi_reopen_stdin(int* last_err)
{
    FILE* F = NULL;
#ifdef _WINDOWS
    F = stdin;
    if (_setmode(_fileno(F), _O_BINARY) == -1) {
        *last_err = -1;
        F = NULL;
    }
    else {
        *last_err = 0;
    }
#else
    F = freopen(NULL, "rb", stdin);
    *last_err = (F == NULL) ? -1 : 0;
#endif
    return F;
}

FILE* ithi_pipe_open(char const* command, bool pipe_write, int* last_err)
{
    FILE* F;

#ifdef _WINDOWS
    F = _popen(command, (pipe_write)?"wb":"rb");
    if (F == NULL) {
        *last_err = -1;
    }
#else
    F = popen(command, (pipe_write) ? "w" : "r");
    if (F == NULL && last_err != NULL) {
        *last_err = errno;
    }
#endif

    return F;
}

FILE* ithi_gzip_compress_open(char const* file_name, int* last_err)
{
    FILE * F = NULL;
    char const* gzip_command = NULL;
    char command[512];
    int n_char = 0;

#ifdef _WINDOWS
    /* Running on windows requires that 7z.exe is installed */
    gzip_command = "7z.exe -si -so -an -tgzip a";
#else
    gzip_command = "gzip";
#endif

#ifdef _WINDOWS
    n_char = sprintf_s(command, sizeof(command), "%s >%s.gz", gzip_command, file_name);
#else 
    n_char = sprintf(command, "%s >%s.gz", gzip_command, file_name);
#endif
    if (n_char <= 0) {
        *last_err = -1;
    }
    else {
        F = ithi_pipe_open(command, true, last_err);
    }

    return F;
}

FILE* ithi_xzcat_decompress_open(char const* file_name, int* last_err)
{
    FILE * F = NULL;
    char const* xzcat_command = NULL;
    char command[512];
    int n_char = 0;

#ifdef _WINDOWS
    /* Running on windows requires that 7z.exe is installed */
    xzcat_command = "7z.exe e -so";
#else
    xzcat_command = "xzcat -k";
#endif

#ifdef _WINDOWS
    n_char = sprintf_s(command, sizeof(command), "%s %s", xzcat_command, file_name);
#else 
    n_char = sprintf(command, "%s %s", xzcat_command, file_name);
#endif
    if (n_char <= 0) {
        *last_err = -1;
    }
    else {
        F = ithi_pipe_open(command, false, last_err);
    }

    return F;
}

void ithi_pipe_close(FILE* F)
{
#ifdef _WINDOWS
    (void)_pclose(F);
#else
    (void)pclose(F);
#endif
}

size_t ithi_copy_to_safe_text(char* text, size_t text_max, uint8_t * x_in, size_t l_in)
{
    size_t text_length = 0;
    bool previous_was_space = true; /* Cannot have space at beginning */

    /* escape any non printable character */
    for (uint32_t i = 0; i < l_in && text_length + 1 < text_max; i++)
    {
        int x = x_in[i];
        bool should_escape = false;

        if (x > ' ' && x < 127 && x != '"' && x != ',' && x != '"' && x != '\''
            && (x != '=' || i > 0))
        {
            previous_was_space = false;
        }
        else if (x == ' ' && !previous_was_space && i != l_in - 1)
        {
            /* Cannot have several spaces */
            previous_was_space = true;
        }
        else
        {
            should_escape = true;
        }

        if (should_escape) {
            if (text_length + 5 < text_max) {
                text[text_length++] = '\\';
                text[text_length++] = '0' + (x / 100);
                text[text_length++] = '0' + (x % 100) / 10;
                text[text_length++] = '0' + x % 10;
            }
            else {
                text[text_length++] = '!';
            }
        }
        else {
            text[text_length++] = (char)x;
        }
    }

    if (text_length < text_max) {
        text[text_length] = 0;
    }

    return (text_length);
}

bool ithi_endswith(char const* target, char const* suffix)
{
    size_t target_length = strlen(target);
    size_t suffix_length = strlen(suffix);
    bool ret = suffix_length <= target_length;
    if (ret) {
        size_t ix = target_length - suffix_length;
        ret &= (strcmp(target + ix, suffix) == 0);
    }
    return ret;
}