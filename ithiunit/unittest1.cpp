#include "targetver.h"
#include "CppUnitTest.h"
#include "hashtest.h"
#include "testRfc6761.h"
#include "LoadTest.h"
#include "SaveTest.h"
#include "MergeTest.h"
#include "CaptureTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ithiunit
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(Hash)
		{
			// TODO: Your test code here
            hashtest test;
            bool ret = test.DoTest();

            Assert::AreEqual(ret, true);
		}

        TEST_METHOD(Rfc6761)
        {
            // TODO: Your test code here
            testRfc6761 test;
            bool ret = test.DoTest();

            Assert::AreEqual(ret, true);
        }

        TEST_METHOD(Load)
        {
            // TODO: Your test code here
            LoadTest test;
            bool ret = test.DoTest();

            Assert::AreEqual(ret, true);
        }

        TEST_METHOD(Save)
        {
            // TODO: Your test code here
            SaveTest test;
            bool ret = test.DoTest();

            Assert::AreEqual(ret, true);
        }

        TEST_METHOD(Merge)
        {
            // TODO: Your test code here
            MergeTest test;
            bool ret = test.DoTest();

            Assert::AreEqual(ret, true);
        }

        TEST_METHOD(Capture)
        {
            // TODO: Your test code here
            CaptureTest test;
            bool ret = test.DoTest();

            Assert::AreEqual(ret, true);
        }
	};
}