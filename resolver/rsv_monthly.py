# Summary of the monthly files.
#
# This is specialized for the organization of data on the compute server.
# It assumes that for a given month, we will have a set of folders 
# named flux-yyyy-mm-dd and recap-yyyy-mm-dd. In these folders,
# we find summary files flux-summary.csv or recap-summary.csv.
# There is one line per ISP, with different columns for the two files:
#
# recap_headers: CC,AS,start,uids,first_isp,
#     googlepdns,cloudflare,opendns,quad9,level3,neustar,he,
#     first_others,nb_https,nb_AAAA,nb_A,
#     A_ISP_only, A_PDNS_only, A_ISP_PDNS, A_others_only, A_ISP_others, A_PDNS_others, A_all3,
#     nb_A_ISP, nb_A_PDNS, nb_A_others,
#     zombies,z_ISP,z_PDNS,z_others,first_3s,first_10s,sum_delay,max_delay

# flux_header: CC,AS,start,uids,nb_isp,nb_pdns,nb_others,
#     nb_A,nb_A_isp,nb_A_pdns,nb_A_others,
#     nb_AAAA,nb_AAAA_isp,nb_AAAA_pdns,nb_AAAA_others,
#     nb_HTTPS,nb_HTTPS_isp,nb_HTTPS_pdns,nb_HTTPS_others
#
# We want to compute a monthly summary showing the total, as this
# would be the basis for the montly metrics, but we also want
# to compute "daily" versions of the metrics, so we can do daily graphs,
# and "ISP" versions of the metric, so we can draw histograms.
#
# So maybe we can compute two summary files for each or flux and recap:
#
# monthly-per-ISP summary: maybe add year and date columns, then same columns
#     as the daily summary, but showing the monthly total for the column
#     for this ISP.
# monthly-per-day summary: maybe add year and date columns, then same columns
#     as the daily summary, but showing the total for this day across all
#     ISP.
#

import pandas as pd
import sys
import os
import rsv_recap
import rsv_flux

def usage():
    print("Usage: python monthly.py results_dir year month")

class per_key_sum:
    def __init__(self, x, summed_columns, has_max_delay, has_average_delay, has_sum_delays):
        self.x = dict()

        self.summed_columns = summed_columns
        self.has_max_delay = has_max_delay
        self.has_average_delay = has_average_delay
        self.has_sum_delays = has_sum_delays
        for name in summed_columns:
            self.x[name] = x[name]
        if self.has_max_delay:
            self.x['max_delay'] = x['max_delay']
        if self.has_average_delay:
            self.x['average_delay'] = x['average_delay']
            if not self.has_sum_delays:
                self.x['sum_delays'] = self.x['uids']*self.x['average_delay']
                # if (self.x['uids'] > 0):
                #    print("Sum delays: " + str(self.x['sum_delays']) + " (" + str(self.x['uids']) + ", " + str(self.x['average_delay']))

    def add(self,x):
        for name in self.summed_columns:
            self.x[name] += x[name]
        if self.has_max_delay and x['max_delay'] > self.x['max_delay']:
            self.x['max_delay'] = x['max_delay']
        if self.has_average_delay and self.x['uids'] > 0:
            if not self.has_sum_delays:
                self.x['sum_delays'] += x['uids']*x['average_delay']
            self.x['average_delay'] = self.x['sum_delays'] / self.x['uids']
        
    def get_row(self):
        row = []
        for name in self.summed_columns:
            row.append(self.x[name])
        if self.has_average_delay:
            row.append(self.x['average_delay'])
        if self.has_max_delay:
            row.append(self.x['max_delay'])
        return row

class summaries:
    def __init__(self, summary_columns):
        self.summed_columns = []
        self.has_max_delay = False
        self.has_average_delay = False
        self.has_sum_delays = False
        self.monthly_per_isp = dict()
        self.monthly_per_day = dict()
        #print(str(summary_columns))
        self.get_columns(summary_columns)
        #print(str(self.summed_columns))


    def get_columns(self, summary_columns):
        header_set = set(["CC", "AS", 'start'])
    
        for c in summary_columns:
            if not c in header_set:
                if c == 'max_delay':
                    self.has_max_delay=True
                elif c == 'average_delay':
                    self.has_average_delay=True
                else:
                    self.summed_columns.append(c)
                    if c == 'sum_delays':
                        self.has_sum_delays = True
        

    def add_cc_as(self, row):
        cc = row['CC']
        if not isinstance(cc, str) or len(cc) != 2:
            cc = '  '
        asn = row['AS']
        if not isinstance(asn, str) or not asn.startswith('AS'):
            asn = 'AS0'
        key = cc + '-' + asn

        if not key in self.monthly_per_isp:
            self.monthly_per_isp[key] = per_key_sum(row, self.summed_columns, self.has_max_delay, self.has_average_delay, self.has_sum_delays)
        else:
            self.monthly_per_isp[key].add(row)

    def add_daily(self, row, year, month, day):
        key = str(year) + '-' + str(month) + '-' + str(day)
        if not key in self.monthly_per_day:
            self.monthly_per_day[key] = per_key_sum(row, self.summed_columns, self.has_max_delay, self.has_average_delay, self.has_sum_delays)
        else:
            self.monthly_per_day[key].add(row)

    def add_file(self, year, month, day, file_path):
        df = pd.read_csv(file_path)
        df.apply(lambda row: self.add_cc_as(row),axis=1)
        df.apply(lambda row: self.add_daily(row, year, month, day),axis=1)
        #print("After loading " + file_path + ":")
        #print("ISP: " + str(len(self.monthly_per_isp)))
        #print("Days: " + str(len(self.monthly_per_day)))

    def load_day(self, month_dir, file_name, year, month):
        day_dir = ""
        if file_name[-3] == '-':
            day = file_name[-2:]
            try:
                day_num = int(day)
            except:
                day_num = -1
            if day_num < 0 or day_num > 31:
                print("Bad day file name: " + day + " (" + file_name + ")")
            else:
                day_dir = os.path.join(month_dir, file_name)
                if not os.path.isdir(day_dir):
                    print("Not a directory: " + day_dir)
                else:
                    summary_files = [ f for f in os.listdir(day_dir) if f.endswith("-summary.csv") ]
                    if len(summary_files) != 1:
                        print ("Found " + str(len(summary_files)) + " summaries in " + day_dir)
                    else:
                        file_path = os.path.join(day_dir, summary_files[0])
                        self.add_file(year, month, day, file_path)

    def save(self, year, month, month_dir, table_name):
        # the isp file contains one row per ISP.
        # Each row starts with CC, AS, Year, Month, then the columns
        # the day file contains one row per day.
        # Each row starts with Year, Month, Day, then the columns
        isp_path = os.path.join(month_dir, table_name + '-monthly-isp.csv')
        day_path = os.path.join(month_dir, table_name + '-monthly-days.csv')

        isp_columns = [ 'year', 'month', 'CC', 'AS']
        day_columns = [ 'year', 'month', 'day']
        isp_columns += self.summed_columns
        day_columns += self.summed_columns
        if self.has_average_delay:
            isp_columns.append('average_delay')
            day_columns.append('average_delay')
        if self.has_max_delay:
            isp_columns.append('max_delay')
            day_columns.append('max_delay')
        #print("ISP-columns: " + str(isp_columns))
        #print("Day-columns: " + str(day_columns))

        isp_t = []
        for key in self.monthly_per_isp:
            p = key.split('-')
            row = [ year, month, p[0], p[1] ] + self.monthly_per_isp[key].get_row()
            isp_t.append(row)
        isp_df = pd.DataFrame(isp_t,columns=isp_columns)
        isp_df.sort_values(['year', 'month', 'uids'], ascending=[True, True, False])
        isp_df.to_csv(isp_path)

        day_t = []
        for key in self.monthly_per_day:
            row = [ year, month, key[-2:] ] + self.monthly_per_day[key].get_row()
            day_t.append(row)
        day_df = pd.DataFrame(day_t,columns=day_columns)
        day_df.sort_values(['year', 'month', 'day'], ascending=[True, True, True])
        day_df.to_csv(day_path)




# main
if len(sys.argv) != 4:
    usage()
    exit(1)
results_dir = sys.argv[1]
year = sys.argv[2]
month = sys.argv[3]

year_dir = os.path.join(results_dir, year)
month_dir = os.path.join(year_dir, month)
if not os.path.isdir(month_dir):
    print("not a valid directory: " + month_dir)
    usage()
    exit(1)

recap_summaries = summaries(rsv_recap.recap_cc_as2.summary_columns())
flux_summaries = summaries(rsv_flux.flux_cc_as2.get_columns())


recap_prefix = "recap-" + year + "-" + month + "-"
flux_prefix = "flux-" + year + "-" + month + "-"
m_list = os.listdir(month_dir)
recap_dirs = [f for f in m_list if f.startswith(recap_prefix)]
flux_dirs = [f for f in m_list if f.startswith(flux_prefix)]

for dir_name in recap_dirs:
    recap_summaries.load_day(month_dir, dir_name, year, month)
recap_summaries.save(year, month, month_dir, 'recap')

for dir_name in flux_dirs:
    flux_summaries.load_day(month_dir, dir_name, year, month)
flux_summaries.save(year, month, month_dir, 'flux')

