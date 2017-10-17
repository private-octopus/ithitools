#include "CaptureSummary.h"
#include "SaveTest.h"


static char const * good_file = "..\\data\\summary1.csv";
static char const * saved_file = "save_test.csv";

SaveTest::SaveTest()
{
}


SaveTest::~SaveTest()
{
}

bool SaveTest::DoTest()
{
    CaptureSummary cs;

    bool ret = cs.Load(good_file);

    if (ret)
    {
        ret = cs.Save(saved_file);

        if (ret)
        {
            CaptureSummary cs2;

            ret = cs2.Load(saved_file);

            if (ret)
            {
                ret = cs.Compare(&cs2);
            }
        }
    }

    return ret;
}
