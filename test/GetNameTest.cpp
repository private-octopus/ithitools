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

#include "pcap_reader.h"
#include "DnsStats.h"
#include "MetricTest.h"
#include "GetNameTest.h"

#ifdef _WINDOWS
#ifndef _WINDOWS64
static char const * getname_input_test = "..\\data\\tiny-capture.pcap";
static char const * getname_test_output = "..\\data\\getname-test.txt";
#else
static char const * getname_input_test = "..\\..\\data\\tiny-capture.pcap";
static char const * getname_test_output = "..\\..\\data\\getname-test.txt";
#endif
#else
static char const * getname_input_test = "data/tiny-capture.pcap";
static char const * getname_test_output = "data/getname-test.txt";
#endif
static char const * getname_test_debug = "getname-out.txt";

GetNameTest::GetNameTest() :
    nb_records_max(64),
    test_out(NULL),
    stats(NULL)
{
}


GetNameTest::~GetNameTest()
{
    if (stats != NULL) {
        delete stats;
        stats = NULL;
    }

    if (test_out != NULL) {
        fclose(test_out);
        test_out = NULL;
    }
}

bool GetNameTest::DoTest()
{
    bool ret = true;
#ifdef _WINDOWS
    errno_t err = fopen_s(&test_out, getname_test_debug, "w");
    ret = (err == 0 && test_out != NULL);
#else
    test_out = fopen(getname_test_debug, "w");
    ret = (test_out != NULL);
#endif

    if (ret) {
        stats = new DnsStats();

        if (stats == NULL) {
            ret = false;
        }
    }

    if (ret) {
        ret = LoadPcapFile(getname_input_test);
    }

    if (test_out != NULL) {
        fclose(test_out);
        test_out = NULL;
    }

    if (ret) {
        ret = MetricTest::compare_files(getname_test_debug, getname_test_output);
    }

    return ret;
}

bool GetNameTest::LoadPcapFile(char const * fileName)
{
    bool ret = true;
    pcap_reader reader;
    size_t nb_records_read = 0;
    size_t nb_udp_dns_frag = 0;
    size_t nb_udp_dns = 0;

    if (!reader.Open(fileName, NULL))
    {
        ret = false;
    }
    else
    {
        while (reader.ReadNext() && nb_udp_dns < nb_records_max)
        {
            nb_records_read++;

            if (reader.tp_version == 17 &&
                (reader.tp_port1 == 53 || reader.tp_port2 == 53))
            {
                if (!reader.is_fragment && reader.tp_length - 8 > 12)
                {
                    nb_udp_dns++;

                    fprintf(test_out, "Packet #%d\n", (int)nb_udp_dns);

                    SubmitPacket(reader.buffer + reader.tp_offset + 8,
                        reader.tp_length - 8);

                    fprintf(test_out, "\n");
                }
            }
        }
    }

    return ret;
}

void GetNameTest::SubmitPacket(uint8_t * packet, uint32_t length)
{
    uint32_t qdcount = 0;
    uint32_t ancount = 0;
    uint32_t nscount = 0;
    uint32_t arcount = 0;
    uint32_t parse_index = 12;

    qdcount = (packet[4] << 8) | packet[5];
    ancount = (packet[6] << 8) | packet[7];
    nscount = (packet[8] << 8) | packet[9];
    arcount = (packet[10] << 8) | packet[11];

    for (uint32_t i = 0; i < qdcount; i++)
    {
        if (parse_index >= length)
        {
            break;
        }
        else
        {
            parse_index = SubmitQuery(packet, length, parse_index);
        }
    }

    for (uint32_t i = 0; i < ancount; i++)
    {
        if (parse_index >= length)
        {
            break;
        }
        else
        {
            parse_index = SubmitRecord(packet, length, parse_index);
        }
    }

    for (uint32_t i = 0; i < nscount; i++)
    {
        if (parse_index >= length)
        {
            break;
        }
        else
        {
            parse_index = SubmitRecord(packet, length, parse_index);
        }
    }

    for (uint32_t i = 0; i < arcount; i++)
    {
        if (parse_index >= length)
        {
            break;
        }
        else
        {
            parse_index = SubmitRecord(packet, length, parse_index);
        }
    }
}

int GetNameTest::SubmitQuery(uint8_t * packet, uint32_t length, uint32_t start)
{
    int rrclass = 0;
    int rrtype = 0;
    uint32_t name_start = start;

    start = SubmitName(packet, length, start);

    if (start + 4 <= length)
    {
        start += 4;
    }
    else
    {
        start = length;
    }

    return start;
}

int GetNameTest::SubmitRecord(uint8_t * packet, uint32_t length, uint32_t start)
{
    int ldata = 0;
    int name_start = start;

    /* Labels are only tabulated in responses, to avoid polluting data with erroneous packets */
    start = SubmitName(packet, length, start);

    if ((start + 10) > length)
    {
        start = length;
    }
    else
    {
        ldata = (packet[start + 8] << 8) | packet[start + 9];

        if (start + ldata + 10 > length)
        {
            start = length;
        }
        else
        {
            start += ldata + 10;
        }
    }

    return start;
}

int GetNameTest::SubmitName(uint8_t * packet, uint32_t length, uint32_t start)
{
    uint8_t name[256];
    size_t name_len = 0;
    const char * zone_prefix = NULL;
    
    start = stats->GetDnsName(packet, length, start, name, sizeof(name), &name_len);

    if (name_len > 0) {
        DnsStats::SetToUpperCase(name, name_len);
        fprintf(test_out, "%s\n", (char *)name);
    }

    return start;
}