/*
* Author: Christian Huitema
* Copyright (c) 2018, Private Octopus, Inc.
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

#ifndef GET_NAME_TEST_H
#define GET_NAME_TEST_H

#include "ithi_test_class.h"
 
class DnsStats;

class GetNameTest : public ithi_test_class
{
public:
    GetNameTest();
    ~GetNameTest();

    bool DoTest() override;

    bool LoadPcapFile(char const * fileName);
    void SubmitPacket(uint8_t * packet, uint32_t length);
    int SubmitQuery(uint8_t * packet, uint32_t length, uint32_t start);
    int SubmitRecord(uint8_t * packet, uint32_t length, uint32_t start);
    int SubmitName(uint8_t * packet, uint32_t length, uint32_t start);

    size_t nb_records_max;
    FILE * test_out;
    DnsStats * stats;
};

class IsIpv4Test : public ithi_test_class
{
public:
    IsIpv4Test();
    ~IsIpv4Test();

    bool DoTest() override;
};

#endif /* GET_NAME_TEST_H */