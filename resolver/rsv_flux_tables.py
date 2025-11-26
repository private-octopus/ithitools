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
from rsv_flux import flux_lines

def usage():
    print("Usage: python rsv_flux_tables.py <output_dir> <flux_metrics_files>\n")
    print("This script will parse the files created by rsv_flux_metrics,")
    print("find all slices for a given CC/AS combination, and then create")
    print("in the output directory a file for each CC/AS that has > 10,000")
    print("events")


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