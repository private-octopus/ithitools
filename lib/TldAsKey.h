#ifndef TLD_AS_KEY_H
#define TLD_AS_KEY_H
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

class TldAsKey
{
public:
    TldAsKey(uint8_t * tld, size_t tld_len);
    ~TldAsKey();

    bool IsSameKey(TldAsKey* key);
    uint32_t Hash();
    TldAsKey* CreateCopy();
    void Add(TldAsKey* key);

    TldAsKey * HashNext;
    TldAsKey * MoreRecentKey;
    TldAsKey * LessRecentKey;

    size_t tld_len;
    uint8_t tld[65];
    uint32_t count;
    uint32_t hash;

    static void CanonicCopy(uint8_t * tldDest, size_t tldDestMax, size_t * tldDestLength,
        uint8_t * tldSrce, size_t tldSrceLength);
    static bool CompareTldEntries(TldAsKey * x, TldAsKey * y);
};

#endif /* TLD_AS_KEY_H */
