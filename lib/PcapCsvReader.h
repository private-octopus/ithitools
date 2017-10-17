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

#ifndef PCAPCSVREADER_H
#define PCAPCSVREADER_H

#include <stdio.h>

typedef struct _pcap_csv_line
{
    char registry_name[64];
    int key_type;
    union
    {
        int key_number;
        char key_value[64];
    };
    int count;
} pcap_csv_line;


class PcapCsvReader
{
public:
    PcapCsvReader();
    ~PcapCsvReader();

    bool Open(char * filekey_name);

    void ReadNext();

    bool IsLower(pcap_csv_line * low_line);

    bool IsEqual(pcap_csv_line * low_line);

    pcap_csv_line line;
    FILE * F;
    bool is_finished;
    char buffer[512];

private:
    int read_number(int* number, int start);
    int read_string(char* text, int text_max, int start);
    int compare_string(char * x, char * y);
};

#endif