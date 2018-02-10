#ifndef ITHI_METRICS_H
#define ITHI_METRICS_H

#include <time.h>
#include "CaptureSummary.h"
#include "M2Data.h"
#include "ComputeM34.h"
#include "ComputeM6.h"
#include "M7Getter.h"


class ithimetrics
{
public:
    ithimetrics();
    ~ithimetrics();

    bool GetMetrics();
    bool GetM7(char const * zone_file_name);

    bool SetIthiFolder(char const * folder);
    bool SetMetricFileNames(int metric_number, char const * metric_file_name);

    bool SetCaptureFileNames(int nb_files, char const ** file_names);
    bool SetDefaultCaptureFiles();
    uint32_t GetNbCaptureFiles() { return nb_capture_files; };
    const char * GetCaptureFileName(uint32_t file_index) {
        return (file_index < nb_capture_files) ? capture_file[file_index] : NULL;
    };

    bool SetDateString(char const * date_string);
    bool SetDefaultDate(time_t current_time);
    const char * GetMetricDate() { return (const char *)metric_date; };

    bool SetAbuseFileName(char const * abuse_file_name);
    bool SetDefaultAbuseFileName(time_t current_time);
    const char * GetAbuseFileName() { return (const char *)abuse_file_name; };

    bool SaveMetricFiles();

    bool Save(char const * file_name);

private:
    ComputeM2 cm2;
    ComputeM3 cm3;
    ComputeM4 cm4;
    ComputeM6 cm6;
    ComputeM7 cm7;

    char * metric_date;
    char * ithi_folder;
    char * metric_file[7];
    bool metric_is_available[7];
    uint32_t nb_capture_files;
    char ** capture_file;
    char * abuse_file_name;


    static bool copy_name(char ** target, char const * name);
};

#endif /* ITHI_METRICS_H */
