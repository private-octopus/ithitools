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

#ifndef HASH_BIN_GENERIC_H
#define HASH_BIN_GENERIC_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

template <class KeyObj>
class BinHash
{
public:
    BinHash()
        :
        tableSize(0),
        tableCount(0),
        hashBin(NULL)
    {}

    ~BinHash()
    {
        Clear();
    }

    KeyObj * InsertOrAdd(KeyObj * key, bool need_alloc, bool* stored)
    {
        KeyObj * retKey = NULL;
        unsigned int newCount = tableCount + 1;

        if (key != NULL)
        {
            if (tableSize < 2 * newCount)
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

                Resize(newSize);
            }

            retKey = DoInsert(key, need_alloc, stored);
        }

        return retKey;
    }

    KeyObj * Retrieve(KeyObj * key)
    {
        KeyObj * ret = 0;

        if (key != 0 && tableSize > 0)
        {
            uint32_t hash_index = key->Hash() % tableSize;

            ret = hashBin[hash_index];

            while (ret != NULL && !key->IsSameKey(ret))
            {
                ret = ret->HashNext;
            }
        }

        return ret;
    }

    KeyObj * Remove(KeyObj * key)
    {
        uint32_t hash_index = key->Hash() % tableSize;
        KeyObj * retKey = hashBin[hash_index];
        KeyObj ** pPrevious = &hashBin[hash_index];

        while (retKey != NULL && !key->IsSameKey(retKey))
        {
            pPrevious = &retKey->HashNext;
            retKey = retKey->HashNext;
        }

        if (retKey != NULL)
        {
            *pPrevious = retKey->HashNext;
            retKey->HashNext = NULL;
            tableCount--;
        }

        return retKey;
    }

    void Clear()
    {
        if (hashBin != NULL)
        {
            for (uint32_t i = 0; i < tableSize; i++)
            {
                while (hashBin[i] != NULL)
                {
                    KeyObj *x = hashBin[i];
                    hashBin[i] = x->HashNext;
                    delete x;
                }
            }

            delete[] hashBin;
            hashBin = NULL;
        }

        tableCount = 0;
        tableSize = 0;
    }

    uint32_t GetCount() {
        return tableCount;
    }

    uint32_t GetSize() {
        return tableSize;
    }

    KeyObj * GetEntry(uint32_t indx)
    {
        return (indx < tableSize) ? hashBin[indx] : NULL;
    }

private:
    uint32_t tableSize;
    uint32_t tableCount;
    KeyObj ** hashBin;

    void Resize(unsigned newSize)
    {
        KeyObj ** oldBins = hashBin;
        uint32_t oldSize = tableSize;

        if (oldSize < newSize)
        {
            KeyObj ** newBins = new KeyObj*[newSize];

            if (newBins == NULL)
            {
                /* Cannot allocate memory. Table is unchanged. */
            }
            else
            {
                hashBin = newBins;
                tableSize = newSize;
                memset(hashBin, 0, sizeof(KeyObj *)*tableSize);
                tableCount = 0;

                if (oldBins != NULL)
                {
                    for (unsigned int i = 0; i < oldSize; i++)
                    {
                        while (oldBins[i] != NULL)
                        {
                            KeyObj* x = oldBins[i];
                            bool stored = false;

                            oldBins[i] = x->HashNext;
                            DoInsert(x, false, &stored);
                            if (!stored)
                            {
                                /* This only happens if there was a duplicate in the original table */
                                delete x;
                            }
                        }
                    }
                    
                    delete[] oldBins;
                }
            }
        }
    }

    KeyObj * DoInsert(KeyObj * key, bool need_alloc, bool * stored)
    {
        KeyObj * retKey = NULL;

        *stored = false;

        if (key != NULL && tableSize > 0)
        {
            uint32_t hash_index = key->Hash() % tableSize;
            KeyObj * oldKey = hashBin[hash_index];

            while (oldKey != NULL && !key->IsSameKey(oldKey))
            {
                oldKey = oldKey->HashNext;
            }

            if (oldKey != NULL)
            {
                /* the keyed object is already in the table*/
                oldKey->Add(key);
                retKey = oldKey;
                *stored = false;
            }
            else
            {
                retKey = key;

                if (need_alloc)
                {
                    retKey = key->CreateCopy();
                }

                if (retKey != NULL)
                {
                    retKey->HashNext = hashBin[hash_index];
                    hashBin[hash_index] = retKey;
                    *stored = true;
                    tableCount++;
                }
            }
        }

        return retKey;
    }
};

template <class KeyObj>
class LruHash
{
public:
    LruHash()
        :
        MostRecentlyUsed(NULL),
        LeastRecentlyUsed(NULL)
    {}

    ~LruHash()
    {
        Clear();
    }

    KeyObj * InsertOrAdd(KeyObj * key, bool need_alloc, bool* stored)
    {
        KeyObj* retKey = binHash.InsertOrAdd(key, need_alloc, stored);

        if (retKey != NULL)
        {
            LruMoveToMostRecent(retKey, *stored);
        }

        return retKey;
    }

    KeyObj * Retrieve(KeyObj * key)
    {
        KeyObj * retKey = binHash.Retrieve(key);

        if (retKey != NULL)
        {
            LruMoveToMostRecent(retKey, false);
        }

        return retKey;
    }

    KeyObj * Remove(KeyObj * key)
    {
        KeyObj * retKey = binHash.Remove(key);

        if (retKey != NULL)
        {
            LruUnlink(retKey);
        }

        return retKey;
    }

    KeyObj * RemoveLRU()
    {
        KeyObj * key = Remove(LeastRecentlyUsed);

        return key;
    }

    uint32_t GetCount() {
        return binHash.GetCount();
    }

    uint32_t GetSize() {
        return binHash.GetSize();
    }

    KeyObj * GetEntry(uint32_t indx)
    {
        return (binHash.GetEntry(indx));
    }

    void Clear()
    {
        binHash.Clear();
        MostRecentlyUsed = NULL;
        LeastRecentlyUsed = NULL;
    }

    KeyObj * MostRecentlyUsed;
    KeyObj * LeastRecentlyUsed;

private:
    BinHash<KeyObj> binHash;
    uint32_t targetSize;

    void LruUnlink(KeyObj * key)
    {
        if (key != NULL)
        {
            /* If the object was newly created, the more and less recent keys should be NULL */

            if (key->MoreRecentKey != NULL)
            {
                key->MoreRecentKey->LessRecentKey = key->LessRecentKey;
            }
            else
            {
                MostRecentlyUsed = key->LessRecentKey;
            }

            if (key->LessRecentKey != NULL)
            {
                key->LessRecentKey->MoreRecentKey = key->MoreRecentKey;
            }
            else
            {
                LeastRecentlyUsed = key->MoreRecentKey;
            }

            key->MoreRecentKey = NULL;
            key->LessRecentKey = NULL;
        }
    }

    void LruMoveToMostRecent(KeyObj * key, bool stored)
    {
        if (key != NULL)
        {
            if (!stored)
            {
                LruUnlink(key);
            }
            key->LessRecentKey = MostRecentlyUsed;
            if (MostRecentlyUsed != NULL)
            {
                MostRecentlyUsed->MoreRecentKey = key;
            }
            MostRecentlyUsed = key;

            if (LeastRecentlyUsed == NULL)
            {
                LeastRecentlyUsed = key;
            }
        }
    }
};


#endif /* HASH_BIN_GENERIC_H */
