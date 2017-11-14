
#include <string.h>
#include "dnscap_common.h"
#include "ithicap.h"
#include "pcap_reader.h"
#include "CaptureSummary.h"
#include "PluginTest.h"

#ifdef _WINDOWS
static char const * pcap_input_test = "..\\data\\tiny-capture.pcap";
static char const * pcap_test_output = "..\\data\\tiny-capture.csv";
static char const * pcap_test_debug = "plugin-capture-out.csv";
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
                        libithicap_open(ts);
                        is_open = true;
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
            libithicap_close(ts);
        }
    }

    return ret;
}

bool PluginTest::DoTest()
{
    bool ret = true;
    int argc = 3;
    char * argv[] = { (char *) "test",
                      (char *) "-o",
                      (char *) pcap_test_debug };

    my_bpftimeval ts = { 0, 0 };

    /* Set the arguments */
    LoadOpt(argc, argv);

    /* Initialize the plug in */
    libithicap_start(NULL);
    
    /* Load the data, which will deal with open and close */
    ret = LoadPcapFile(pcap_input_test);

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

            ret = cs.Compare(&tcs);
        }
    }

    return ret;
}
