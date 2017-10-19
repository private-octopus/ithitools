#include <stdio.h>
#include <algorithm>
#include "DnsStats.h"
#include "ithimetrics.h"



ithimetrics::ithimetrics()
    :
    nb_rootqueries(0),
    m3_1(0),
    m3_2(0)
{
}


ithimetrics::~ithimetrics()
{
}

bool ithimetrics::GetMetrics(CaptureSummary * cs)
{
    bool ret = true;
    
    GetM3_1(cs);
    GetM3_2(cs);
    GetM4_1(cs);
    GetM4_2(cs);
    GetM4_3(cs);

    return ret;
}

bool ithimetrics::Save(char const * file_name)
{
    FILE* F;

#ifdef _WINDOWS
    errno_t err = fopen_s(&F, file_name, "w");
    bool ret = (err == 0);
#else
    bool ret;
    F = fopen(file_name, "w");
    ret = (F != NULL);
#endif

    if (ret)
    {
        fprintf(F, "M3.1, , %6f,\n", m3_1);
        fprintf(F, "M3.2, , %6f,\n", m3_2);

        for (size_t i = 0; i < m4_1.size(); i++)
        {
            fprintf(F, "M4.1, %s, %6f,\n", m4_1[i].domain, m4_1[i].frequency);
        }

        for (size_t i = 0; i < m4_2.size(); i++)
        {
            fprintf(F, "M4.2, %s, %6f,\n", m4_2[i].domain, m4_2[i].frequency);
        }

        for (size_t i = 0; i < m4_3.size(); i++)
        {
            fprintf(F, "M4.3, %s, %6f,\n", m4_3[i].domain, m4_3[i].frequency);
        }
    }

    if (F != NULL)
    {
        fclose(F);
    }

    return ret;
}

void ithimetrics::GetM3_1(CaptureSummary * cs)
{
    uint32_t nb_noerror = cs->GetCountByNumber(
        DnsStats::GetTableName(REGISTRY_DNS_root_QR), 0);
    uint32_t nb_nxdomain = cs->GetCountByNumber(
        DnsStats::GetTableName(REGISTRY_DNS_root_QR), 3);
    nb_rootqueries = nb_noerror + nb_nxdomain;

    if (nb_rootqueries > 0)
    {
        m3_1 = (double)nb_nxdomain;
        m3_1 /= (double)nb_rootqueries;
    }
    else
    {
        m3_1 = 0;
    }
}

void ithimetrics::GetM3_2(CaptureSummary * cs)
{
    uint32_t nb_useful = cs->GetCountByNumber(
        DnsStats::GetTableName(REGISTRY_DNS_UsefulQueries), 1);
    uint32_t nb_useless = cs->GetCountByNumber(
        DnsStats::GetTableName(REGISTRY_DNS_UsefulQueries), 0);
    uint32_t total = nb_useful + nb_useless;

    if (total > 0)
    {
        m3_2 = (double)nb_useless;
        m3_2 /= (double)total;
    }
    else
    {
        m3_2 = 0;
    }
}

void ithimetrics::GetM4_1(CaptureSummary * cs)
{
    GetM4_X(cs, REGISTRY_DNS_RFC6761TLD, &m4_1);
}

void ithimetrics::GetM4_2(CaptureSummary * cs)
{
    GetM4_X(cs, REGISTRY_DNS_LeakedTLD, &m4_2);
}

void ithimetrics::GetM4_3(CaptureSummary * cs)
{
    if (nb_rootqueries > 0)
    {
        std::vector<CaptureLine *> extract;

        cs->Extract(
            DnsStats::GetTableName(REGISTRY_DNS_LeakByLength), &extract);
        m4_3.reserve(extract.size());

        for (size_t i = 0; i < extract.size(); i++)
        {
            metric4_line line;
            size_t indx = 0;

            line.domain[indx++] = 'l';
            line.domain[indx++] = 'e';
            line.domain[indx++] = 'n';
            line.domain[indx++] = 'g';
            line.domain[indx++] = 't';
            line.domain[indx++] = 'h';
            line.domain[indx++] = '_';

#ifdef _WINDOWS
            (void) _itoa_s(extract[i]->key_number, &line.domain[indx],
                sizeof(line.domain) - indx, 10);
#else
            (void)itoa(extract[i]->key_number, &line.domain[indx], 10);
#endif
            line.frequency = ((double)extract[i]->count) / ((double)nb_rootqueries);

            if (extract.size() < 8 || line.frequency >= 0.001)
            {
                m4_3.push_back(line);
            }
        }

        std::sort(m4_3.begin(), m4_3.end(), metric4_line_is_bigger);
    }
}

void ithimetrics::GetM4_X(CaptureSummary * cs, uint32_t table_id, std::vector<metric4_line>* m4_x)
{
    if (nb_rootqueries > 0)
    {
        std::vector<CaptureLine *> extract;

        cs->Extract(
            DnsStats::GetTableName(table_id), &extract);
        m4_x->reserve(extract.size());

        for (size_t i = 0; i < extract.size(); i++)
        {
            metric4_line line;

            line.domain[strlen(extract[i]->key_value)] = 0;
            line.frequency = ((double)extract[i]->count) / ((double)nb_rootqueries);

            if (extract.size() < 8 || line.frequency >= 0.001)
            {
                memcpy(line.domain, extract[i]->key_value, strlen(extract[i]->key_value));
                m4_x->push_back(line);
            }
        }

        std::sort(m4_x->begin(), m4_x->end(), metric4_line_is_bigger);
    }
}

bool ithimetrics::metric4_line_is_bigger(metric4_line x, metric4_line y)
{
    return (x.frequency > y.frequency);
}
