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

#include <string.h>
#include "testRfc6761.h"
#include "DnsStats.h"

static char const * to_succeed[] =
{
    "example",
    "INVALID",
    "local",
    "LOCALHOST",
    "Onion",
    "TEST"
};

static const size_t size_to_succeed = sizeof(to_succeed) / sizeof(char const *);

static char const * to_fail[] =
{
    "ag",
    "agakhan",
    "bofa",
    "bom",
    "xn--1qqw23a",
    "xn--30rr7y"
};

static const size_t size_to_fail = sizeof(to_fail) / sizeof(char const *);

testRfc6761::testRfc6761()
{
}


testRfc6761::~testRfc6761()
{
}

bool testRfc6761::DoTest()
{
    bool ret = true;

    for (size_t i = 0; ret && i < size_to_succeed; i++)
    {
        ret = DnsStats::IsRfc6761Tld((uint8_t *)to_succeed[i], strlen(to_succeed[i]));
    }


    for (size_t i = 0; ret && i < size_to_fail; i++)
    {
        ret = !DnsStats::IsRfc6761Tld((uint8_t *)to_fail[i], strlen(to_fail[i]));
    }

    return ret;
}
