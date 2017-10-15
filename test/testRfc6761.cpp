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
