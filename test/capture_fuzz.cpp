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

#include "DnsStats.h"
#include "CaptureSummary.h"
#include "capture_fuzz.h"
#include "pcap_reader.h"

#ifdef _WINDOWS
#ifndef _WINDOWS64
static char const * pcap_input_test = "..\\data\\tiny-capture.pcap";
#else
static char const * pcap_input_test = "..\\..\\data\\tiny-capture.pcap";
#endif
#else
static char const * pcap_input_test = "data/tiny-capture.pcap";
#endif



capture_fuzz::capture_fuzz() :
    nb_records_read(0),
    nb_udp_dns_frag(0),
    nb_udp_dns(0),
    fuzz_fail(0),
    nb_rounds(5),
    fuzz_random_context(0xDEADBEEFBABACAFEull),
    buffer(NULL),
    buffer_size(0),
    fuzzed_length(0),
    stats(NULL)
{
}

capture_fuzz::~capture_fuzz()
{
    if (buffer != NULL) {
        delete[] buffer;
        buffer = NULL;
    }

    if (stats != NULL) {
        delete stats;
        stats = NULL;
    }
}

bool capture_fuzz::DoTest()
{
    bool ret = true;
    stats = new DnsStats;

    if (stats == NULL) {
        ret = false;
    }
    else {
        for (size_t i = 0; ret && i < nb_rounds; i++) {
            ret = LoadPcapFile(pcap_input_test);
        }
    }

    if (ret) {
        /* Check that the stats can be properly extracted */
        CaptureSummary cs;

        try {
            stats->ExportToCaptureSummary(&cs);
        }
        catch (...) {
            ret = false;
        }
    }

    return ret;
}

bool capture_fuzz::LoadPcapFile(char const * fileName)
{
    bool ret = true;
    pcap_reader reader;

    if (!reader.Open(fileName, NULL))
    {
        ret = false;
    }
    else
    {
        while (reader.ReadNext())
        {
            nb_records_read++;

            if (reader.tp_version == 17 &&
                (reader.tp_port1 == 53 || reader.tp_port2 == 53))
            {
                if (reader.is_fragment)
                {
                    nb_udp_dns_frag++;
                }
                else
                {
                    if (FuzzPacket(reader.buffer + reader.tp_offset + 8,
                        (size_t)reader.tp_length - 8)) {
                        try {
                            my_bpftimeval ts;
                            ts.tv_sec = reader.frame_header.ts_sec;
                            ts.tv_usec = reader.frame_header.ts_usec;
                            stats->SubmitPacket(reader.buffer + reader.tp_offset + 8,
                                reader.tp_length - 8, reader.ip_version, reader.buffer + reader.ip_offset, ts);
                        }
                        catch (...) {
                            /* Fuzz fails if the process throws an exception! */
                            ret = false;
                        }
                        nb_udp_dns++;
                    }
                    else {
                        fuzz_fail++;
                    }
                }
            }
        }
    }

    return ret;
}

bool capture_fuzz::FuzzPacket(uint8_t * packet, size_t packet_length)
{
    bool ret = true;
    uint64_t random_pilot = picoquic_test_random(&fuzz_random_context);

    if (packet_length < 8) {
        return false;
    }

    /* Reallocate the capture buffer */
    if (packet_length > buffer_size)
    {
        size_t new_size = packet_length;
        uint8_t * new_buf = new uint8_t[packet_length];
        if (new_buf == NULL)
        {
            ret = false;
        }
        else
        {
            if (buffer != NULL)
            {
                delete[] buffer;
            }
            buffer = new_buf;
            buffer_size = new_size;
        }
    }

    if (ret) {
        /* smash some random location in the packet */
        size_t x = random_pilot % packet_length;
        size_t y = (size_t)((random_pilot >> 17) & 0x07) + 1;

        memcpy(buffer, packet, packet_length);

        random_pilot = picoquic_test_random(&fuzz_random_context);
        for (size_t i = 0; i < y && x + i < packet_length; i++) {
            packet[i] = (uint8_t)(random_pilot & 0xFF);
            random_pilot >>= 8;
        }

        fuzzed_length = packet_length;
    }

    return ret;
}

/* Pseudo random generation suitable for tests. Guaranties that the
* same seed will produce the same sequence, allows for specific
* random sequence for a given test.
* Adapted from http://xoroshiro.di.unimi.it/splitmix64.c,
* Written in 2015 by Sebastiano Vigna (vigna@acm.org)  */

uint64_t capture_fuzz::picoquic_test_random(uint64_t * random_context)
{
    uint64_t z = (*random_context += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
    return z ^ (z >> 31);
}
