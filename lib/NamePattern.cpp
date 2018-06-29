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

#include <stdlib.h> 
#include <string.h>
#include <math.h>
#include "NamePattern.h"

/*
 * The characters are indexed as follow:
 *  - Digits 0 to 9: indices 0 to 9
 *  - Letters A to Z: indices 10 to 35
 *  - Hyphen: index 36.
 * 
 * We use a set of metrics for separating alpha strings and DGA. These
 * are driven by frequency tables, computed on a set of 1000 domain names, from
 * the registered TLD list. Long explanation of that belongs in the documentation.
 */

static double char_proba[26] = {
    0.076492537, 0.067164179, 0.11380597, 0.041044776, 0.035447761, 0.055970149,
    0.03358209, 0.044776119, 0.026119403, 0.01119403, 0.018656716, 0.048507463,
    0.054104478, 0.026119403, 0.024253731, 0.044776119, 0.001865672, 0.041044776,
    0.10261194, 0.046641791, 0.005597015, 0.029850746, 0.029850746, 0.005597015,
    0.01119403, 0.003731343
};

static uint8_t char_group[26] = {
    0, 3, 2, 2, 0, 3, 2, 2, 1, 4, 3, 2, 2, 1, 1, 2, 4, 1, 1, 1, 2, 3, 3, 4, 3, 4
};

static int char_group_weight[5] = {
    2, 6, 8, 6, 4
};

static double char_group_proba[5] = {
    0.110244689, 0.478354396, 0.294433988, 0.103791342, 0.013175585
};

static uint8_t transition_group[26] = {
    0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1
};

static double transition_group_weight[4] = {
    4.0*4.0 / (26.0*26.0), 4.0*22.0 / (26.0*26.0), 22.0*4.0 / (26.0*26.0), 22.0*22.0 / (26.0*26.0)
};

static double transition_group_proba[4] = {
    0.036855037, 0.314847315, 0.341172341, 0.307125307
};

static double Tld_Dga_Eval_P(size_t len, uint8_t * val)
{
    size_t count[5];
    double p = 1.0;

    memset(count, 0, sizeof(count));

    for (size_t i=0; i<len; i++)
    {
        uint8_t char_index = val[i] - 10;
        p *= 26.0*char_proba[char_index];
    }

    return p;
}

static double Tld_Dga_Eval_C_U(size_t len, uint8_t * val)
{
    size_t count[5];
    double klt = 0;

    memset(count, 0, sizeof(count));

    for (size_t i = 0; i < len; i++)
    {
        uint8_t char_group_index = char_group[val[i] - 10];
        count[char_group_index] ++;
    }

    for (int j = 0; j < 5; j++)
    {
        double p = ((double)count[j] + (double)char_group_weight[j] / 26.0) / ((double)(len + 1));
        double kl = p * log(26.0*p / double(char_group_weight[j]));
        klt += kl;
    }

    return klt;
}

static double Tld_Dga_Eval_T_U(size_t len, uint8_t * val)
{
    size_t count[4];
    double klt = 0;
    uint8_t previous_group = 2;

    memset(count, 0, sizeof(count));

    for (size_t i = 0; i<len; i++)
    {
        uint8_t t_group_index = transition_group[val[i] - 10];

        if (previous_group < 2)
        {
            count[2 * previous_group + t_group_index] ++;
        }
        previous_group = t_group_index;
    }

    for (int j = 0; j < 4; j++)
    {
        double p = ((double)count[j] + (double)transition_group_weight[j]) / ((double)(len));
        double kl = p * log(p / transition_group_weight[j]);
        klt += kl;
    }

    return klt;
}

static double Tld_Dga_Eval_C_A(size_t len, uint8_t * val)
{
    size_t count[5];
    double klt = 0;

    memset(count, 0, sizeof(count));

    for (size_t i = 0; i<len; i++)
    {
        uint8_t char_group_index = char_group[val[i] - 10];
        count[char_group_index] ++;
    }

    for (int j = 0; j < 5; j++)
    {
        double p = ((double)count[j] + (double)char_group_weight[j] / 26.0) / ((double)(len + 1));
        double kl = p * log(p / char_group_proba[j]);
        klt += kl;
    }

    return klt;
}

static double Tld_Dga_Eval_T_A(size_t len, uint8_t * val)
{
    size_t count[4];
    double klt = 0;
    uint8_t previous_group = 2;

    memset(count, 0, sizeof(count));

    for (size_t i = 0; i<len; i++)
    {
        uint8_t t_group_index = transition_group[val[i] - 10];

        if (previous_group < 2)
        {
            count[2 * previous_group + t_group_index] ++;
        }
        previous_group = t_group_index;
    }

    for (int j = 0; j < 4; j++)
    {
        double p = ((double)count[j] + (double)transition_group_weight[j]) / ((double)(len));
        double kl = p * log(p / transition_group_proba[j]);
        klt += kl;
    }

    return klt;
}


/*
* List of target patterns
*/

NamePatternDef name_pattern_list[] = {
    { "idn_7_64", name_pattern_idn, false, 7, 64 },
    { "num_1_3", name_pattern_num, false, 1, 3 },
    { "num_4_8", name_pattern_num, false, 4, 8 },
    { "dga_alpha_6", name_pattern_alpha, true, 6, 6 },
    { "dga_alpha_7", name_pattern_alpha, true, 7, 7 },
    { "dga_alpha_8", name_pattern_alpha, true, 8, 8 },
    { "dga_alpha_9", name_pattern_alpha, true, 9, 9 },
    { "dga_alpha_10", name_pattern_alpha, true, 10, 10 },
    { "dga_alpha_11", name_pattern_alpha, true, 11, 11 },
    { "dga_alpha_12", name_pattern_alpha, true, 12, 12 },
    { "dga_alpha_13", name_pattern_alpha, true, 13, 13 },
    { "dga_alpha_14", name_pattern_alpha, true, 14, 14 },
    { "dga_alpha_15", name_pattern_alpha, true, 15, 15 },
    { "dga_alpha_16", name_pattern_alpha, true, 16, 16 },
    { "alpha_6_7", name_pattern_alpha, false, 6, 7 },
    { "alpha_8_9", name_pattern_alpha, false, 8, 9 },
    { "alpha_10_12", name_pattern_alpha, true, 10, 12 },
    { "alpha_13_16", name_pattern_alpha, true, 13, 16 }
};

size_t name_pattern_list_nb = sizeof(name_pattern_list) / sizeof(NamePatternDef);

NamePattern::NamePattern()
{
}

NamePattern::~NamePattern()
{
}

bool NamePattern::Preprocess(uint8_t * str, size_t len, uint8_t * val, uint32_t * char_pattern)
{
    bool ret = true;
    uint32_t flags = name_pattern_alphanum |
        name_pattern_alpha | name_pattern_hexa | name_pattern_num;

    for (size_t i = 0; i < len; i++)
    {
        int x = str[i];
        val[i] = 0;

        if (x >= '0' && x <= '9')
        {
            val[i] = x - '0';
            flags &= (name_pattern_alphanum | name_pattern_hexa | name_pattern_num);
        }
        else if (x >= 'a' && x <= 'f')
        {
            val[i] = 10 + x - 'a';
            flags &= (name_pattern_alphanum | name_pattern_alpha | name_pattern_hexa);
        }
        else if (x >= 'g' && x <= 'z')
        {
            val[i] = 10 + x - 'a';
            flags &= (name_pattern_alphanum | name_pattern_alpha);
        }
        else if (x >= 'A' && x <= 'F')
        {
            val[i] = 10 + x - 'A';
            flags &= (name_pattern_alphanum | name_pattern_alpha | name_pattern_hexa);
        }
        else if (x >= 'G' && x <= 'Z')
        {
            val[i] = 10 + x - 'A';
            flags &= (name_pattern_alphanum | name_pattern_alpha);
        }
        else if (x == '-')
        {
            val[i] = 36;
            if (i == 0)
            {
                flags &= (name_pattern_num);
            }
            else
            {
                flags = 0;
            }
        }
        else
        {
            flags = 0;
            ret = false;
            break;
        }
    }

    if (ret && flags == 0 && len > 4)
    {
        if (val[0] == 10 + 'x' - 'a' &&
            val[1] == 10 + 'n' - 'a' &&
            val[2] == 36 && val[3] == 36)
        {
            flags |= name_pattern_idn;
        }
    }

    *char_pattern = flags;

    return ret;
}


/*
 * CheckAlphaRandom implements a decision tree, tuned from a test case
 * of TLD names and randomly generated DGA. This code will have to be
 * updated if we can find better metrics, or better training sets.
 */
bool NamePattern::CheckAlphaRandom(uint8_t * val, size_t len)
{
    bool ret = false;
    double metric_p = Tld_Dga_Eval_P(len, val);
    double metric_t_a;
    double metric_c_a;
    double metric_t_u;
    double metric_c_u;

    if (metric_p < 0.9)
    {
        /* 1- These are most likely DGA strings */
        if (len >= 10)
        {
            /* 12- For long strings, there is no doubt */
            ret = true;
        }
        else
        {
            /* 11 */
            metric_t_a = Tld_Dga_Eval_T_A(len, val);
            if (metric_t_a > 0.5)
            {
                /* 112 */
                /* further evidence that these are DGA strings */
                ret = true;
            }
            else {
                /* 111 */
                metric_t_u = Tld_Dga_Eval_T_U(len, val);
                if (metric_t_u <= 0.25)
                {
                    /* 1111 */
                    if (metric_p < 0.1)
                    {
                        ret = true;
                    }
                    else
                    {
                        metric_c_a = Tld_Dga_Eval_C_A(len, val);
                        if (metric_c_a > 0.4)
                        {
                            ret = metric_t_a < 0.1;
                        }
                        else
                        {
                            metric_c_u = Tld_Dga_Eval_C_U(len, val);
                            if (metric_c_u > 0.19)
                            {
                                ret = metric_t_a > 0.2;
                            }
                            else
                            {
                                ret = metric_t_a > 0.19;
                            }
                        }
                    }
                }
                else
                {
                    /* 1112 */
                    if (metric_t_u > 0.44)
                    {
                        ret = false;
                    }
                    else
                    {
                        metric_c_a = Tld_Dga_Eval_C_A(len, val);
                        ret = (metric_c_a >= 0.275);
                    }
                }
            }
        }
    }
    else
    {
        /* 2 */
        /* These are most likely TLD strings */
        if (len < 9)
        {
            /* 21 */
            metric_t_a = Tld_Dga_Eval_T_A(len, val);
            if (metric_t_a <= 0.1)
            {
                /* 211 */
                ret = false;
            }
            else
            {
                /* 212*/
                if (metric_t_a <= 0.20)
                {
                    ret = false;
                }
                else
                {
                    /* 2122 */
                    if (len <= 6)
                    {
                        ret = false;
                    }
                    else
                    {
                        metric_t_u = Tld_Dga_Eval_T_U(len, val);

                        ret = metric_t_u < 0.125;
                    }
                }
            }
        }
        else
        {
            /* 22 */
            metric_c_u = Tld_Dga_Eval_C_U(len, val);

            if (metric_c_u > 0.3)
            {
                ret = false;
            }
            else
            {
                metric_t_a = Tld_Dga_Eval_T_A(len, val);
                if (metric_t_a > 0.15)
                {
                    ret = true;
                }
                else if (len < 12)
                {
                    ret = false;
                }
                else
                {
                    metric_t_u = Tld_Dga_Eval_T_U(len, val);
                    ret = metric_t_u < 0.35;
                }
            }

        }
    }

    return ret;
}

char const * NamePattern::GetPattern(uint8_t * str, size_t len)
{
    char const * pattern_found = NULL;
    uint8_t val[64];
    uint32_t char_pattern;

    if (len < 64 && Preprocess(str, len, val, &char_pattern))
    {
        bool is_random = false;

        if ((name_pattern_alpha&char_pattern) != 0)
        {
            /* The randomness detection test is only defined for the
             * alphabetic strings */
            is_random = CheckAlphaRandom(val, len);
        }

        for (size_t i = 0; i < name_pattern_list_nb && pattern_found == NULL; i++)
        {
            if ((name_pattern_list[i].char_pattern&char_pattern) != 0 &&
                name_pattern_list[i].length_min <= len &&
                name_pattern_list[i].length_max >= len &&
                (!name_pattern_list[i].is_random || is_random))
            {
                pattern_found = name_pattern_list[i].pattern_name;
            }
        }
    }

    return pattern_found;
}



