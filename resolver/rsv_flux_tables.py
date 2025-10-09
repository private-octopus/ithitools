# Recapitulate the tables per CC/AS.
# Produce one file for all CC/AS that have more that 1000 commands per day (10000 in the week)
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
    print("Usage: python rsv_flux_tables.py <output_dir> <flux_metrics_files>\n")
    print("This script will parse the files created by rsv_flux_metrics,")
    print("find all slices for a given CC/AS combination, and then create")
    print("in the output directory a file for each CC/AS that has > 10,000")
    print("events")

flux_columns = [
    "CC", "AS", "start", "uids", "nb_isp", "nb_pdns", "nb_others",
    "nb_A", "nb_A_isp", "nb_A_pdns", "nb_A_others",
    "nb_AAAA", "nb_AAAA_isp", "nb_AAAA_pdns", "nb_AAAA_others",
    "nb_HTTPS", "nb_HTTPS_isp", "nb_HTTPS_pdns", "nb_HTTPS_others",
]

class flux_row:
    def __init__(self, row):
        self.query_cc = row['CC']
        self.query_AS = row['AS']
        self.start = row['start']
        self.total_uids = row['uids']
        self.nb_isp = row['nb_isp']
        self.nb_pdns = row['nb_pdns']
        self.nb_others = row['nb_others']
        self.nb_A = row['nb_A']
        self.nb_A_isp = row['nb_A_isp']
        self.nb_A_pdns = row['nb_A_pdns']
        self.nb_A_others = row['nb_A_others']
        self.nb_AAAA = row['nb_AAAA']
        self.nb_AAAA_isp = row['nb_AAAA_isp']
        self.nb_AAAA_pdns = row['nb_AAAA_pdns']
        self.nb_AAAA_others = row['nb_AAAA_others']
        self.nb_HTTPS = row['nb_HTTPS']
        self.nb_HTTPS_isp = row['nb_HTTPS_isp']
        self.nb_HTTPS_pdns = row['nb_HTTPS_pdns']
        self.nb_HTTPS_others = row['nb_HTTPS_others']

    def add_row(self, row):
        self.total_uids += row['uids']
        self.nb_isp += row['nb_isp']
        self.nb_pdns += row['nb_pdns']
        self.nb_others += row['nb_others']
        self.nb_A += row['nb_A']
        self.nb_A_isp += row['nb_A_isp']
        self.nb_A_pdns += row['nb_A_pdns']
        self.nb_A_others += row['nb_A_others']
        self.nb_AAAA += row['nb_AAAA']
        self.nb_AAAA_isp += row['nb_AAAA_isp']
        self.nb_AAAA_pdns += row['nb_AAAA_pdns']
        self.nb_AAAA_others += row['nb_AAAA_others']
        self.nb_HTTPS += row['nb_HTTPS']
        self.nb_HTTPS_isp += row['nb_HTTPS_isp']
        self.nb_HTTPS_pdns += row['nb_HTTPS_pdns']
        self.nb_HTTPS_others += row['nb_HTTPS_others']

class flux_cc_as:
    def __init__(self, query_cc, query_AS):
        self.query_cc = query_cc
        self.query_AS = query_AS
        self.slices = dict()
        self.total_uids = 0

    def add_row(self, row):
        self.total_uids += row['uids']
        start = row['start']
        if not start in self.slices:
            self.slices[start] = flux_row(row)
        else:
            self.slices[start].add_row(row)

    def get_columns(self):
        return flux_columns

    def save_file(self, file_name):
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
                r_row = self.slices[start]
                row = self.slices[start]
                s = ""
                s += str(r_row.query_cc) + ","
                s += str(r_row.query_AS) + ","
                s += str(r_row.start) + ","
                s += str(r_row.total_uids) + ","
                s += str(r_row.nb_isp) + ","
                s += str(r_row.nb_pdns) + ","
                s += str(r_row.nb_others) + ","
                s += str(r_row.nb_A) + ","
                s += str(r_row.nb_A_isp) + ","
                s += str(r_row.nb_A_pdns) + ","
                s += str(r_row.nb_A_others) + ","
                s += str(r_row.nb_AAAA) + ","
                s += str(r_row.nb_AAAA_isp) + ","
                s += str(r_row.nb_AAAA_pdns) + ","
                s += str(r_row.nb_AAAA_others) + ","
                s += str(r_row.nb_HTTPS) + ","
                s += str(r_row.nb_HTTPS_isp) + ","
                s += str(r_row.nb_HTTPS_pdns) + ","
                s += str(r_row.nb_HTTPS_others) + ","
                s += "\n"
                F.write(s)

class flux_lines:
    def __init__(self):
        self.cc_as_list = dict()

    def add_row(self, row):
        cc = str(row['CC'])
        asn = str(row['AS'])
        key = cc + '-' + asn
        if not key in self.cc_as_list:
            self.cc_as_list[key] = flux_cc_as(cc, asn)
        self.cc_as_list[key].add_row(row)

    def load_flux(self, file_name):
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
    
    rcl = flux_lines()
    for csv_file in csv_files:
        rcl.load_flux(csv_file)

    nb_files = 0
    for key in rcl.cc_as_list:
        if rcl.cc_as_list[key].total_uids > 10000:
            as_file = os.path.join(output_dir, "flux-" + 
                                   rcl.cc_as_list[key].query_cc + "-" + 
                                   rcl.cc_as_list[key].query_AS + ".csv")
            rcl.cc_as_list[key].save_file(as_file)
            print("Saved: " + str(len(rcl.cc_as_list[key].slices)) + " slice in " + as_file)
            nb_files += 1
    print("Saved " + str(nb_files) + " CC/AS files.")