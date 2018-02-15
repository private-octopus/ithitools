#ifndef ITHI_PUBLISHER_H
#define ITHI_PUBLISHER_H

#include <stdint.h>
#include <stdlib.h>
#include <vector>

typedef struct _metric_line
{
    char metric_name[64];
    char key_value[64];
    double frequency;
} MetricLine;

typedef struct _metric_file
{
    char file_name[256];
    int year;
    int month;
    int day;
    std::vector<MetricLine *> line;
} MetricFile;

class ithipublisher
{
public:
    ithipublisher();
    ~ithipublisher();

    bool CollectMetricFiles(char const * ithi_folder, char const * metric_id);
    bool GetDateFromFile(MetricFile * file, char const * metric_id);
    std::vector<MetricFile *> file;

    virtual bool ComputeData(char const * web_folder) = 0;
};

#endif /* ITHI_PUBLISHER_H */
