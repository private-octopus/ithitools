# APNIC test.
#
# Load the per top AS recap.
# 
# Usage: python rsv_other_file.py <csv_file> <recap directory>

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
    print("Usage: python rsv_other_spike_filter.py <output.csv> <recap-dir>")

def add_row(x, t):
    if x['uids'] > 0:
        ratio = x['first_others']/x['uids']
        if ratio >= 0.05 and x['first_others'] > 100:
            t.append([x['CC'], x['AS'], x['uids'], x['first_others'], ratio ])

# Main program
if __name__ == "__main__":
    time_start = time.time()
    if len(sys.argv) < 3:
        usage()
        exit(-1)

    other_spike_out = sys.argv[1]
    csv_files, has_error = rsv_arguments.parse_file_list(sys.argv[2:], [ ".csv"])
    if has_error:
        print("Invalid list of input files.")
        usage()
        exit(-1)

    is_first = True
    t = []
    for csv_file in csv_files:
        t1 = []
        df = pd.read_csv(csv_file,skipinitialspace=True)
        df.apply(lambda x: add_row(x, t1), axis=1)
        if len(t1) > 0:
            t1.sort(key=lambda x:x[4], reverse=True)
            t.append(t1[0])
    dft = pd.DataFrame(t, columns=[ 'CC', 'AS', 'uids', 'first_others', 'ratio' ])
    dft.to_csv(other_spike_out)






