#include <string.h>
#include "DnsStatHash.h"


DnsStatHash::DnsStatHash()
    :
    tableSize(0),
    tableCount(0),
    hashTable(NULL)
{
}


DnsStatHash::~DnsStatHash()
{
    Clear();
}

void DnsStatHash::Clear()
{
    if (hashTable != NULL)
    {
        for (uint32_t i = 0; i < tableSize; i++)
        {
            if (hashTable[i] != NULL)
            {
                delete hashTable[i];
                hashTable[i] = NULL;
            }
        }

        delete[] hashTable;
        hashTable = NULL;
    }

    tableCount = 0;
    tableSize = 0;
}

bool DnsStatHash::Resize(unsigned newSize)
{
    bool ret = false;
    dns_registry_entry_t ** oldTable = hashTable;
    unsigned int oldSize = tableSize;

    if (oldSize >= newSize)
    {
        ret = true;
    }
    else
    {
        dns_registry_entry_t ** newTable = new dns_registry_entry_t*[newSize];

        if (newTable != NULL)
        {
            hashTable = newTable;
            tableSize = newSize;
            memset(hashTable, 0, sizeof(dns_registry_entry_t *)*tableSize);
            ret = true;
            tableCount = 0;

            if (oldTable != NULL)
            {
                for (unsigned int i = 0; ret && i < oldSize; i++)
                {
                    if (oldTable[i] != NULL)
                    {
                        ret = DoInsert(oldTable[i], false);
                    }
                }

                if (!ret)
                {
                    hashTable = oldTable;
                    tableSize = oldSize;
                    delete[] newTable;
                }
                else
                {
                    delete[] oldTable;
                }
            }
        }
    }

    return ret;
}

bool DnsStatHash::InsertOrAdd(dns_registry_entry_t* key, bool need_alloc)
{
    bool ret = true;
    unsigned int newCount = tableCount + 1;

    if (key == 0)
    {
        ret = false;
    }
    else if (tableSize < 2 * newCount)
    {
        unsigned int newSize = tableSize;

        if (tableSize == 0)
        {
            newSize = 128;
        }

        while (newSize < 4 * newCount)
        {
            newSize *= 2;
        }

        ret = Resize(newSize);
    }

    if (ret)
    {
        key->hash = ComputeHash(key);
        ret = DoInsert(key, need_alloc);
    }

    return ret;
}

bool DnsStatHash::Retrieve(dns_registry_entry_t * key, uint32_t * count)
{
    bool ret = false;
    unsigned int hash_index;

    *count = 0;
    key->hash = ComputeHash(key);
    hash_index = key->hash%tableSize;

    for (unsigned int i = 0; i < tableSize; i++)
    {
        if (hashTable[hash_index] == NULL)
        {
            break;
        }
        else if (IsSameKey(hashTable[hash_index], key))
        {
            /* found it. return the count */
            *count = hashTable[hash_index]->count;
            ret = true;
            break;
        }
        else
        {
            hash_index++;

            if (hash_index >= tableSize)
            {
                hash_index = 0;
            }
        }
    }

    return ret;
}

uint32_t DnsStatHash::GetCount() {
    return tableCount;
}

uint32_t DnsStatHash::GetSize() {
    return tableSize;
}

dns_registry_entry_t * DnsStatHash::GetEntry(uint32_t indx)
{
    return (indx < tableSize) ? hashTable[indx] : NULL;
}

bool DnsStatHash::DoInsert(dns_registry_entry_t* key, bool need_alloc)
{
    bool ret = false;
    unsigned int hash_index = key->hash%tableSize;

    for (unsigned int i = 0; i < tableSize; i++)
    {
        if (hashTable[hash_index] == NULL)
        {
            if (need_alloc)
            {
                hashTable[hash_index] = new dns_registry_entry_t;
                if (hashTable[hash_index] != NULL)
                {
                    memcpy(hashTable[hash_index], key, sizeof(dns_registry_entry_t));

                    tableCount++;
                    ret = true;
                }
                else
                {
                    ret = false;
                }
            }
            else
            {
                hashTable[hash_index] = key;
                tableCount++;
                ret = true;
            }
            break;
        }
        else if (IsSameKey(hashTable[hash_index], key))
        {
            /* found it. Just increment the counter */
            hashTable[hash_index]->count++;
            ret = false;
            break;
        }
        else
        {
            hash_index++;

            if (hash_index >= tableSize)
            {
                hash_index = 0;
            }
        }
    }

    return ret;
}

uint32_t DnsStatHash::ComputeHash(dns_registry_entry_t * key)
{
    uint64_t hash64 = 0;

    hash64 = key->registry_id;
    hash64 ^= (hash64 << 23) ^ (hash64 >> 17);
    hash64 ^= key->key_type;
    hash64 ^= (hash64 << 23) ^ (hash64 >> 17);
    hash64 ^= key->key_length;
    hash64 ^= (hash64 << 23) ^ (hash64 >> 17);
    for (uint32_t i = 0; i < key->key_length; i++)
    {
        hash64 ^= key->key_value[i];
        hash64 ^= (hash64 << 23) ^ (hash64 >> 17);
    }

    return (uint32_t)(hash64 ^ (hash64 >> 32));
}

bool DnsStatHash::IsSameKey(dns_registry_entry_t * key1, dns_registry_entry_t * key2)
{
    bool ret = key1->hash == key2->hash &&
        key1->registry_id == key2->registry_id &&
        key1->key_type == key2->key_type &&
        key1->key_length == key2->key_length &&
        memcmp(key1->key_value, key2->key_value, key1->key_length) == 0;

    return ret;
}

