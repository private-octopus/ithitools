#include "CaptureSummary.h"
#include "LoadTest.h"

LoadTest::LoadTest()
{
}

LoadTest::~LoadTest()
{
}


static char const * no_such_file = "no_such_file.csv";
static char const * good_file = "..\\data\\summary1.csv";


bool LoadTest::DoTest()
{
    bool ret = DoNoSuchTest();

    if (ret)
    {
        ret = DoGoodTest();
    }

    return ret;
}

bool LoadTest::DoGoodTest()
{
    CaptureSummary cs;

    bool ret = cs.Load(good_file);

    if (ret)
    {
        /* Count the lines in the test file and verify that the count matches. */
        FILE* F = NULL;
        char buffer[512];
        size_t nb_lines = 0;

#ifdef _WINDOWS
        errno_t err = fopen_s(&F, good_file, "r");
        bool ret = (err == 0);
#else
        bool ret;
        F = fopen(file_name, "r");
        ret = (F != NULL);
#endif

        while (ret && fgets(buffer, sizeof(buffer), F))
        {
            nb_lines++;
        }

        if (F != NULL)
        {
            fclose(F);
        }

        if (ret)
        {
            ret = nb_lines = cs.Size();
        }
    }

    return ret;
}

bool LoadTest::DoNoSuchTest()
{
    CaptureSummary cs;

    bool ret = !cs.Load(no_such_file);

    return ret;
}
