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
from rsv_recap import recap_log, recap_lines
import concurrent.futures

def usage():
    print("Usage: python rsv_recap2_daily_fix.py <output_dir>\n")
    print("This script will parse the per log summaries, ")
    print("find all slices for a given CC/AS combination, and then create")
    print("in the output directory a file for each CC/AS that has > 10,000")
    print("events")

# Main program -- we will start by parsing the input files.
if __name__ == "__main__":
    time_start = time.time()
    if len(sys.argv) < 2:
        usage()
        exit(-1)

    output_dir = sys.argv[1]
    if not os.path.isdir(output_dir):
        print("Invalid output folder.")
        usage()
        exit(-1)
    


    # All the buckets have been processed. Now, create the tables.
    recap_prefix="recap2-"
    rcl = recap_lines()
    flist =  os.listdir(output_dir)
    recap_files = [f for f in flist if f.startswith(recap_prefix)]

    print(recap_files)

    for rcf in recap_files:
        rcp = os.path.join(output_dir,rcf)
        rcl.load_recap(rcp)

    #nb_files = 0
    #for key in rcl.cc_as_list:
    #    if rcl.cc_as_list[key].total_uids > 10000:
    #        as_file = os.path.join(output_dir, "recap-" + 
    #                               rcl.cc_as_list[key].query_cc + "-" + 
    #                               rcl.cc_as_list[key].query_AS + ".csv")
    #        rcl.cc_as_list[key].save_file(as_file)
    #        print("Saved: " + str(len(rcl.cc_as_list[key].slices)) + " slice in " + as_file)
    #        nb_files += 1
    #print("Saved " + str(nb_files) + " CC/AS files.")
    
    summary_file = os.path.join(output_dir, "recap2-summary.csv")
    df = rcl.summary_df()
    df.to_csv(summary_file)
    print("Saved " + str(df.shape[0]) + " CC/AS summaries in " + summary_file)