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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ithiutil.h"
#include "cbor.h"
#include "cdns.h"

cdns::cdns():
    F(NULL),
    buf(NULL),
    buf_size(0),
    buf_read(0),
    buf_parsed(0),
    end_of_file(false)
{
}

cdns::~cdns()
{
    if (F != NULL) {
        fclose(F);
    }
    
    if (buf != NULL) {
        delete[] buf;
    }
}

bool cdns::open(char const* file_name, size_t buf_size)
{
    bool ret = false;

    if (F == NULL && buf == NULL) {
        if (buf_size == 0) {
            buf_size = 0x10000;
        }

        F = ithi_file_open(file_name, "r");
        if (F != NULL) {
            buf = (uint8_t*)malloc(buf_size);

            if (buf != NULL) {
                ret = load_buffer();
            }
        }
    }

    return ret;
}

bool cdns::load_buffer()
{
    size_t byte_read = 0;

    if (buf_parsed < buf_read) {
        buf_read -= buf_parsed;
        memmove(buf, buf + buf_parsed, buf_read);
    }
    else {
        buf_read = 0;
    }
    buf_parsed = 0;

    if (buf_read < buf_size && !end_of_file) {
        size_t asked = buf_size - buf_read;
        size_t n_bytes = fread(buf + buf_read, 1, asked, F);

        end_of_file = (n_bytes < asked);
        buf_read += n_bytes;
    }

    return (buf_read > 0);
}
