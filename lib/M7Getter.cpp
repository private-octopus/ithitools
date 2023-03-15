
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
    for (size_t i = 0; i < algo_count.size(); i++)
    {
        if (algo_count[i] != NULL)
        {
            delete algo_count[i];
            algo_count[i] = NULL;
        }
    }
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
            uint32_t algo_code = 0;

            if (ParseRecord(buffer, &tld, &tld_length, &has_ds, &algo_code))
            {
                TldDSAsKey key(tld, tld_length, has_ds, algo_code);
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

                if (key->algo_nb > 0) {
                    AddAlgoFrequencies(key);
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
 * We are only concerned with the name, the record type, and for DS records the algorithm number.
 */

bool M7Getter::ParseRecord(char * buffer, char ** p_tld, size_t * tld_length, bool * has_ds, uint32_t* algo_code)
{
    bool ret = true;
    size_t i = 0;
    int x;
    int nb_dots = 0;


    *p_tld = buffer;
    *tld_length = 0;
    *has_ds = false;
    *algo_code = 0;

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
            i += 2;
            /* skip space until next number */
            do
            {
                i++;
            } while (buffer[i] == ' ' || buffer[i] == '\t');

            /* skip digits (key ID) */
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
            /* Skip more spaces */
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
            /* Compute algorithm number */
            if (buffer[i] < '0' || buffer[i] > '9')
            {
                ret = false;
            }
            else while (buffer[i] >= '0' && buffer[i] <= '9')
            {
                if (*algo_code > 0x1FFFFFF) {
                    ret = false;
                    break;
                }
                *algo_code *= 10;
                *algo_code += (uint32_t)(buffer[i] - '0');
                i++;
            }
        }
    }

    return ret;
}

bool M7Getter::AddAlgoFrequencies(TldDSAsKey* key)
{
    bool ret = true;
    double total = 0;
    bool is_cc = (key->name_len == 2);

    for (uint32_t i = 0; i < key->algo_nb; i++) {
        total += key->algo_count[i];
    }
    for (uint32_t i = 0; i < key->algo_nb; i++) {
        bool found = false;
        double frequency = ((double)key->algo_count[i]) / total;
        for (size_t j = 0; j < algo_count.size(); j++) {
            if (algo_count[j]->algo_code == key->algo_code[i]) {
                algo_count[j]->algo_frequency += frequency;
                if (is_cc) {
                    algo_count[j]->algo_frequency_cc += frequency;
                }
                found = true;
                break;
            }
        }
        if (!found) {
            m7_algo_count_t * algo_val = new m7_algo_count_t;
                
            algo_val->algo_code = key->algo_code[i];
            algo_val->algo_frequency = frequency;
            algo_val->algo_frequency_cc = (is_cc) ? frequency : 0.0;
            this->algo_count.push_back(algo_val);
        }
    }

    return ret;
}

TldDSAsKey::TldDSAsKey(const char * name, size_t name_len, bool has_ds, uint32_t algo_code)
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

    if (has_ds) {
        this->algo_nb = 1;
        this->algo_code[0] = algo_code;
        this->algo_count[0] = 1;
    }
    else {
        this->algo_nb = 0;
    }
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
    TldDSAsKey * x = new TldDSAsKey((char  const *)name, name_len, false, 0);

    x->count = count;
    x->ds_count = ds_count;
    x->hash = hash;
    x->algo_nb = algo_nb;
    for (uint32_t i = 0; i < algo_nb; i++) {
        x->algo_code[i] = algo_code[i];
        x->algo_count[i] = algo_count[i];
    }
    return x;
}

void TldDSAsKey::Add(TldDSAsKey * key)
{
    count += key->count;
    ds_count += key->ds_count;
    for (uint32_t i = 0; i < key->algo_nb; i++) {
        bool is_found = false;
        for (uint32_t j = 0; j < algo_nb; j++) {
            if (algo_code[j] == key->algo_code[i]) {
                algo_count[j] += key->algo_count[i];
                is_found = true;
                break;
            }
        }
        if (!is_found && algo_nb < 8) {
            algo_code[algo_nb] = key->algo_code[i];
            algo_count[algo_nb] = key->algo_count[i];
            algo_nb++;
        }
    }
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

    for (size_t j = 0; ret && j < m7Getter.algo_count.size(); j++) {
        double frequency = m7Getter.algo_count[j]->algo_frequency / ((double)m7Getter.nb_tld_queried);
        ret &= (fprintf(F_out, "M7.3,%s,%s, %u, %6f,\n", date, version, 
            m7Getter.algo_count[j]->algo_code, frequency) > 0);
    }

    for (size_t j = 0; ret && j < m7Getter.algo_count.size(); j++) {
        double frequency = m7Getter.algo_count[j]->algo_frequency_cc / ((double)m7Getter.nb_cc_tld_queried);
        ret &= (fprintf(F_out, "M7.4,%s,%s, %u, %6f,\n", date, version, 
            m7Getter.algo_count[j]->algo_code, frequency) > 0);
    }

    return ret;
}
