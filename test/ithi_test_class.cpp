#include <string.h>

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
#include "M2DataTest.h"
#include "ithi_test_class.h"

enum test_list_enum {
    test_enum_hash = 0,
    test_enum_Rfc6761,
    test_enum_Load,
    test_enum_Save,
    test_enum_Merge,
    test_enum_Capture,
    test_enum_Metric,
    test_enum_Pattern,
    test_enum_Plugin,
    test_enum_Csv,
    test_enum_M2Data,
    test_enum_max_number
};

ithi_test_class::ithi_test_class()
{
}

ithi_test_class::~ithi_test_class()
{
}

int ithi_test_class::get_number_of_tests()
{
    return test_list_enum::test_enum_max_number;
}

char const * ithi_test_class::GetTestName(int number)
{
    switch (number) {
    case test_list_enum::test_enum_hash:
        return("hash");
    case test_list_enum::test_enum_Rfc6761:
        return("rfc6761");
    case test_list_enum::test_enum_Load:
        return("load");
    case test_list_enum::test_enum_Save:
        return("save");
    case test_list_enum::test_enum_Merge:
        return("merge");
    case test_list_enum::test_enum_Capture:
        return("capture");
    case test_list_enum::test_enum_Metric:
        return("metric");
    case test_list_enum::test_enum_Pattern:
        return("pattern");
    case test_list_enum::test_enum_Plugin:
        return("plugin");
    case test_list_enum::test_enum_Csv:
        return("csv");
    case test_list_enum::test_enum_M2Data:
        return("m2data");
    default:
        break;
    }
    return nullptr;
}

int ithi_test_class::GetTestNumberByName(const char * name)
{
    for (int i = 0; i < test_list_enum::test_enum_max_number; i++)
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
    return test_list_enum::test_enum_max_number;
}

ithi_test_class * ithi_test_class::TestByNumber(int number)
{
    ithi_test_class * test = NULL;

    switch (number) {
    case test_list_enum::test_enum_hash:
        test = new hashtest();
        break;
    case test_list_enum::test_enum_Rfc6761:
        test = new testRfc6761();
    case test_list_enum::test_enum_Load:
        test = new LoadTest();
    case test_list_enum::test_enum_Save:
        test = new SaveTest();
    case test_list_enum::test_enum_Merge:
        test = new MergeTest();
    case test_list_enum::test_enum_Capture:
        test = new CaptureTest();
    case test_list_enum::test_enum_Metric:
        test = new MetricTest();
    case test_list_enum::test_enum_Pattern:
        test = new PatternTest();
    case test_list_enum::test_enum_Plugin:
        test = new PluginTest();
    case test_list_enum::test_enum_Csv:
        test = new CsvTest();
    case test_list_enum::test_enum_M2Data:
        test = new M2DataTest();
    default:
        break;
    }

    return test;
}


