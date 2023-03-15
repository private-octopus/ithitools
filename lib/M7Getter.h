#ifndef M7_GETTER_H
#define M7_GETTER_H

#include "ComputeMetric.h"
#include <vector>

class TldDSAsKey
{
public:
    TldDSAsKey(const char * name, size_t name_len, bool has_ds, uint32_t algo_code);
    virtual ~TldDSAsKey();

    bool IsSameKey(TldDSAsKey* key);
    uint32_t Hash();
    virtual TldDSAsKey* CreateCopy();
    void Add(TldDSAsKey* key);

    TldDSAsKey * HashNext;

    size_t name_len;
    uint8_t name[64];
    uint32_t ds_count;
    uint32_t count;
    uint32_t hash;
    uint32_t algo_nb;
    uint32_t algo_code[8];
    uint32_t algo_count[8];
};


typedef struct _st_m7_algo_count_t {
    uint32_t algo_code;
    double algo_frequency;
    double algo_frequency_cc;
} m7_algo_count_t;

class M7Getter
{
public:
    M7Getter();
    ~M7Getter();

    bool GetM7(char const * root_zone_file_name);
    int nb_tld_queried;
    int nb_ds_present;
    int nb_cc_tld_queried;
    int nb_cc_ds_present;
    std::vector<m7_algo_count_t *> algo_count;

private:
    bool ParseRecord(char * buffer, char ** p_tld, size_t * tld_length, bool * has_ds, uint32_t* algo_code);
    bool AddAlgoFrequencies(TldDSAsKey * key);
};

class ComputeM7 : public ComputeMetric
{
public:
    ComputeM7();
    ~ComputeM7();

    bool Load(char const * single_file_name) override;
    bool Compute() override;
    bool Write(FILE * F_out, char const* date, char const* version) override;

private:
    M7Getter m7Getter;
    double m71;
    double m72;
};

#endif

