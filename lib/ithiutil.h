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

#ifndef ITHIUTIL_H
#define ITHIUTIL_H

#include <stdio.h>
#include <stdint.h>

FILE* ithi_file_open_ex(char const* file_name, char const* flags, int* last_err);

FILE* ithi_file_open(char const* file_name, char const* flags);

FILE* ithi_reopen_stdin(int* last_err);

FILE* ithi_pipe_open(char const* command, bool pipe_write, int* last_err);

FILE* ithi_gzip_compress_open(char const* file_name, int* last_err);

FILE* ithi_xzcat_decompress_open(char const* file_name, int* last_err);

void ithi_pipe_close(FILE* F);

size_t ithi_copy_to_safe_text(char* text, size_t text_max, uint8_t* x_in, size_t l_in);

bool ithi_endswith(char const* target, char const* suffix);

size_t ithi_strip_end_space(char* line);

#endif

