#ifndef CAPTURE_SUMMARY_H
#define CAPTURE_SUMMARY_H

#include <stdint.h>
#include <vector>

typedef struct _capture_line
{
    char registry_name[64];
    int key_type;
    union
    {
        int key_number;
        char key_value[64];
    };
    int count;
} CaptureLine;

class CaptureSummary
{
public:
    CaptureSummary();
    ~CaptureSummary();

    bool Load(char const * file_name);

    bool Save(char const * file_name);

    bool Compare(CaptureSummary * x);

    bool AddLine(CaptureLine * line, bool need_allocation);

    void Reserve(size_t target_size);

    void Sort();

    size_t Size();

    uint32_t GetCountByNumber(char const * table_name, uint32_t number);

    void Extract(char const * table_name, std::vector<CaptureLine *> *extract);

    bool Merge(size_t nb_files, char const ** file_name);
    bool Merge(size_t nb_summaries, CaptureSummary ** cs);

private:
    std::vector<CaptureLine *> summary;
    int read_number(int* number, size_t start, char * buffer, size_t buffer_max);
    int read_string(char* text, int text_max, size_t start, char * buffer, size_t buffer_max);

    static int compare_string(char const * x, char const * y);
    static bool CaptureLineIsLower(CaptureLine * x, CaptureLine * y);
    static bool CaptureLineIsSameKey(CaptureLine * x, CaptureLine * y);
};

#endif

