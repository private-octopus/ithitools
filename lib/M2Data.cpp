#include <algorithm>
#include <string.h>
#include "CsvHelper.h"
#include "M2Data.h"

M2Data::M2Data()
    :
    year(0),
    month(0),
    M2Type(M2DataType::Unknown)
{
}


M2Data::~M2Data()
{
}

bool M2Data::IsSooner(M2Data * x, M2Data * y)
{
    return (x->year < y->year || (x->year == y->year && x->month == y->month));
}

bool M2Data::TldIsSmaller(M2DataLine_t x, M2DataLine_t y)
{
    return (strcmp(x.name, y.name) < 0);
}

bool M2Data::Load(char const * monthly_csv_file_name)
{
    FILE* F;
    M2DataLine_t line;
    char buffer[512];

#ifdef _WINDOWS
    errno_t err = fopen_s(&F, monthly_csv_file_name, "r");
    bool ret = (err == 0 && F != NULL);
#else
    bool ret;
    F = fopen(monthly_csv_file_name, "r");
    ret = (F != NULL);
#endif

    while (ret && fgets(buffer, sizeof(buffer), F))
    {
        int start = 0;
        memset(&line, 0, sizeof(M2DataLine_t));

        if (M2Type == M2DataType::TLD_old)
        {
            start = CsvHelper::read_string(line.name + 1, sizeof(line.name) - 1, start, buffer, sizeof(buffer));
            if (line.name[1] == 0)
            {
                line.name[0] = 0;
            }
            else
            {
                line.name[0] = '.';
            }
        }
        else
        {
            start = CsvHelper::read_string(line.name, sizeof(line.name), start, buffer, sizeof(buffer));
        }
        switch (M2Type)
        {
        case TLD:
            start = CsvHelper::read_string(line.TldType, sizeof(line.TldType), start, buffer, sizeof(buffer));
            break;
        case Registrar:
            start = CsvHelper::read_number(&line.RegistrarId, start, buffer, sizeof(buffer));
            break;
        case TLD_old:
            /* Nothing there */
            break;
        default:
            /* Assume TLD_old if default */
            break;
        }
        start = CsvHelper::read_number(&line.Domains, start, buffer, sizeof(buffer));
        start = CsvHelper::read_number(&line.AbusiveDomains, start, buffer, sizeof(buffer));
        start = CsvHelper::read_double(&line.AbuseScore, start, buffer, sizeof(buffer));

        for (int i = 0; i < 4; i++)
        {
            start = CsvHelper::read_number(&line.abuse_count[i], start, buffer, sizeof(buffer));
        }
        start = CsvHelper::read_number(&line.NewlyAbusiveDomains, start, buffer, sizeof(buffer));
        start = CsvHelper::read_number(&line.LastMonthAbusiveDomains, start, buffer, sizeof(buffer));
        start = CsvHelper::read_double(&line.LastMonthAbuseScore, start, buffer, sizeof(buffer));

        /* TODO: check that the parsing is good */
        if (line.name[0] != 0 && line.Domains != 0)
        {
            /* allocate data and add to vector */
            dataset.push_back(line);
        }
    }

    if (F != NULL)
    {
        fclose(F);
    }
    return ret;
}

void M2Data::Sort()
{
    std::sort(dataset.begin(), dataset.end(), M2Data::TldIsSmaller);
}

bool M2Data::Save()
{
    char file_name[256];
    FILE* F;
    bool ret = false;

    if (0 > snprintf(file_name, sizeof(file_name), "M2-%4d-%2d-%2d-%s.csv",
        year, month, day, (M2Type == M2DataType::Registrar) ? "-registrars" : "tlds"))
    {
        ret = false;
    }

    if (ret)
    {
#ifdef _WINDOWS
        errno_t err = fopen_s(&F, file_name, "w");
        ret = (err == 0 && F != NULL);
#else
        F = fopen(file_name, "w");
        ret = (F != NULL);
#endif
    }

    if (!ret)
    {
        printf("Cannot save <%s>\n", file_name);
    }

    for (size_t i = 0; i < dataset.size(); i++)
    {
        fprintf(F, "%s,", (M2Type == M2DataType::Registrar) ? "Registrar" : "TLD");
        fprintf(F, "\"%s\",", dataset[i].name);
        fprintf(F, "%d,", dataset[i].Domains);
        for (int j = 0; j < 4; j++)
        {
            fprintf(F, "%d,", dataset[i].abuse_count[j]);
        }

        fprintf(F, "\n");
    }

    if (F != NULL)
    {
        fclose(F);
    }

    if (ret)
    {
        printf("Saved <%s>, %d lines.\n", file_name, (int)dataset.size() );
    }

    return ret;
}

/*
 * Expect names of the form:
 *      2017-sept-tlds.csv
 *      2017-10-31_registrars.csv
 *      2017-10-30_tlds.csv
 */
static char const * month_names[12] = {
    "jan", "feb", "mar", "apr", "may", "june", "july", "aug", "sept", "oct", "nov", "dec"
};

static const int month_day[12] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static char const * file_suffix[3] = { "_tlds.csv", "_registrars.csv", "-tlds.csv" };
static M2DataType const file_type[3] = { M2DataType::TLD, M2DataType::Registrar, M2DataType::TLD_old };

#ifdef _WINDOWS
static int file_sep = '\\';
#else
static int file_sep = '/';
#endif


char const * M2Data::get_file_name(char const * monthly_csv_file_path)
{
    char const * start = monthly_csv_file_path;
    char const * x = monthly_csv_file_path;

    while (*x)
    {
        if (*x == file_sep)
        {
            start = x + 1;
        }
        x++;
    }

    return start;
}

bool M2Data::parse_file_name(char const * monthly_csv_file_name)
{
    bool ret = true;
    size_t len = strlen(monthly_csv_file_name);
    size_t char_index = 0;
    int y = 0;
    int m = 0;
    int d = 0;
    int suffix_len = 0;

    if (len < 4)
    {
        ret = false;
    }
    else
    {
        for (int i = 0; i < 3; i++)
        {
            size_t sl = strlen(file_suffix[i]);
            if (sl < len && strcmp(monthly_csv_file_name + len - sl, file_suffix[i]) == 0)
            {
                M2Type = file_type[i];
                suffix_len = (int) sl;
                break;
            }
        }

        if (M2Type == Unknown)
        {
            ret = false;
        }

        for (int i = 0; ret && i < 4; i++)
        {
            char c = monthly_csv_file_name[char_index++];

            if (c < '0' || c > '9')
            {
                ret = false;
            }
            else
            {
                y *= 10;
                y += c - '0';
            }
        }

        if (y < 2017 || y > 2057)
        {
            ret = false;
        }
    }

    if (ret && monthly_csv_file_name[char_index++] != '-')
    {
        ret = false;
    }

    if (ret)
    {
        if (M2Type == TLD_old)
        {
            for (int i = 0; i < 12; i++)
            {
                size_t ml = strlen(month_names[i]);
                if ((char_index + ml) < len &&
                    strncmp(&monthly_csv_file_name[char_index], month_names[i], ml) == 0)
                {
                    m = i+1;
                    char_index += ml;
                    break;
                }
            }


            if (m <= 0)
            {
                ret = false;
            }
            else
            {
                d = month_day[m-1];
            }
        }
        else
        {
            for (int i = 0; ret && i < 2; i++)
            {
                char c = monthly_csv_file_name[char_index++];

                if (c < '0' || c > '9')
                {
                    ret = false;
                }
                else
                {
                    m *= 10;
                    m += c - '0';
                }
            }

            if (m <= 0 || m > 12)
            {
                ret = false;
            }

            if (ret && monthly_csv_file_name[char_index++] != '-')
            {
                ret = false;
            }

            for (int i = 0; ret && i < 2; i++)
            {
                char c = monthly_csv_file_name[char_index++];

                if (c < '0' || c > '9')
                {
                    ret = false;
                }
                else
                {
                    d *= 10;
                    d += c - '0';
                }
            }

            if (d <= 0 || d > 31)
            {
                ret = false;
            }
        }
    }

    if (ret && char_index+suffix_len != len)
    {
        ret = false;
    }

    if (ret)
    {
        year = y;
        month = m;
        day = d;
    }

    return ret;
}
