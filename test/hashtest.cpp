#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "HashBinGeneric.h"
#include "hashtest.h"

/* List of test strings */
static char const * to_hash[] = {
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
    "corp"
};

static const size_t size_to_hash = sizeof(to_hash) / sizeof(char const *);

static char const * to_not_hash[] = {
    "comg",
    "homestation",
    "backnet",
    "router",
    "gateway",
    "nashr",
    "pk5001z"
};

static const size_t size_to_not_hash = sizeof(to_not_hash) / sizeof(char const *);

static char const * to_hash_long[] = {
    "aaa",
    "aarp",
    "abarth",
    "abb",
    "abbott",
    "abbvie",
    "abc",
    "able",
    "abogado",
    "abudhabi",
    "ac",
    "academy",
    "accenture",
    "accountant",
    "accountants",
    "aco",
    "active",
    "actor",
    "ad",
    "adac",
    "ads",
    "adult",
    "ae",
    "aeg",
    "aero",
    "aetna",
    "af",
    "afamilycompany",
    "afl",
    "africa",
    "ag",
    "agakhan",
    "agency",
    "ai",
    "aig",
    "aigo",
    "airbus",
    "airforce",
    "airtel",
    "akdn",
    "al",
    "alfaromeo",
    "alibaba",
    "alipay",
    "allfinanz",
    "allstate",
    "ally",
    "alsace",
    "alstom",
    "am",
    "americanexpress",
    "americanfamily",
    "amex",
    "amfam",
    "amica",
    "amsterdam",
    "analytics",
    "android",
    "anquan",
    "anz",
    "ao",
    "aol",
    "apartments",
    "app",
    "apple",
    "aq",
    "aquarelle",
    "ar",
    "aramco",
    "archi",
    "army",
    "arpa",
    "art",
    "arte",
    "as",
    "asda",
    "asia",
    "associates",
    "at",
    "athleta",
    "attorney",
    "au",
    "auction",
    "audi",
    "audible",
    "audio",
    "auspost",
    "author",
    "auto",
    "autos",
    "avianca",
    "aw",
    "aws",
    "ax",
    "axa",
    "az",
    "azure",
    "ba",
    "baby",
    "baidu",
    "banamex",
    "bananarepublic",
    "band",
    "bank",
    "bar",
    "barcelona",
    "barclaycard",
    "barclays",
    "barefoot",
    "bargains",
    "baseball",
    "basketball",
    "bauhaus",
    "bayern",
    "bb",
    "bbc",
    "bbt",
    "bbva",
    "bcg",
    "bcn",
    "bd",
    "be",
    "beats",
    "beauty",
    "beer",
    "bentley",
    "berlin",
    "best",
    "bestbuy",
    "bet",
    "bf",
    "bg",
    "bh",
    "bharti",
    "bi",
    "bible",
    "bid",
    "bike",
    "bind",
    "bing",
    "bingo",
    "bio",
    "biz",
    "bj",
    "black",
    "blackfriday",
    "blanco",
    "blockbuster",
    "blog",
    "bloomberg",
    "blue",
    "bm",
    "bms",
    "bmw",
    "bn",
    "bnl",
    "bnpparibas",
    "bo",
    "boats",
    "boehringer",
    "bofa",
    "bom",
    "bond",
    "boo",
    "book",
    "booking",
    "boots",
    "bosch",
    "bostik",
    "boston",
    "bot",
    "boutique",
    "box",
    "br",
    "bradesco",
    "bridgestone",
    "broadway",
    "broker",
    "brother",
    "brussels",
    "bs",
    "bt",
    "budapest",
    "bugatti",
    "build",
    "builders",
    "business",
    "buy",
    "buzz",
    "bv",
    "bw",
    "by",
    "bz",
    "bzh",
    "ca",
    "cab",
    "cafe",
    "cal",
    "call",
    "calvinklein",
    "cam",
    "camera",
    "camp",
    "cancerresearch",
    "canon",
    "capetown",
    "capital",
    "capitalone",
    "car",
    "caravan",
    "cards",
    "care",
    "career",
    "careers",
    "cars",
    "cartier",
    "casa",
    "caseih",
    "cash",
    "casino",
    "cat",
    "catering",
    "catholic",
    "cba",
    "cbn",
    "cbre",
    "cbs",
    "cc",
    "cd",
    "ceb",
    "center",
    "ceo",
    "cern",
    "cf",
    "cfa",
    "cfd",
    "cg",
    "ch",
    "chanel",
    "channel",
    "chase",
    "chat",
    "cheap",
    "chintai",
    "chloe",
    "christmas",
    "chrome",
    "chrysler",
    "church",
    "ci",
    "cipriani",
    "circle",
    "cisco",
    "citadel",
    "citi",
    "citic",
    "city",
    "cityeats",
    "ck",
    "cl",
    "claims",
    "cleaning",
    "click",
    "clinic",
    "clinique",
    "clothing",
    "cloud",
    "club",
    "clubmed",
    "cm",
    "cn",
    "co",
    "coach",
    "codes",
    "coffee",
    "college",
    "cologne",
    "com"
};

const size_t size_to_hash_long = sizeof(to_hash_long) / sizeof(char const *);

class hashTestKey
{
public:

    hashTestKey(uint8_t const * text, size_t text_len, size_t indx)
        :
        count(1),
        hash(0),
        HashNext(NULL),
        LessRecentKey(NULL),
        MoreRecentKey(NULL),
        indx(indx)
    {
        if (text_len > 16)
        {
            text_len = 16;
        }

        memcpy(this->text, text, text_len);
        this->text_len = (uint32_t) text_len;
    }

    ~hashTestKey()
    {}

    bool IsSameKey(hashTestKey * key)
    {
        bool ret = (key->text_len == this->text_len &&
            memcmp(key->text, this->text, this->text_len) == 0);
        return ret;
    };

    uint32_t Hash()
    {
        if (hash == 0)
        {
            hash = 0xDEADBEEF;
            for (size_t i = 0; i < text_len; i++)
            {
                hash = hash * 101 + text[i];
            }
        }
        return hash;
    };

    hashTestKey * CreateCopy()
    {
        hashTestKey * x = new hashTestKey(text, text_len, indx);

        if (x != NULL)
        {
            x->count = count;
            x->hash = hash;
        }

        return x;
    };

    void Add(hashTestKey * key)
    {
        count += key->count;
    };

    uint32_t count;
    uint8_t text[16];
    uint32_t text_len;
    uint32_t hash;
    size_t indx;
    hashTestKey * HashNext;
    hashTestKey * LessRecentKey;
    hashTestKey * MoreRecentKey;

};

hashtest::hashtest()
{
}

hashtest::~hashtest()
{
}

bool hashtest::DoBinHashTest(char const ** hash_input, size_t nb_input)
{
    BinHash<hashTestKey> hashTable;
    bool ret = true;

    /* Enter all the data in the input table */
    for (size_t i = 0; ret && i < nb_input; i++)
    {
        hashTestKey key((uint8_t const *)hash_input[i], strlen(hash_input[i]), i);
        bool stored;

        hashTestKey * retKey = hashTable.InsertOrAdd(&key, true, &stored);

        if (retKey == NULL)
        {
            ret = false;
        }
        else if (!stored)
        {
            ret = false;
        }
    }

    if (ret && hashTable.GetCount() != nb_input)
    {
        ret = false;
    }

    /* Verify that all the data can be rewritten without creating new entries */
    for (size_t i = 0; ret && i < nb_input; i++)
    {
        hashTestKey key((uint8_t const *)hash_input[i], strlen(hash_input[i]), i);
        bool stored;

        hashTestKey * retKey = hashTable.InsertOrAdd(&key, true, &stored);

        if (retKey == NULL)
        {
            ret = false;
        }
        else if (stored)
        {
            ret = false;
        }
        else if (retKey->count != 2)
        {
            ret = false;
        }
    }

    /* Verify that all the data can be retrieved */
    for (size_t i = 0; ret && i < nb_input; i++)
    {
        hashTestKey key((uint8_t const *)hash_input[i], strlen(hash_input[i]), i);

        hashTestKey * retKey = hashTable.Retrieve(&key);

        if (retKey == NULL)
        {
            ret = false;
        }
        else if (!key.IsSameKey(retKey))
        {
            ret = false;
        }
        else if (retKey->count != 2)
        {
            ret = false;
        }
    }

    /* Verify that all the non-data cannot be retrieved */
    for (size_t i = 0; ret && i < size_to_not_hash; i++)
    {
        hashTestKey key((uint8_t const *)to_not_hash[i], strlen(to_not_hash[i]), 0xFF);

        hashTestKey * retKey = hashTable.Retrieve(&key);

        if (retKey != NULL)
        {
            ret = false;
        }
    }

    return ret;
}

bool hashtest::LruCheck(void * vtable)
{
    bool ret = true;
    LruHash<hashTestKey> * hashTable = (LruHash<hashTestKey> *) vtable;

    /* Check that the LRU Hash order is correct -- should be same as table order */

    size_t lru_counted = 0;
    hashTestKey * previous = NULL;
    hashTestKey * next = hashTable->LeastRecentlyUsed;

    while (next != NULL)
    {
        if (previous != NULL && previous->indx >= next->indx)
        {
            ret = false;
            break;
        }
        previous = next;
        next = next->MoreRecentKey;
        lru_counted++;
    }

    if (ret && lru_counted != hashTable->GetCount())
    {
        ret = false;
    }

    if (ret)
    {
        lru_counted = 0;
        previous = NULL;
        next = hashTable->MostRecentlyUsed;

        while (next != NULL)
        {
            if (previous != NULL && previous->indx <= next->indx)
            {
                ret = false;
                break;
            }
            previous = next;
            next = next->LessRecentKey;
            lru_counted++;
        }

        if (ret && lru_counted != hashTable->GetCount())
        {
            ret = false;
        }
    }

    return ret;
}

bool hashtest::DoLruHashTest(char const ** hash_input, size_t nb_input)
{
    LruHash<hashTestKey> hashTable;
    size_t target_size = nb_input;
    bool ret = true;

    /* Enter all the data in the input table */
    for (size_t i = 0; ret && i < nb_input; i++)
    {
        hashTestKey key((uint8_t const *)hash_input[i], strlen(hash_input[i]), i);
        bool stored;

        hashTestKey * retKey = hashTable.InsertOrAdd(&key, true, &stored);

        if (i == 0x41)
        {
            ret = LruCheck((void*)&hashTable);
        }

        if (retKey == NULL)
        {
            ret = false;
        }
        else if (!stored)
        {
            ret = false;
        }
    }

    if (ret && hashTable.GetCount() != target_size)
    {
        ret = false;
    }

    if (ret)
    {
        ret = LruCheck((void*)&hashTable);
    }

    /* Verify that all the data can be rewritten without creating new entries */
    for (size_t i = 0; ret && i < nb_input; i++)
    {
        hashTestKey key((uint8_t const *)hash_input[i], strlen(hash_input[i]), i);
        bool stored;

        hashTestKey * retKey = hashTable.InsertOrAdd(&key, true, &stored);

        if (retKey == NULL)
        {
            ret = false;
        }
        else if (stored)
        {
            ret = false;
        }
        else if (retKey->count != 2)
        {
            ret = false;
        }
    }

    if (ret)
    {
        ret = LruCheck((void*)&hashTable);
    }

    /* Verify that all the data can be retrieved */
    for (size_t i = 0; ret && i < nb_input; i++)
    {
        hashTestKey key((uint8_t const *)hash_input[i], strlen(hash_input[i]), i);

        hashTestKey * retKey = hashTable.Retrieve(&key);

        if (retKey == NULL)
        {
            ret = false;
        }
        else if (!key.IsSameKey(retKey))
        {
            ret = false;
        }
        else if (retKey->count != 2)
        {
            ret = false;
        }
    }

    if (ret)
    {
        ret = LruCheck((void*)&hashTable);
    }

    /* Verify that all the non-data cannot be retrieved */
    for (size_t i = 0; ret && i < size_to_not_hash; i++)
    {
        hashTestKey key((uint8_t const *)to_not_hash[i], strlen(to_not_hash[i]), 0xFF);

        hashTestKey * retKey = hashTable.Retrieve(&key);

        if (retKey != NULL)
        {
            ret = false;
        }
    }

    /* Check that the LRU Hash order is correct -- should be same as table order */

    if (ret)
    {
        ret = LruCheck((void*)&hashTable);
    }

    /* Delete some random object, then check the table */
    if (ret)
    {
        size_t i = nb_input / 2;
        hashTestKey key((uint8_t const *)hash_input[i], strlen(hash_input[i]), i);
        hashTestKey * retKey = hashTable.Remove(&key);

        if (retKey == NULL)
        {
            ret = false;
        }
        else
        {
            delete retKey;
            target_size--;

            if (hashTable.GetCount() != target_size)
            {
                ret = false;
            }
            else
            {
                ret = LruCheck((void*)&hashTable);
            }     
        }
    }

    /* Delete the LRU object, then check the table */
    while (ret && hashTable.GetCount() > 0)
    {
        hashTestKey * retKey = hashTable.RemoveLRU();
        target_size--;

        if (retKey == NULL)
        {
            ret = false;
        }
        else
        {
            delete retKey;

            if (hashTable.GetCount() != target_size)
            {
                ret = false;
            }
            else
            {
                ret = LruCheck((void*)&hashTable);
            }
        }
    }
    return ret;
}

bool hashtest::DoTest()
{
    bool ret = DoBinHashTest(to_hash, size_to_hash);

    if (ret)
    {
        ret = DoBinHashTest(to_hash_long, size_to_hash_long);
    }

    if (ret)
    {
        ret = DoLruHashTest(to_hash, size_to_hash);
    }

    if (ret)
    {
        ret = DoLruHashTest(to_hash_long, size_to_hash_long);
    }
    return ret;
}