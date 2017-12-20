#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <vector>

enum M2DataType {
    Unknown = 0,
    TLD,
    Registrar,
    TLD_old
};

enum abuseType {
    Phishing = 0,
    Malware = 1,
    Spam = 2,
    Botnet = 3
};

typedef struct st_M2DataLine_t {
    char name[128];
    union {
        int RegistrarId;
        char TldType[64];
    };
    int Domains;
    int AbusiveDomains;
    double AbuseScore;
    int abuse_count[4];
    int NewlyAbusiveDomains;
    int LastMonthAbusiveDomains;
    double LastMonthAbuseScore;
} M2DataLine_t;

class M2Data
{
public:
    M2Data();
    ~M2Data();

    static bool IsSooner(M2Data * x, M2Data * y);
    static bool TldIsSmaller(M2DataLine_t x, M2DataLine_t y);
    static bool IsReservedRegistry(int registrar_id);

    bool Load(char const * monthly_csv_file_name);

    void Sort();

    bool Save();

    char const * get_file_name(char const * monthly_csv_file_path);
    bool parse_file_name(char const * monthly_csv_file_name);

    std::vector<M2DataLine_t> dataset;
    int year;
    int month;
    int day;
    M2DataType M2Type;
};

