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

#ifndef CAPTURE_FUZZ_H
#define CAPTURE_FUZZ_H

#include "ithi_test_class.h"

#if __cplusplus < 199711L
#ifndef override
#define override 
#endif
#endif

class DnsStats;

class capture_fuzz : public ithi_test_class
{
public:
    capture_fuzz();
    ~capture_fuzz();
    bool DoTest() override;
    bool LoadPcapFile(char const * fileName);

    bool FuzzPacket(uint8_t * packet, size_t packet_length);

    static uint64_t picoquic_test_random(uint64_t * random_context);

    size_t nb_records_read;
    size_t nb_udp_dns_frag;
    size_t nb_udp_dns;
    size_t fuzz_fail;
    size_t nb_rounds;
    uint64_t fuzz_random_context;
    uint8_t *buffer;
    size_t buffer_size;
    size_t fuzzed_length;
    DnsStats * stats;
};

#endif /* CAPTURE_FUZZ_H */