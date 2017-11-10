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

#ifndef NAME_PATTERN_H
#define NAME_PATTERN_H

#include <stdint.h>

#define PATTERN_CHAR_INDEX_MAX 37

#define PATTERN_CHAR_NUM_MAX 10
#define PATTERN_CHAR_HEXA_MAX 16
#define PATTERN_CHAR_ALPHA_MAX 36
#define PATTERN_CHAR_ALPHA_MIN 10

enum NamePatternChars {
    name_pattern_alphanumdash = 0,
    name_pattern_alphanum = 1,
    name_pattern_alpha = 2,
    name_pattern_hexa = 4,
    name_pattern_num = 8,
    name_pattern_idn = 16
};

struct NamePatternDef {
    char const * pattern_name;
    NamePatternChars char_pattern;
    bool is_random;
    size_t length_min;
    size_t length_max;
};

extern NamePatternDef name_pattern_list[];
extern size_t name_pattern_list_nb;

class NamePattern
{
public:
    NamePattern();
    ~NamePattern();

    static bool Preprocess(uint8_t * str, size_t len, uint8_t * val, uint32_t * char_pattern);

    static bool CheckAlphaRandom(uint8_t * val, size_t len);

    static char const * GetPattern(uint8_t * val, size_t len);

};

#endif /* NAME_PATTERN_H */

