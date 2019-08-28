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

#include "CdnsTest.h"

#ifdef _WINDOWS
#ifndef _WINDOWS64
static char const* cbor_in = "..\\data\\tiny-capture.cbor";
#else
static char const* cbor_in = "..\\..\\data\\tiny-capture.cbor";
#endif
#else
static char const* cbor_in = "data/tiny-capture.cbor";
#endif
static char const* text_out = "tiny-capture-cbor.txt";


CdnsTest::CdnsTest()
{
}

CdnsTest::~CdnsTest()
{
}

bool CdnsTest::DoTest()
{
    cdns cap_cbor;
    bool ret = cap_cbor.open(cbor_in, 700000);

    if (ret) {
        ret = cap_cbor.dump(text_out);
    }

    return ret;
}

