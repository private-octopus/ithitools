#ifndef ITHI_METRICS_H
#define ITHI_METRICS_H

#include <time.h>
#include "CaptureSummary.h"

class M2Data;

typedef struct _st_metric4_line_t {
    char domain[64];
    double frequency;
} metric4_line_t;

typedef struct _st_metric6_registered_t {
    uint32_t key;
    char const * key_name;
} metric6_registered_t;

typedef struct _st_metric6_parameter {
    uint32_t parameter_value;
    uint32_t parameter_count;
} metric6_parameter_t;

typedef struct _st_metric6_def_t {
    char const * m6_prefix;
    uint32_t table_id;
    uint32_t nb_registered;
    metric6_registered_t * registry;
    char const * iana_file_name;
} metric6_def_t;

typedef struct _st_metric6_line_t {
    char const * m6_prefix;
    double m6_x_1;
    double m6_x_2;
    std::vector<metric6_parameter_t> m6_x_3;
} metric6_line_t;

class ithimetrics
{
public:
    ithimetrics();
    ~ithimetrics();

    bool GetMetrics(CaptureSummary* cs);
    bool GetM7(char const * zone_file_name);

    bool SetIthiFolder(char const * folder);
    bool SetMetricFileNames(int metric_number, char const * metric_file_name);

    bool SetCaptureFileNames(int nb_files, char const ** file_names);
    bool SetDefaultCaptureFiles();
    uint32_t GetNbCaptureFiles() { return nb_capture_files; };
    const char * GetCaptureFileName(uint32_t file_index) { 
        return (file_index < nb_capture_files)?capture_file[file_index]:NULL; };

    bool SetDateString(char const * date_string);
    bool SetDefaultDate(time_t current_time);
    const char * GetMetricDate() { return (const char *)metric_date; };

    bool SetAbuseFileName(char const * abuse_file_name);
    bool SetDefaultAbuseFileName(time_t current_time);
    const char * GetAbuseFileName() { return (const char *)abuse_file_name; };

    bool SaveMetricFiles();

    bool Save(char const * file_name);

private:
    uint32_t nb_rootqueries;
    uint32_t nb_userqueries;
    uint32_t nb_nondelegated;
    uint32_t nb_delegated;
    char * metric_date;
    char * ithi_folder;
    char * metric_file[7];
    bool metric_is_available[7];
    uint32_t nb_capture_files;
    char ** capture_file;
    char * abuse_file_name;
    M2Data * m2_data;

    double m2_1234[4];
    double m3_1;
    double m3_2;
    std::vector<metric4_line_t> m33_1;
    std::vector<metric4_line_t> m33_2;
    std::vector<metric4_line_t> m33_3;
    double m33_4;
    double m4_1;
    std::vector<metric4_line_t> m4_2;
    std::vector<metric4_line_t> m4_3;
    std::vector<metric6_line_t> m6;
    double m7;

    bool SaveM1(FILE * F);
    bool SaveM2(FILE * F);
    bool SaveM3(FILE * F);
    bool SaveM4(FILE * F);
    bool SaveM5(FILE * F);
    bool SaveM6(FILE * F);
    bool SaveM7(FILE * F);

    void GetM2();
    void GetM3_1(CaptureSummary* cs);
    void GetM3_2(CaptureSummary* cs);
    void GetM33_1(CaptureSummary* cs);
    void GetM33_2(CaptureSummary* cs);
    void GetM33_3(CaptureSummary* cs);
    void GetM4_1(CaptureSummary* cs);
    void GetM4_2(CaptureSummary* cs);
    void GetM4_3(CaptureSummary* cs);
    void GetM3_X(CaptureSummary* cs, uint32_t table_id,
        std::vector<metric4_line_t> * mstring_x, double min_share);
    void GetM4_X(CaptureSummary* cs, uint32_t table_id,
        std::vector<metric4_line_t> * mstring_x, double min_share);
    void GetStringM_X(CaptureSummary* cs, uint32_t table_id, 
        std::vector<metric4_line_t> * mstring_x, uint32_t nbqueries,double min_share);
    void GetM6(CaptureSummary* cs);

    static bool metric4_line_is_bigger(metric4_line_t x, metric4_line_t y);
    static bool copy_name(char ** target, char const * name);
};
#endif
