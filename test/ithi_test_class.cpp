
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

#include "CaptureSummary.h"

#include "hashtest.h"
#include "testRfc6761.h"
#include "LoadTest.h"
#include "SaveTest.h"
#include "MergeTest.h"
#include "CaptureTest.h"
#include "MetricTest.h"
#include "PatternTest.h"
#include "PluginTest.h"
#include "CsvTest.h"
#include "M1DataTest.h"
#include "M2DataTest.h"
#include "ithi_test_class.h"
#include "PublishTest.h"
#include "TldCountTest.h"
#include "DnsPrefixTest.h"
#include "GetNameTest.h"
#include "OdiPublishTest.h"
#include "QNameTest.h"
#include "StatsByIpTest.h"
#include "capture_fuzz.h"
#include "CborTest.h"
#include "CdnsTest.h"

enum test_list_enum {
    test_enum_hash = 0,
    test_enum_Rfc6761,
    test_enum_Load,
    test_enum_Save,
    test_enum_Merge,
    test_enum_Merge_List,
    test_enum_Capture,
    test_enum_CaptureNxCache,
#ifdef PRIVACY_CONSCIOUS
    test_enum_Capture_Addresses,
    test_enum_Capture_Names,
#endif
    test_enum_Metric,
    test_enum_MetricDate,
    test_enum_MetricCaptureFile,
    test_enum_Pattern,
    test_enum_Plugin,
    test_enum_BadPlugin,
    test_enum_Csv,
    test_enum_M1Data,
    test_enum_M2Data,
    test_enum_Publish,
    test_enum_PublishIndex, 
    test_enum_TldCount,
    test_enum_DnsPrefix,
    test_enum_GetName,
    test_enum_Ipv4Tld,
    test_enum_OdiPublish,
    test_enum_CmpName,
    test_enum_QNameMini,
    test_enum_StatsByIp,
    test_enum_CaptureFuzz,
    test_enum_max_number,
    test_enum_cbor,
    test_enum_cbor_skip,
    test_enum_cdns,
    test_enum_cdns_dump,
    test_enum_cdns_capture,

};

ithi_test_class::ithi_test_class()
{
}

ithi_test_class::~ithi_test_class()
{
}

int ithi_test_class::get_number_of_tests()
{
    return test_enum_max_number;
}

char const * ithi_test_class::GetTestName(int number)
{
    switch (number) {
    case test_enum_hash:
        return("hash");
    case test_enum_Rfc6761:
        return("rfc6761");
    case test_enum_Load:
        return("load");
    case test_enum_Save:
        return("save");
    case test_enum_Merge:
        return("merge");
    case test_enum_Merge_List:
        return("merge_list");
    case test_enum_Capture:
        return("capture");
    case test_enum_CaptureNxCache:
        return("CaptureNxCache");
#ifdef PRIVACY_CONSCIOUS
    case test_enum_Capture_Addresses:
        return("CaptureAddresses");
    case test_enum_Capture_Names:
        return("CaptureNames");
#endif
    case test_enum_Metric:
        return("metric");
    case test_enum_MetricDate:
        return("metricDate");
    case test_enum_MetricCaptureFile:
        return("metricCaptureFile");
    case test_enum_Pattern:
        return("pattern");
    case test_enum_Plugin:
        return("plugin");
    case test_enum_BadPlugin:
        return ("badPlugin");
    case test_enum_Csv:
        return("csv");
    case test_enum_M1Data:
        return("m1data");
    case test_enum_M2Data:
        return("m2data");
    case test_enum_Publish:
        return("publish");
    case test_enum_PublishIndex:
        return("publishIndex");
    case test_enum_TldCount:
        return("TldCount");
    case test_enum_DnsPrefix:
        return("DnsPrefix");
    case test_enum_GetName:
        return("GetName");
    case test_enum_Ipv4Tld:
        return("Ipv4Tld");
    case test_enum_OdiPublish:
        return("OdiPublish");
    case test_enum_CmpName:
        return("CmpName");
    case test_enum_QNameMini:
        return("QNameMini");
    case test_enum_StatsByIp:
        return("StatsByIp");
    case test_enum_CaptureFuzz:
        return("CaptureFuzz");
    case test_enum_cbor:
        return("cbor");
    case test_enum_cbor_skip:
        return("cborSkip");
    case test_enum_cdns:
        return("cdns");
    case test_enum_cdns_dump:
        return("cdns_dump");
    case test_enum_cdns_capture:
        return("cdns_capture");
    default:
        break;
    }
    return NULL;
}

int ithi_test_class::GetTestNumberByName(const char * name)
{
    for (int i = 0; i < test_enum_max_number; i++)
    {
#ifdef _WINDOWS
        if (_strcmpi(name, GetTestName(i)) == 0)
#else
        if (strcasecmp(name, GetTestName(i)) == 0)
#endif
        {
            return i;
        }
    }
    return test_enum_max_number;
}

ithi_test_class * ithi_test_class::TestByNumber(int number)
{
    ithi_test_class * test = NULL;

    switch ((test_list_enum)number) {
    case test_enum_hash:
        test = new hashtest();
        break;
    case test_enum_Rfc6761:
        test = new testRfc6761();
        break;
    case test_enum_Load:
        test = new LoadTest();
        break;
    case test_enum_Save:
        test = new SaveTest();
        break;
    case test_enum_Merge:
        test = new MergeTest();
        break;
    case test_enum_Merge_List:
        test = new MergeListTest();
        break;
    case test_enum_Capture:
        test = new CaptureTest();
        break;
    case test_enum_CaptureNxCache:
        test = new CaptureNxCacheTest();
        break;
#ifdef PRIVACY_CONSCIOUS
    case test_enum_Capture_Addresses:
        test = new CaptureAddressesTest();
        break;
    case test_enum_Capture_Names:
        test = new CaptureNamesTest();
        break;
#endif
    case test_enum_Metric:
        test = new MetricTest();
        break;
    case test_enum_MetricDate:
        test = new MetricDateTest();
        break;
    case test_enum_MetricCaptureFile:
        test = new MetricCaptureFileTest();
        break;
    case test_enum_Pattern:
        test = new PatternTest();
        break;
    case test_enum_Plugin:
        test = new PluginTest();
        break;
    case test_enum_BadPlugin:
        test = new PluginTestBad();
        break;
    case test_enum_Csv:
        test = new CsvTest();
        break;
    case test_enum_M1Data:
        test = new M1DataTest();
        break;
    case test_enum_M2Data:
        test = new M2DataTest();
        break;
    case test_enum_Publish:
        test = new PublishTest();
        break;
    case test_enum_PublishIndex:
        test = new PublishIndexTest();
        break;
    case test_enum_TldCount:
        test = new TldCountTest();
        break;
    case test_enum_DnsPrefix:
        test = new DnsPrefixTest();
        break;
    case test_enum_GetName:
        test = new GetNameTest();
        break;
    case test_enum_Ipv4Tld:
        test = new IsIpv4Test();
        break;
    case test_enum_OdiPublish:
        test = new OdiPublishTest();
        break;
    case test_enum_CmpName:
        test = new CmpNameTest();
        break;
    case test_enum_QNameMini:
        test = new QNameTest();
        break;
    case test_enum_StatsByIp:
        test = new StatsByIpTest();
        break;
    case test_enum_CaptureFuzz:
        test = new capture_fuzz();
        break;
    case test_enum_cbor:
        test = new CborTest();
        break;
    case test_enum_cbor_skip:
        test = new CborSkipTest();
        break;
    case test_enum_cdns:
        test = new CdnsTest();
        break;
    case test_enum_cdns_dump:
        test = new CdnsDumpTest();
        break;
    case test_enum_cdns_capture:
        test = new CdnsCaptureTest();
        break;
    default:
        break;
    }

    return test;
}

#if 0
bool ithi_test_class::CaptureLineIsSameKey(struct _capture_line* x, struct _capture_line* y)
{
    bool ret = false;
    int cmp;

    cmp = CaptureSummary::compare_string(x->registry_name, y->registry_name);

    if (cmp != 0)
    {
        ret = false;
    }
    else if (x->key_type != y->key_type)
    {
        ret = false;
    }
    else
    {
        if (x->key_type == 0)
        {
            ret = (x->key_number == y->key_number);
        }
        else
        {
            cmp = compare_string(x->key_value, y->key_value);

            ret = cmp == 0;
        }
    }

    return ret;
}
#endif

bool ithi_test_class::CompareCS(CaptureSummary* x, CaptureSummary * y)
{
    bool ret = true;
    size_t min_size = (x->Size() < y->Size())?x->Size():y->Size();

    for (size_t i = 0; ret && i < min_size; i++)
    {
        if (CaptureSummary::compare_string(x->summary[i]->registry_name, y->summary[i]->registry_name) != 0) {
            TEST_LOG("Summary %d differ, name = %s vs %s\n", i, x->summary[i]->registry_name, y->summary[i]->registry_name);
            ret = false;
        }
        else if (x->summary[i]->key_type != y->summary[i]->key_type)
        {
            TEST_LOG("Summary %d differ, key type = %d vs %d\n", i, x->summary[i]->key_type, y->summary[i]->key_type);
            ret = false;
        }
        else if (x->summary[i]->key_type == 0 && x->summary[i]->key_number != y->summary[i]->key_number) {
            TEST_LOG("Summary %d differ, key = %d vs %d\n", i, x->summary[i]->key_number, y->summary[i]->key_number);
            ret = false;
        }
        else if (x->summary[i]->key_type == 0 && CaptureSummary::compare_string(x->summary[i]->key_value, y->summary[i]->key_value) != 0) {
            TEST_LOG("Summary %d differ, key = %s vs %s\n", i, (char const*)x->summary[i]->key_value, (char const*)y->summary[i]->key_value);
            ret = false;
        }
        else if (x->summary[i]->count != y->summary[i]->count)
        {
            TEST_LOG("Summary %d differ, count = %d vs %d\n", i, x->summary[i], y->summary[i]);
            ret = false;
        }
    }

    if (x->Size() > min_size) {
        TEST_LOG("%d extra keys\n", (int)(x->Size() - min_size));
        ret = false;
    }

    if (y->Size() > min_size) {
        TEST_LOG("%d missing keys\n", (int)(y->Size() - min_size));
        ret = false;
    }

    return ret;
} 


static FILE * F_log = NULL;

void SET_LOG_FILE(FILE * F)
{
    F_log = F;
}

FILE * GET_LOG_FILE()
{
    return F_log;
}

void TEST_LOG(const char * fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (F_log != NULL)
    {
#ifdef _WINDOWS
        vfprintf_s(F_log, fmt, args);
#else
        vfprintf(F_log, fmt, args);
#endif
    }
    va_end(args);
}

