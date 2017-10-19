#ifndef ITHI_METRICS_H
#define ITHI_METRICS_H
#include "CaptureSummary.h"

typedef struct _st_metric4_line {
    char domain[64];
    double frequency;
} metric4_line;

class ithimetrics
{
public:
    ithimetrics();
    ~ithimetrics();

    bool GetMetrics(CaptureSummary* cs);

    bool Save(char const * file_name);

private:
    uint32_t nb_rootqueries;

    double m3_1;
    double m3_2;
    std::vector<metric4_line> m4_1;
    std::vector<metric4_line> m4_2;
    std::vector<metric4_line> m4_3;

    void GetM3_1(CaptureSummary* cs);
    void GetM3_2(CaptureSummary* cs);
    void GetM4_1(CaptureSummary* cs);
    void GetM4_2(CaptureSummary* cs);
    void GetM4_3(CaptureSummary* cs);
    void GetM4_X(CaptureSummary* cs, uint32_t table_id, std::vector<metric4_line> * m4_x);

    static bool metric4_line_is_bigger(metric4_line x, metric4_line y);
};
#endif
