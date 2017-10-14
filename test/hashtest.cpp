#include <stdint.h>
#include <string.h>
#include "HashBinGeneric.h"
#include "hashtest.h"

/* List of test strings */
char const * to_hash[] = {
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

const size_t size_to_hash = sizeof(to_hash) / sizeof(char const *);

char const * to_not_hash[] = {
    "comg",
    "homestation",
    "backnet",
    "router",
    "gateway",
    "nashr",
    "pk5001z"
};

const size_t size_to_not_hash = sizeof(to_not_hash) / sizeof(char const *);

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
        this->text_len = text_len;
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

bool hashtest::DoBinHashTest()
{
    BinHash<hashTestKey> hashTable;
    bool ret = true;

    /* Enter all the data in the input table */
    for (size_t i = 0; ret && i < size_to_hash; i++)
    {
        hashTestKey key((uint8_t const *)to_hash[i], strlen(to_hash[i]), i);
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

    if (ret && hashTable.GetCount() != size_to_hash)
    {
        ret = false;
    }

    /* Verify that all the data can be rewritten without creating new entries */
    for (size_t i = 0; ret && i < size_to_hash; i++)
    {
        hashTestKey key((uint8_t const *)to_hash[i], strlen(to_hash[i]), i);
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
    for (size_t i = 0; ret && i < size_to_hash; i++)
    {
        hashTestKey key((uint8_t const *)to_hash[i], strlen(to_hash[i]), i);

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

bool hashtest::DoLruHashTest()
{
    LruHash<hashTestKey> hashTable;
    bool ret = true;

    /* Enter all the data in the input table */
    for (size_t i = 0; ret && i < size_to_hash; i++)
    {
        hashTestKey key((uint8_t const *)to_hash[i], strlen(to_hash[i]), i);
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

    if (ret && hashTable.GetCount() != size_to_hash)
    {
        ret = false;
    }

    if (ret)
    {
        ret = LruCheck((void*)&hashTable);
    }

    /* Verify that all the data can be rewritten without creating new entries */
    for (size_t i = 0; ret && i < size_to_hash; i++)
    {
        hashTestKey key((uint8_t const *)to_hash[i], strlen(to_hash[i]), i);
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
    for (size_t i = 0; ret && i < size_to_hash; i++)
    {
        hashTestKey key((uint8_t const *)to_hash[i], strlen(to_hash[i]), i);

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

    return ret;
}

bool hashtest::DoTest()
{
    bool ret = DoBinHashTest();

    if (ret)
    {
        ret = DoLruHashTest();
    }

    return ret;
}