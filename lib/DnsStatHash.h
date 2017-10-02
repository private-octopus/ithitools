#ifndef DNSSTAT_HASH_H
#define DNSSTAT_HASH_H

#include <stdint.h>
#include <stdio.h>

/*
* Accumulate statistics:
*
* Statistics are of the form "Registry, Key value, counter". These are maintained
* in a big hash table, initialized to nothing at the beginning of the run. Each
* packet that we parse provides some entries of the form "Registry, Key Value, 1 occurence".
* If the value is present in the hash base, increment the counter, else,
* create the entry.
*
* At the end of the run, write the values in CSV file.
*/

typedef struct dns_registry_entry_s {
    uint32_t hash;
    uint32_t registry_id;
    uint32_t count;
    uint32_t key_type;
    uint32_t key_length;
    union {
        uint32_t key_number;
        uint8_t key_value[64];
    };
} dns_registry_entry_t;

class DnsStatHash
{
public:
    DnsStatHash();
    ~DnsStatHash();

    bool Resize(unsigned tableSize);
    bool InsertOrAdd(dns_registry_entry_t * key, bool need_alloc = true);
    bool Retrieve(dns_registry_entry_t * key, uint32_t * count);

    uint32_t GetCount();

    uint32_t GetSize();

    dns_registry_entry_t * GetEntry(uint32_t indx);

private:
    uint32_t tableSize;
    uint32_t tableCount;
    dns_registry_entry_t ** hashTable;

    void Clear();
    bool DoInsert(dns_registry_entry_t * key, bool need_alloc);
    static uint32_t ComputeHash(dns_registry_entry_t * key);
    static bool IsSameKey(dns_registry_entry_t * key1, dns_registry_entry_t * key2);
};

#endif /* DNSSTAT_HASH_H */