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
#include "NamePattern.h"
#include "PatternTest.h"

static char const * alpha_test_strings[] = {
    "local",
    "home",
    "ip",
    "localdomain",
    "dhcp",
    "localhost",
    "localnet",
    "lan",
    "telus",
    "internal",
    "belkin",
    "invalid",
    "workgroup",
    "domain",
    "corp",
    "comg",
    "homestation",
    "backnet",
    "router",
    "gateway",
    "nashr",
    "pvt",
    "rbl",
    "toj",
    "actdsltmp",
    "dlinkrouter",
    "guest",
    "intra",
    "intranet",
    "reviewimages",
    "totolink",
    "airdream",
    "dlink",
    "station",
    "tendaap",
    "arris",
    "be",
    "blinkap",
    "enterprise",
    "mickeymouse",
    "netg",
    "openstacklocal",
    "private",
    "realtek",
    "site",
    "wimax",
    "domainname",
    "server",
    "yfserver",
    "oops",
    "fco",
    "public"
};

static char const * dga_test_strings[] = {
    "UABXC",
    "OAJYPL",
    "HXCAIF",
    "SQRNAN",
    "LSYBHX",
    "KGJGHC",
    "MPKNUN",
    "DWBHEO",
    "ODGYSS",
    "CUBXMR",
    "GSRGHXK",
    "JVLCOFI",
    "XMMWWAQ",
    "WOHDDKE",
    "USOLUNQ",
    "TFPENXO",
    "FYXWVXS",
    "KGPSQDH",
    "SDSJEUV",
    "YMVVXKV",
    "GNTRPORN",
    "HGSCKOIK",
    "DCNXTYKK",
    "ILFFOOFO",
    "NNVQHFNC",
    "FRJAVYTT",
    "RHWDQQHC",
    "TAQDXBJQ",
    "LWHICUTB",
    "GYNAXCJV",
    "DLSYJUKTF",
    "KHHHQLIJM",
    "UFSURVJVO",
    "JOFATXXFV",
    "LIJIFTQMQ",
    "VAUJDYTGL",
    "PJWGUKWTB",
    "PQBNNOCWK",
    "DAYUJQURY",
    "JYSHIAEFB",
    "BUCQGVMTBJ",
    "NVQTSTJRQJ",
    "LNIPRUISSM",
    "BASYFFAEPC",
    "QOVJMDRNFG",
    "TPBKPGPPBT",
    "TOMIMOQGNX",
    "CSLQAVLYGA",
    "CUNCCFSROF",
    "UTQUNRWTSC",
    "BKRSEEBIEOG",
    "MHHCGJBUWAX",
    "WUFNMTXAKBY",
    "ACGWTANXSPC",
    "ARTMYTERLBE",
    "DOXFXATNCSM",
    "IPBKTSEBJOG",
    "PDKCJCDGHKQ",
    "NQWMVJKJYMR",
    "FLYWDQHXQFD",
    "LKVPHCQYWIHT",
    "QMOYGUYMWOFY",
    "UPVKDGCRKPEF",
    "XFNGURUQKDBW",
    "FEIFXJXQJEDU",
    "DVXBOGYITOUB",
    "ICHKTJDEBVNA",
    "QWXJYEVHBTTE",
    "XKWVTBWPBSLU",
    "GHVYDFRQVQIK",
    "FTSWTBOCLJAXC",
    "SEXYJKONUSSGL",
    "GYPYEMKDJNEOS",
    "SPBKFOBFLRPVG",
    "MRWWSWUPLFLRN",
    "XGUKBFWQJDVEV",
    "LRRPFWFGQGJHT",
    "CGFERUHFBEMXL",
    "RSGLPBXQMALQP",
    "HQLFORNEFPKGF",
    "JJRDTXPCKEKCDS",
    "IYOBSBSBIXSXGE",
    "RYSQMAWTXWDHFO",
    "BUIUHGCNXRCNGT",
    "CAECYADNIJKXWM",
    "UBGDJGLXYIUWOF",
    "RYGJEVUAXMPTMH",
    "FKHPMUOXDXCOFQ",
    "NYEIBCNKWLQRQY",
    "FHKPUFKWIFEREJ",
    "OFMBRCHXNVJNUPA",
    "YAWEDTADBHUYIJK",
    "TRGGHMPQXXSXPBM",
    "DWOLELPPGJRKQFA",
    "JYLCKCXOHRCIWWP",
    "VXUDCAVXFUPDTEN",
    "SRAGVIEPXUQCWFU",
    "XEWIFYFQDCIFOEJ",
    "MYQRABLQQFSXWJN",
    "REBKQMUULWRKMBE"
};

static char const * hexa_test_strings[] = {
    "CDE48D",
    "DD68DC",
    "21EEB4",
    "662D3F",
    "B8DD7F",
    "B8FADD",
    "051BED",
    "9C6B10",
    "04E8BB",
    "D04293",
    "FDEB979",
    "B533740",
    "86E7BB8",
    "1326950",
    "2C1470E",
    "1979FF7",
    "71779DC",
    "B54B511",
    "8BE9421",
    "D5B80D5",
    "B9D7B99C",
    "BC371F00",
    "52CA5C71",
    "FAD0DD9F",
    "504F6E40",
    "C925DAEE",
    "30156F9B",
    "6A6CC3FB",
    "652C496A",
    "F07B88B8",
    "5BF21DB22",
    "18E744C94",
    "546D24E49",
    "888F0C216",
    "CE57EF2DA",
    "7C50747E3",
    "94FC618FD",
    "FEF92BAB7",
    "F95A08ED6",
    "B10C63DDA",
    "E01DA44C94",
    "449A28BEE2",
    "9511A4B3F5",
    "30A4AA065E",
    "D001C8EBD6",
    "5E0FC51FC6",
    "D08B2CF704",
    "54D5484DB7",
    "820989B599",
    "B08B8AEE49",
    "93D324E698A",
    "BBD955DAD4E",
    "023A3622EAC",
    "D58ECE104B3",
    "07A45C6713E",
    "7529C4B5CBE",
    "6BCF9ECDB05",
    "2B31F47835D",
    "D720F86F131",
    "F7E835D226C",
    "930F107EA288",
    "5EF95AF37F2F",
    "5762715A564E",
    "542792C6C351",
    "92D03B9A6CC0",
    "09268B7F2121",
    "EC3254BE0F39",
    "960B8380487F",
    "1244C9A0F17B",
    "64AADD90516D",
    "622E79DFFDC2D",
    "7C6540C1E014D",
    "EB42D5F202899",
    "8033CCF2E05B9",
    "D616C8BD558A2",
    "0D0D7D53F5F48",
    "96E94D44FA9F4",
    "A412E953738B6",
    "EEA50D56ABA90",
    "7E46557FF87EA",
    "7B9B3F68F373D1",
    "15B6A570C14D54",
    "C02922FA1475C8",
    "E8B128C18882B8",
    "A693FE485B2C74",
    "94D03BE70E630E",
    "AFD49172FD6C44",
    "93405C16AF4629",
    "E68F0CDFD90FEE",
    "F2CD6DFF7D0F36",
    "3EE9D38E0EDF939",
    "647266F284924E6",
    "A064084AAED4E87",
    "89D05D446F1D532",
    "0ED3495B8877A0A",
    "2AED0505F5D70F3",
    "47AC184EEBAFEF7",
    "5F2585784C1AA8D",
    "9C69CF53F0B5992",
    "011A56A4F5D5A1D"
};

static char const * num_test_strings[] = {
    "648",
    "327",
    "817",
    "599",
    "265",
    "852",
    "142",
    "281",
    "665",
    "002",
    "6570",
    "6839",
    "8457",
    "3514",
    "6961",
    "1168",
    "4016",
    "8599",
    "9293",
    "7221",
    "29849",
    "00082",
    "90525",
    "85025",
    "35808",
    "68384",
    "99312",
    "97955",
    "31912",
    "47008",
    "892107",
    "119959",
    "495111",
    "942922",
    "351171",
    "703643",
    "487807",
    "957560",
    "771851",
    "871199",
    "2870346",
    "1741177",
    "6615749",
    "9777367",
    "7433883",
    "8998945",
    "4790205",
    "6768462",
    "5751598",
    "9780098",
    "66718574",
    "82686013",
    "61436233",
    "67312556",
    "63720579",
    "6661919",
    "56023693",
    "20833947",
    "57024032",
    "12170507",
    "277196047",
    "095612156",
    "049759329",
    "322243357",
    "201147357",
    "258301853",
    "223171979",
    "684819398",
    "505766519",
    "558252248"
};


PatternTest::PatternTest()
{
}


PatternTest::~PatternTest()
{
}

bool PatternTest::TestOnePattern(char const  ** test_strings,
    size_t nb_test_strings, uint32_t pattern)
{
    bool ret = true;
    uint8_t val[64];
    uint32_t flags;
    size_t len;

    for (size_t i = 0; ret && i < nb_test_strings; i++)
    {
        flags = 0;
        len = strlen(test_strings[i]);
        if (!NamePattern::Preprocess((uint8_t *)test_strings[i], len, val, &flags))
        {
            ret = false;
        }
        else if ((flags&pattern) == 0)
        {
            ret = false;
        }
    }

    return ret;
}

bool PatternTest::TestRandom(char const  ** test_strings,
    size_t nb_test_strings, bool is_random)
{
    bool ret = true;
    uint8_t val[64];
    uint32_t flags;
    size_t len;
    uint32_t nb_test = 0;
    uint32_t nb_fail = 0;

    for (size_t i = 0; ret && i < nb_test_strings; i++)
    {
        flags = 0;
        len = strlen(test_strings[i]);
        if (len < 6)
        {
            continue;
        }

        nb_test++;

        if (!NamePattern::Preprocess((uint8_t *)test_strings[i], len, val, &flags))
        {
            ret = false;
        }
        else if ((flags&name_pattern_alpha) == 0)
        {
            ret = false;
        }
        else
        {
            bool test_random = NamePattern::CheckAlphaRandom(val, len);

            if ((test_random & !is_random) ||
                (!test_random & is_random))
            {
                nb_fail++;
            }
        }
    }

    if (ret)
    {
        /* Note that this test is *very* lenient. We do not
           have in fact a good classifier, and it tends to classify
           as "random" strings that are not, like "belkin". But
           building the right classifier is a long term project.
           This test will be revisited when at a later time we
           get a better classifier. */

        if (nb_fail * 25 > nb_test * 10)
        {
            ret = false;
        }
    }

    return ret;
}

bool PatternTest::DoTest()
{
    bool ret = true;

    if (!TestOnePattern(alpha_test_strings, 
        sizeof(alpha_test_strings) / sizeof(char const *),
        name_pattern_alpha))
    {
        ret = false;
    }
    else if (!TestOnePattern(dga_test_strings, 
        sizeof(dga_test_strings) / sizeof(char const *),
        name_pattern_alpha))
    {
        ret = false;
    }
    else if (!TestOnePattern(hexa_test_strings,
        sizeof(hexa_test_strings) / sizeof(char const *),
        name_pattern_hexa))
    {
        ret = false;
    }
    else if (!TestOnePattern(num_test_strings,
        sizeof(num_test_strings) / sizeof(char const *),
        name_pattern_num))
    {
        ret = false;
    }
    else if(!TestRandom(alpha_test_strings,
        sizeof(alpha_test_strings) / sizeof(char const *),
        false))
    {
        ret = false;
    }
    else if (!TestRandom(dga_test_strings,
        sizeof(dga_test_strings) / sizeof(char const *),
        true))
    {
        ret = false;
    }
    return ret;
}
