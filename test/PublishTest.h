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

#ifndef METRIC_COLLECTION_TEST_H
#define METRIC_COLLECTION_TEST_H

#include "ithi_test_class.h"

class PublishTest : public ithi_test_class
{
public:
    PublishTest();
    ~PublishTest();

    bool DoTest() override;

    bool DoOneTest(int metric_id, char const ** metric_files, size_t nb_files,
        char const * target_file, char const * ref_file);

    static bool CreateTestDirectory(int metric_id, char const ** file_names, int nb_files);

    static bool CopyFileToDirectory(char const * file_name, char const * dir_name);
};

class PublishIndexTest : public ithi_test_class
{
public:
    PublishIndexTest();
    ~PublishIndexTest();

    bool DoTest() override;
};


#endif /* METRIC_COLLECTION_TEST_H */