/*
* Author: Christian Huitema
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

#include <string.h>
#include "dnscap_common.h"
#include "ithicap.h"
#include "pcap_reader.h"
#include "CaptureSummary.h"
#include "PluginTest.h"
#ifndef _WINDOWS
#include <sys/socket.h>
#endif

#ifdef _WINDOWS
#ifndef _WINDOWS64
static char const * pcap_input_test = "..\\data\\tiny-capture.pcap";
static char const * pcap_test_output = "..\\data\\tiny-capture.csv";
static char const * pcap_test_debug = "plugin-capture-out.csv";
#else
static char const * pcap_input_test = "..\\..\\data\\tiny-capture.pcap";
static char const * pcap_test_output = "..\\..\\data\\tiny-capture.csv";
static char const * pcap_test_debug = "plugin-capture-out.csv";
#endif
#else
static char const * pcap_input_test = "data/tiny-capture.pcap";
static char const * pcap_test_output = "data/tiny-capture.csv";
static char const * pcap_test_debug = "plugin-capture-out.csv";
#endif

PluginTest::PluginTest()
{
}


PluginTest::~PluginTest()
{
}


static void GetSourceAddress(int ip_type, uint8_t * ip_header, iaddr *from)
{
    if (ip_type == 4)
    {
        from->af = AF_INET;
        memcpy(&from->u.a4, ip_header + 12, 4);
    }
    else
    {
        from->af = AF_INET6;
        memcpy(&from->u.a6, ip_header + 8, 16);
    }
}

static void GetDestAddress(int ip_type, uint8_t * ip_header, iaddr *to)
{
    if (ip_type == 4)
    {
        to->af = AF_INET;
        memcpy(&to->u.a4, ip_header + 16, 4);
    }
    else
    {
        to->af = AF_INET6;
        memcpy(&to->u.a6, ip_header + 24, 16);
    }
}

void PluginTest::LoadOpt(int argc, char * argv[])
{
    /* Set the output file parameter */
    libithicap_getopt(&argc, &argv);
}

bool PluginTest::LoadPcapFile(char const * fileName)
{
    bool ret = true;
    pcap_reader reader;
    size_t nb_records_read = 0;
    size_t nb_udp_dns_frag = 0;
    size_t nb_udp_dns = 0;
    my_bpftimeval ts;
    bool is_open = false;

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
                    /* Retrieve the addresses and the time */
                    iaddr from; 
                    iaddr to;

                    GetSourceAddress(reader.ip_version,
                        reader.buffer + reader.ip_offset, &from);
                    GetDestAddress(reader.ip_version,
                        reader.buffer + reader.ip_offset, &to);
                    ts.tv_sec = reader.frame_header.ts_sec;
                    ts.tv_usec = reader.frame_header.ts_usec;

                    /* If not open yet, open it */
                    if (!is_open)
                    {
                        if (libithicap_open(ts) != 0) {
                            ret = false;
                            break;
                        } else {
                            is_open = true;
                        }
                    }

                    /* Submit to the plugin */
                    libithicap_output("plugin-test", from, to, reader.tp_version,
                        DNSCAP_OUTPUT_ISDNS, reader.tp_port1, reader.tp_port2,
                        ts, (const u_char*) reader.buffer, 
                        (const unsigned)reader.frame_header.incl_len,
                        (const u_char*)(reader.buffer + reader.tp_offset + 8), 
                        (const unsigned)reader.tp_length - 8);

                    nb_udp_dns++;
                }
            }
        }

        if (is_open)
        {
            if (libithicap_close(ts) != 0)
            {
                ret = false;
            }
        }
    }

    return ret;
}

bool PluginTest::DoTest()
{
    return DoOneTest(1);
}

bool PluginTest::DoOneTest(int nb_repeat)
{
    bool ret = true;
    int argc = 3;
    char * argv[] = { (char *) "test",
                      (char *) "-o",
                      (char *) pcap_test_debug };

    /* Set the arguments */
    LoadOpt(argc, argv);

    /* Initialize the plug in */
    libithicap_start(NULL);
    
    /* Load the data, which will deal with open and close */
    for (int i = 0; ret && i < nb_repeat; i++) {
        ret = LoadPcapFile(pcap_input_test);
    }

    if (nb_repeat > 1) {
        ret = !ret;
    }

    /* Stop the plugin */
    libithicap_stop();

    if (ret)
    {
        CaptureSummary tcs;
        CaptureSummary cs;

        ret = tcs.Load(pcap_test_output);

        if (ret)
        {
            ret = cs.Load(pcap_test_debug);
        }

        if (ret)
        {
            cs.Sort();
            tcs.Sort();

            ret = ithi_test_class::CompareCS(&cs, &tcs);
        }
    }

    return ret;
}

PluginTestBad::PluginTestBad()
{
}

PluginTestBad::~PluginTestBad()
{
}

bool PluginTestBad::DoTest()
{
    return p_test.DoOneTest(10);
}
