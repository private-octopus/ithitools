
#include <stdio.h>
#include <algorithm>
#include <string.h>
#include <stdint.h>
#include "HashBinGeneric.h"
#include "CaptureSummary.h"
#include "DnsStats.h"
#include "ithiutil.h"
#include "M7Getter.h"




M7Getter::M7Getter()
    :
    nb_tld_queried(0),
    nb_ds_present(0),
    nb_cc_tld_queried(0),
    nb_cc_ds_present(0)
{
}


M7Getter::~M7Getter()
{
}

bool M7Getter::GetM7(char const * root_zone_file_name)
{
    
    char buffer[512];
    BinHash<TldDSAsKey> table;
    bool ret = true;
    FILE* F = ithi_file_open(root_zone_file_name, "r");

    if (F == NULL)
    {
        ret = false;
    }
    else
    {
        /*
         * Read the file and parse the records.
         */
        while (fgets(buffer, sizeof(buffer), F) != NULL)
        {
            /* Parse to find the name and the record type.
             * If this is a TLD, check whether this is a DS record, and add to hash table.
             */
            char * tld;
            size_t tld_length;
            bool has_ds;

            if (ParseRecord(buffer, &tld, &tld_length, &has_ds))
            {
                TldDSAsKey key(tld, tld_length, has_ds);
                bool stored = false;

                (void)table.InsertOrAdd(&key, true, &stored);
            }
        }

        fclose(F);

        /*
         * Read the hash table and count the tld and ds.
         */
        for (uint32_t i = 0; i < table.GetSize(); i++)
        {
            TldDSAsKey * key = table.GetEntry(i);

            while (key != NULL)
            {
                nb_tld_queried++;

                if (key->ds_count > 0)
                {
                    nb_ds_present++;
                }

                if (key->name_len == 2) {
                    nb_cc_tld_queried++;

                    if (key->ds_count > 0)
                    {
                        nb_cc_ds_present++;
                    }
                }

                key = key->HashNext;
            }
        }
    }

    return ret;
}

/*
 * The root zone format is defined in
 * We consider records like this:
 * 1) Generic:
 *      aaa.			172800	IN	NS	ns4.dns.nic.aaa.
 * 2) Continuation:
 *      /dOaASaogqVsGxL5GyvYqb64s+2FpMVQJC0L4iTfg7mJxl0trJliMpOfco9+7qfxrU6ogYdNOw==
 *
 * We are only concerned with the name, and with the record type.
 */

bool M7Getter::ParseRecord(char * buffer, char ** p_tld, size_t * tld_length, bool * has_ds)
{
    bool ret = true;
    size_t i = 0;
    int x;
    int nb_dots = 0;


    *p_tld = buffer;
    *tld_length = 0;
    *has_ds = false;

    for (i = 0; (x = buffer[i]) != 0; i++)
    {
        if ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z') ||
            (x >= '0' && x <= '9') || (x == '-'))
        {
            continue;
        }
        else if (x == '.')
        {
            nb_dots++;
        }
        else
        {
            break;
        }
    }

    if (i <= 2 || nb_dots > 1)
    {
        ret = false;
    }
    else
    {
        *tld_length = i - 1;

        /* skip spaces */
        if (buffer[i] != ' ' && buffer[i] != '\t')
        {
            ret = false;
        }
        else
        {
            do
            {
                i++;
            } while (buffer[i] == ' ' || buffer[i] == '\t');
        }
        /* skip TTL */
        if (buffer[i] < '0' || buffer[i] > '9')
        {
            ret = false;
        }
        else
        {
            do
            {
                i++;
            } while (buffer[i] >= '0' && buffer[i] <= '9');
        }
        /* skip more spaces */
        if (buffer[i] != ' ' && buffer[i] != '\t')
        {
            ret = false;
        }
        else
        {
            do
            {
                i++;
            } while (buffer[i] == ' ' || buffer[i] == '\t');
        }
        /* skip CLASS */
        if ((buffer[i] == 'i' || buffer[i] == 'I') &&
            (buffer[i+1] == 'n' || buffer[i+1] == 'N'))
        {
            i += 2;
        }
        else
        {
            ret = false;
        }
        /* skip more space*/
        if (buffer[i] != ' ' && buffer[i] != '\t')
        {
            ret = false;
        }
        else
        {
            do
            {
                i++;
            } while (buffer[i] == ' ' || buffer[i] == '\t');
        }
        /* check ds_bit */
        if ((buffer[i] == 'd' || buffer[i] == 'D') &&
            (buffer[i + 1] == 's' || buffer[i + 1] == 'S') &&
            (buffer[i + 2] == ' ' || buffer[i + 2] == '\t'))
        {
            *has_ds = true;
        }
    }

    return ret;
}

TldDSAsKey::TldDSAsKey(const char * name, size_t name_len, bool has_ds)
    :
    HashNext(NULL),
    ds_count((has_ds)?1:0),
    count(1),
    hash(0)
{
    if (name_len > 63)
    {
        name_len = 63;
    }
    for (size_t i = 0; i < name_len; i++)
    {
        int c = name[i];
        if (c >= 'a' && c <= 'z')
        {
            c = c + 'A' - 'a';
        }
        this->name[i] = c;
    }

    for (size_t i = name_len; i < 64; i++)
    {
        this->name[i] = 0;
    }
    this->name_len = name_len;
}

TldDSAsKey::~TldDSAsKey()
{
}

bool TldDSAsKey::IsSameKey(TldDSAsKey * key)
{
    return (key->name_len == this->name_len &&
        memcmp(key->name, this->name, key->name_len) == 0);
}

uint32_t TldDSAsKey::Hash()
{
    if (hash == 0)
    {
        uint32_t x = 0xC001CAFE;

        for (size_t i = 0; i < name_len; i++)
        {
            x = (x * 101) + name[i];
        }

        hash = x;
    }

    return hash;
}

TldDSAsKey * TldDSAsKey::CreateCopy()
{
    TldDSAsKey * x = new TldDSAsKey((char  const *)name, name_len, false);

    x->count = count;
    x->ds_count = ds_count;
    x->hash = hash;

    return x;
}

void TldDSAsKey::Add(TldDSAsKey * key)
{
    count += key->count;
    ds_count += key->ds_count;
}

ComputeM7::ComputeM7()
    :
    m71(0),
    m72(0)
{
}

ComputeM7::~ComputeM7()
{
}

bool ComputeM7::Load(char const * single_file_name)
{
    return m7Getter.GetM7(single_file_name);
}

bool ComputeM7::Compute()
{
    bool ret = true;

    if (m7Getter.nb_tld_queried > 0)
    {
        m71 = ((double)m7Getter.nb_ds_present) / ((double)m7Getter.nb_tld_queried);

        if (m7Getter.nb_cc_tld_queried > 0) {
            m72 = ((double)m7Getter.nb_cc_ds_present) / ((double)m7Getter.nb_cc_tld_queried);
        }
    }
    else
    {
        ret = false;
    }

    return ret;
}

bool ComputeM7::Write(FILE * F_out, char const* date, char const* version)
{
    bool ret = true;

    ret = (fprintf(F_out, "M7.1,%s,%s, , %6f,\n", date, version, m71) > 0);
    ret &= (fprintf(F_out, "M7.2,%s,%s, , %6f,\n", date, version, m72) > 0);

    return ret;
}
