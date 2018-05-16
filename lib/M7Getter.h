#ifndef M7_GETTER_H
#define M7_GETTER_H

#include "ComputeMetric.h"

class TldDSAsKey
{
public:
    TldDSAsKey(const char * name, size_t name_len, bool has_ds);
    ~TldDSAsKey();

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
};

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


private:

    bool ParseRecord(char * buffer, char ** p_tld, size_t * tld_length, bool * has_ds);
};

class ComputeM7 : public ComputeMetric
{
public:
    ComputeM7();
    ~ComputeM7();

    bool Load(char const * single_file_name) override;
    bool LoadRecursiveCapture(char const * capture_file_name);
    bool Compute() override;
    bool Write(FILE * F_out) override;

private:
    M7Getter m7Getter;
    double m71;
    double m72;
    double m73;
    double m74;
};

#endif

