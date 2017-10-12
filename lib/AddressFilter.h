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

#ifndef ADDRESS_FILTER_H
#define ADDRESS_FILTER_H

#include <stdint.h>
#include "HashWriteOnceGeneric.h"

class IPAsKey
{
public:
    IPAsKey(uint8_t * addr, size_t addr_len);
    ~IPAsKey();

    bool IsSameKey(IPAsKey* key);
    uint32_t Hash();
    IPAsKey* CreateCopy();
    void Add(IPAsKey* key);


    size_t addr_len;
    uint8_t addr[16];
    uint32_t count;
    uint32_t hash;
};

class AddressFilter
{
public:
    AddressFilter();
    ~AddressFilter();

    void AddToList(uint8_t * addr, size_t len);

    bool IsInList(uint8_t * addr, size_t len);

    HashWriteOnce<IPAsKey> table;
};

#endif /* ADDRESS_FILTER_H */