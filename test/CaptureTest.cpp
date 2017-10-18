#include "DnsStats.h"
#include "CaptureTest.h"
#include "pcap_reader.h"

static char const * pcap_input_test = "..\\data\\tiny-capture.pcap";
static char const * pcap_test_output = "..\\data\\tiny-capture.csv";

CaptureTest::CaptureTest()
{
}


CaptureTest::~CaptureTest()
{
}

bool CaptureTest::DoTest()
{
    DnsStats capture;
    CaptureSummary cs;
    char const * list[1] = { pcap_input_test };
    bool ret = capture.LoadPcapFiles(1, list);

    if (ret)
    {
        ret = capture.ExportToCaptureSummary(&cs);

        if (ret)
        {
            CaptureSummary tcs;

            ret = tcs.Load(pcap_test_output);

            if (ret)
            {
                cs.Sort();
                tcs.Sort();

                ret = cs.Compare(&tcs);
            }
        }
    }

    return ret;
}
