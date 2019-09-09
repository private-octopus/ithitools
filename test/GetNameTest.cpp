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
#include "ithiutil.h"

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

static uint8_t getNameTest1[] = { 7, 'e', 'x', 'a', 'm', 'p', 'l', 'e', 3, 'c', 'o', 'm', 0 };
static uint8_t getNameTest2[] = { 9, 'e', 'x', 'a', 'm', 'p', 'l', 'e', '-', '2', 3, 'c', 'o', 'm', 0 };
static uint8_t getNameTest3[] = { 9, 'e', 'x', 'a', 'm', 'p', 'l', 'e', '_', '3', 3, 'c', 'o', 'm', 0 };
static uint8_t getNameTest4[] = { 9, 'e', 'x', 'a', 'm', 'p', 'l', 'e', ':', '4', 3, 'c', 'o', 'm', 0 };
static uint8_t getNameTest5[] = { 9, 'e', 'x', 'a', 'm', 'p', 'l', 'e', '.', '5', 3, 'c', 'o', 'm', 0 };
static uint8_t getNameTest6[] = { 9, 'e', 'x', 'a', 'm', 'p', 'l', 'e', 0x7F, '6', 3, 'c', 'o', 'm', 0 };
static uint8_t getNameTest7[] = { 9, 'e', 'x', 'a', 'm', 'p', 'l', 'e', ' ', '7', 3, 'c', 'o', 'm', 0 };
static uint8_t getNameTest8[] = { 10, ' ', 'e', 'x', 'a', 'm', 'p', 'l', 'e', '-', '8', 3, 'c', 'o', 'm', 0 };
static uint8_t getNameTest9[] = { 9, 'e', 'x', 'a', 'm', 'p', 'l', 'e', '-', '9', 3, 0x8c, 0xFF, 0x81, 0 };

static struct st_getNameTestLine {
    uint8_t * dns;
    size_t dns_length;
    char const * expected;
} getNameTestData[] = {
    { getNameTest1, sizeof(getNameTest1), "example.com" },
    { getNameTest2, sizeof(getNameTest2), "example-2.com" },
    { getNameTest3, sizeof(getNameTest3), "example_3.com" },
    { getNameTest4, sizeof(getNameTest4), "example:4.com" },
    { getNameTest5, sizeof(getNameTest5), "example\\0465.com" },
    { getNameTest6, sizeof(getNameTest6), "example\\1276.com" },
    { getNameTest7, sizeof(getNameTest7), "example 7.com" },
    { getNameTest8, sizeof(getNameTest8), "\\032example-8.com" },
    { getNameTest9, sizeof(getNameTest9), "example-9.\\140\\255\\129" }
};

bool GetNameTest::DoTest()
{
    bool ret = true;

    test_out = ithi_file_open(getname_test_debug, "w");
    ret = (test_out != NULL);

    for (size_t i = 0; ret && i < sizeof(getNameTestData) / sizeof(struct st_getNameTestLine); i++) {
        char name_out[1024];
        size_t name_length = 0;
        size_t next = 0;

        next = DnsStats::GetDnsName(getNameTestData[i].dns, (uint32_t)getNameTestData[i].dns_length, 0, (uint8_t *)name_out, sizeof(name_out), &name_length);

        if (next != getNameTestData[i].dns_length) {
            ret = false;
        } else  if (strlen(getNameTestData[i].expected) != name_length) {
            ret = false;
        } else if (memcmp(getNameTestData[i].expected, name_out, name_length) != 0) {
            ret = false;
        }
    }


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
    uint32_t tld_offset = 0;
    uint32_t previous_offset = 0;
    int nb_name_parts = 0;
    bool gotTld = DnsStats::GetTLD(packet, length, 12, &tld_offset, &previous_offset, &nb_name_parts);

    qdcount = (packet[4] << 8) | packet[5];
    ancount = (packet[6] << 8) | packet[7];
    nscount = (packet[8] << 8) | packet[9];
    arcount = (packet[10] << 8) | packet[11];

    if (gotTld) {
        char text[256];
        uint32_t flags;

        if (previous_offset != 0) {
            (void) DnsStats::NormalizeNamePart(packet[previous_offset],
                &packet[previous_offset + 1], (uint8_t *)text, sizeof(text), &flags);
            fprintf(test_out, "%s.", text);
        }
        (void) DnsStats::NormalizeNamePart(packet[tld_offset],
            &packet[tld_offset + 1], (uint8_t *)text, sizeof(text), &flags);
        fprintf(test_out, "%s", text);

        fprintf(test_out, " %d\n", nb_name_parts);
    }
    else {
        fprintf(test_out, "??? %d\n", nb_name_parts);
    }

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
    uint8_t name[1024];
    size_t name_len = 0;
    
    start = stats->GetDnsName(packet, length, start, name, sizeof(name), &name_len);

    if (name_len > 0) {
        DnsStats::SetToUpperCase(name, name_len);
        fprintf(test_out, "%s\n", (char *)name);
    }

    return start;
}

IsIpv4Test::IsIpv4Test()
{
}

IsIpv4Test::~IsIpv4Test()
{
}

static struct st_is_ipv4_test_data {
    char const * name;
    bool expected;
} is_ipv4_test_data[] = {
    { "0.0.0.0", true },
    { "1.2.3.4", true },
    { "11.12.13.14", true },
    { "111.112.113.114", true },
    { "255.255.255.255", true },
    { "example.com.0.0.0.0", true },
    { "example.com.1.2.3.4", true },
    { "example.com.11.12.13.14", true },
    { "example.com.111.112.113.114", true },
    { "example.com.255.255.255.255", true },
    { "a.0.0.0.0", true },
    { "b.1.2.3.4", true },
    { "c.11.12.13.14", true },
    { "example-com.111.112.113.114", true },
    { "example_com.255.255.255.255", true },
    { "0.0.0", false },
    { "0.0.0.-1", false },
    { "1000.2.3.4", false },
    { "13.14", false },
    { "111.-112.113.114", false },
    { "example.com.1000.2.3.4", false },
    { "example.com.13.14", false },
    { "example.com.111.-112.113.114", false },
    { "example-com111.112.113.114", false },
    { "example.com", false }
};

bool IsIpv4Test::DoTest()
{
    size_t nb_test = sizeof(is_ipv4_test_data) / sizeof(struct st_is_ipv4_test_data);
    bool ret = true;

    for (size_t i = 0; i < nb_test; i++) {
        bool actual = DnsStats::IsIpv4Name((uint8_t *)is_ipv4_test_data[i].name, strlen(is_ipv4_test_data[i].name));

        if (actual != is_ipv4_test_data[i].expected) {
            ret = false;
            break;
        }
    }

    return ret;
}
