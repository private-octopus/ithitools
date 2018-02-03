
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
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
        break;
    case test_list_enum::test_enum_Load:
        test = new LoadTest();
        break;
    case test_list_enum::test_enum_Save:
        test = new SaveTest();
        break;
    case test_list_enum::test_enum_Merge:
        test = new MergeTest();
        break;
    case test_list_enum::test_enum_Capture:
        test = new CaptureTest();
        break;
    case test_list_enum::test_enum_Metric:
        test = new MetricTest();
        break;
    case test_list_enum::test_enum_Pattern:
        test = new PatternTest();
        break;
    case test_list_enum::test_enum_Plugin:
        test = new PluginTest();
        break;
    case test_list_enum::test_enum_Csv:
        test = new CsvTest();
        break;
    case test_list_enum::test_enum_M2Data:
        test = new M2DataTest();
        break;
    default:
        break;
    }

    return test;
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

