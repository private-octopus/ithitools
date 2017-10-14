#include "targetver.h"
#include "CppUnitTest.h"
#include "hashtest.h"

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

	};
}