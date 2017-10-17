#include "CaptureSummary.h"
#include "MergeTest.h"

static char const * summary1 = "..\\data\\summary1.csv";
static char const * summary2 = "..\\data\\summary2.csv";
static char const * target = "..\\data\\merge-1-2.csv";

MergeTest::MergeTest()
{
}


MergeTest::~MergeTest()
{
}

bool MergeTest::DoTest()
{
    CaptureSummary cs;
    char const * list[2] = { summary1, summary2 };

    bool ret = cs.Merge(2, list);

    if (ret)
    {
        CaptureSummary tcs;

        ret = tcs.Load(target);

        if (ret)
        {
            tcs.Sort();

            ret = cs.Compare(&tcs);
        }
    }

    return ret;
}
