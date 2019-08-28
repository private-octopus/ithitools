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
            this->buf_size = 0x10000;
        }
        else {
            this->buf_size = buf_size;
        }

        F = ithi_file_open(file_name, "rb");
        if (F != NULL) {
            buf = new uint8_t[this->buf_size];

            if (buf != NULL) {
                ret = load_buffer();
            }
        }
    }

    return ret;
}

bool cdns::dump(char const* file_out)
{
    FILE * F_out = ithi_file_open(file_out, "w");
    size_t out_size = 10 * buf_size;
    char* out_buf = new char[out_size];
    bool ret = (F_out != NULL && out_buf != NULL);

    if (ret) {
        int err = 0;
        char* p_out = out_buf;
        uint8_t* last = cbor_to_text(buf, buf + buf_read, &p_out, out_buf + out_size, &err);

        fwrite(out_buf, 1, p_out - out_buf, F_out);
        fprintf(F_out, "\nProcessed=%d\nErr = %d\n", (int)(last - buf), err);
    }
    if (F_out != NULL){
        fclose(F_out);
    }
    if (out_buf != NULL) {
        delete[] out_buf;
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
