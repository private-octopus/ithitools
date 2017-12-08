/*
* Author: Christian Huitema
* Copyright (c) 2017, Private Octopus, Inc.
* All rights reserved.
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Private Octopus, Inc. BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "targetver.h"
#include "CppUnitTest.h"
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

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ithiunit
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(Hash)
		{
            hashtest test;
            bool ret = test.DoTest();

            Assert::AreEqual(ret, true);
		}

        TEST_METHOD(Rfc6761)
        {
            testRfc6761 test;
            bool ret = test.DoTest();

            Assert::AreEqual(ret, true);
        }

        TEST_METHOD(Load)
        {
            LoadTest test;
            bool ret = test.DoTest();

            Assert::AreEqual(ret, true);
        }

        TEST_METHOD(Save)
        {
            SaveTest test;
            bool ret = test.DoTest();

            Assert::AreEqual(ret, true);
        }

        TEST_METHOD(Merge)
        {
            MergeTest test;
            bool ret = test.DoTest();

            Assert::AreEqual(ret, true);
        }

        TEST_METHOD(Capture)
        {
            CaptureTest test;
            bool ret = test.DoTest();

            Assert::AreEqual(ret, true);
        }

        TEST_METHOD(Metric)
        {
            MetricTest test;
            bool ret = test.DoTest();

            Assert::AreEqual(ret, true);
        }

        TEST_METHOD(Pattern)
        {
            // TODO: Your test code here
            PatternTest test;
            bool ret = test.DoTest();

            Assert::AreEqual(ret, true);
        }

        TEST_METHOD(Plugin)
        {
            // TODO: Your test code here
            PluginTest test;
            bool ret = test.DoTest();

            Assert::AreEqual(ret, true);
        }

        TEST_METHOD(CsvHelper)
        {
            // TODO: Your test code here
            CsvTest test;
            bool ret = test.DoTest();

            Assert::AreEqual(ret, true);
        }
	};
}