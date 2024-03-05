/*
* Author: Christian Huitema
* Copyright (c) 2023, Private Octopus, Inc.
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

#ifndef HYPERLOGLOGTEST_H
#define HYPERLOGLOGTEST_H

#include "ithi_test_class.h"

class HyperLogLog_test : public ithi_test_class
{
public:
    HyperLogLog_test();
    ~HyperLogLog_test();

    bool DoTest() override;
    bool DoNameTest(size_t nb_names);
    bool DoNumberTest(int nb_number);

private:
    FILE* F;
    uint64_t number_old;
};

class Fnv64_test : public ithi_test_class
{
public:
    Fnv64_test();
    ~Fnv64_test();

    bool DoTest() override;
};

class BucketId_test : public ithi_test_class
{
public:
    BucketId_test();
    ~BucketId_test();

    bool DoTest() override;
};

class LeadingZeroes_test : public ithi_test_class
{
public:
    LeadingZeroes_test();
    ~LeadingZeroes_test();

    bool DoTest() override;
};

#endif /* HYPERLOGLOGTEST_H */
