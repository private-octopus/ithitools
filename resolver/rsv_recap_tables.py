# Recapitulate the tables per CC/AS.
# Produce one file for all CC/AS that have more that 1000 commands per day
# 

import sys
import os
from pathlib import Path
import ip2as
import rsv_log_parse
#import rsv_both_graphs
import pandas as pd
import traceback
import top_as
import time
import bz2
from rsv_delay_class import delay_query_as
import rsv_arguments 


def usage():
    print("Usage: python rsv_recap_tables.py <output_dir> <recap_metrics_files>\n")
    print("This script will parse the files created by rsv_recap_metrics,")
    print("find all slices for a given CC/AS combination, and then create")
    print("in the output directory a file for each CC/AS that has > 10,000")
    print("events")

recap_columns = [ 
    'CC', 'AS', 'start', 'uids', 'first_isp',
    'googlepdns', 'cloudflare', 'opendns', 'quad9', 'level3', 'neustar', 'he',
    'first_others', 'nb_https',
    'nb_A', 'dups_isp', 'dups_pdns', 'dups_both', 'dups_others', 'dups_long',
    'zombie_1', 'zombie2',
    'sum_delay', 'max_delay'
]

recap_first_columns =  [ 
    'CC', 'AS', 'start', 'uids', 'first_isp' ]

recap_final_columns = [ 'nb_https',
    'nb_A', 'dups_isp', 'dups_pdns', 'dups_both', 'dups_others', 'dups_long',
    'zombie_1', 'zombie2', 'sum_delay', 'max_delay' ]

recap_final_columns_corrected = [ 'nb_https',
    'nb_A', 'dups_isp', 'dups_pdns', 'dups_both', 'dups_others', 'dups_long',
    'zombie_1', 'zombie_2', 'sum_delay', 'max_delay' ]

recap_pdns = [
    'googlepdns', 'cloudflare', 'opendns', 'quad9', 'level3', 'neustar', 'he' ]

class recap_cc_as:
    def __init__(self, query_cc, query_AS):
        self.query_cc = query_cc
        self.query_AS = query_AS
        self.slices = dict()
        self.total_uids = 0
        self.total_pdns = [ 0, 0, 0, 0, 0, 0, 0 ]
        self.top_pdns = [ 0, 1, 2 ]
        self.skipped_pdns = [ ]

    def add_row(self, row):
        self.total_uids += row['uids']
        start = row['start']
        if not start in self.slices:
            self.slices[start] = row
        else:
            for col in recap_columns[3:-1]:
                self.slices[start][col] += row[col]
            if self.slices[start]['max_delay'] < row['max_delay']:
                self.slices[start]['max_delay'] = row['max_delay']

    def evaluate(self):
        self.total_pdns = [ 0, 0, 0, 0, 0, 0, 0 ]
        self.top_pdns = [ 0, 1, 2 ]
        self.skipped_pdns = [ ]
        for start in self.slices:
            for i in range(0, 7):
                self.total_pdns[i] += self.slices[start][recap_pdns[i]]
        t = []
        for i in range(0, 7):
            t.append([i, self.total_pdns[i]])
        t.sort(key=lambda x:x[1], reverse=True)
        for i in range(0,3):
            x = t[i]
            self.top_pdns[i] = x[0]
        top_set = set(self.top_pdns)
        for i in range(0,7):
            if not i in top_set:
                self.skipped_pdns.append(i)

    def get_columns(self):
        columns = []
        columns += recap_first_columns
        for i in range(0,3):
            columns.append(recap_pdns[self.top_pdns[i]])
        columns.append('first_others')
        columns += recap_final_columns_corrected
        columns.append('average_delay')
        return columns

    def save_file(self, file_name):
        self.evaluate()

        with open(file_name, "wt") as F:
            headers = self.get_columns()
            s = ""
            for header in headers:
                s += header + ","
            s += "\n"
            F.write(s)
            slice_list = list(self.slices.keys())
            slice_list.sort()
            for start in slice_list:
                row = self.slices[start]
                s = ""
                for header in recap_first_columns:
                    try:
                        s += str(row[header]) + ","
                    except:
                        print("First column: " + header + ", s: " + s)
                        print("Row[" + str(start) + "] has " + str(len[row]) + " elements.")
                        F.flush()
                        exit(-1)
                for i in range(0,3):
                    s += str(row[recap_pdns[self.top_pdns[i]]]) + ","
                total_skipped = 0
                for skipped in self.skipped_pdns:
                    total_skipped += row[recap_pdns[skipped]]
                total_skipped += row['first_others']
                s += str(total_skipped) + ","
                for header in recap_final_columns:
                    try:
                        rv = row[header]
                    except:
                        print("Final column: " + header + ", s: " + s)
                        print(str(row))
                        F.flush()
                        exit(-1)

                    s += str(rv) + ","
                uids = row['uids']
                if uids <= 0:
                    s += "0"
                else:
                    average_delay = row['sum_delay']/uids
                    s += str(average_delay)
                s += "\n"
                F.write(s)

class recap_lines:
    def __init__(self):
        self.cc_as_list = dict()

    def add_row(self, row):
        cc = str(row['CC'])
        asn = str(row['AS'])
        key = cc + '-' + asn
        if not key in self.cc_as_list:
            self.cc_as_list[key] = recap_cc_as(cc, asn)
        self.cc_as_list[key].add_row(row)

    def load_recap(self, file_name):
        df = pd.read_csv(file_name, sep=",", skipinitialspace=True)
        print(file_name + ": " + str(df.shape[0]) + " lines.")
        df.apply(lambda row: self.add_row(row),axis=1)
        print("After loading " + file_name + ", " + str(len(self.cc_as_list)) + " CC/AS.")


# Main program
if __name__ == "__main__":
    time_start = time.time()
    if len(sys.argv) < 2:
        usage()
        exit(-1)

    output_dir = sys.argv[1]
    if not os.path.isdir(output_dir):
        print("Invalid output dir: " + output_dir)
        usage()
        exit(-1)

    csv_files, has_error = rsv_arguments.parse_file_list(sys.argv[2:], [ ".csv"])
    if has_error:
        print("Invalid list of input files.")
        usage()
        exit(-1)
    
    rcl = recap_lines()
    for csv_file in csv_files:
        rcl.load_recap(csv_file)

    nb_files = 0
    for key in rcl.cc_as_list:
        if rcl.cc_as_list[key].total_uids > 10000:
            as_file = os.path.join(output_dir, "recap-" + 
                                   rcl.cc_as_list[key].query_cc + "-" + 
                                   rcl.cc_as_list[key].query_AS + ".csv")
            rcl.cc_as_list[key].save_file(as_file)
            print("Saved: " + str(len(rcl.cc_as_list[key].slices)) + " slice in " + as_file)
            nb_files += 1
    print("Saved " + str(nb_files) + " CC/AS files.")