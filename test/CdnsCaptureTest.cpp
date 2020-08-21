/*
* Author: Christian Huitema
* Copyright (c) 2019, Private Octopus, Inc.
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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cbor.h"
#include "cdns.h"
#include "DnsStats.h"

#include "CdnsCaptureTest.h"

#ifdef _WINDOWS
#ifndef _WINDOWS64
static char const* cbor_in = "..\\data\\tiny-capture.cbor";
static char const* cbor_csv_ref = "..\\data\\tiny-capture-cbor.csv";
#else
static char const* cbor_in = "..\\..\\data\\tiny-capture.cbor";
static char const* cdns_in = "..\\..\\data\\tiny-capture.cdns";
static char const* cbor_csv_ref = "..\\..\\data\\tiny-capture-cbor.csv";
#endif
#else
static char const* cbor_in = "data/tiny-capture.cbor";
static char const* cdns_in = "data/tiny-capture.cbor";
static char const* cbor_csv_ref = "data/tiny-capture-cbor.csv";
#endif
static char const* cbor_csv_out = "tiny-capture-cbor.csv";
static char const* cdns_csv_out = "tiny-capture-cdns.csv";


CdnsCaptureTest::CdnsCaptureTest()
{
}

CdnsCaptureTest::~CdnsCaptureTest()
{
}

bool CdnsCaptureTest::DoTest(char const* test_in, char const* test_out)
{
    DnsStats capture;
    CaptureSummary cs;
    bool ret = capture.LoadCborFile(test_in);

    if (ret)
    {
        ret = capture.ExportToCaptureSummary(&cs);

        if (ret)
        {
            CaptureSummary tcs;

            ret = tcs.Load(cbor_csv_ref);

            if (ret)
            {
                cs.Sort();
                tcs.Sort();

                ret = ithi_test_class::CompareCS(&cs, &tcs);

                if (!ret)
                {
                    cs.Save(test_out);
                }
            }
        }
    }

    return ret;
}

CdnsCaptureTestDraft::CdnsCaptureTestDraft()
{
}

CdnsCaptureTestDraft::~CdnsCaptureTestDraft()
{
}

bool CdnsCaptureTestDraft::DoTest()
{
    CdnsCaptureTest test;
    return test.DoTest(cbor_in, cbor_csv_out);
}

CdnsCaptureTestRfc::CdnsCaptureTestRfc()
{
}

CdnsCaptureTestRfc::~CdnsCaptureTestRfc()
{
}

bool CdnsCaptureTestRfc::DoTest()
{
    CdnsCaptureTest test;
    return test.DoTest(cdns_in, cdns_csv_out);
}
