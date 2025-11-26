# Create a csv file with one row per ASN or group of ASN with the two hours
# buckets starting at 00:30 UTC with buckets:
# Country, AS
# NbUIDs,
#uid resolved first by ISP
#uid resolved first by top-1 public resolver (with its name in the column header)
#uid resolved first by top-2 public resolver (with its name in the column header)
#uid resolved first by top-3 public resolver (with its name in the column header)
# Question: this requires per ISP definition of public resolver, which does not allow
# for easy total across several ISP. Will rather tabulate all PDNS, do the
# reduction to top 3 later.
# HTTPS
# Duplicate total
# duplicate [0,500ms] with ISP only
# duplicate [0,500ms] with ISP + different AS
# duplicate [0,500ms] with ISP + public resolver
# duplicate [500ms,30s]
# zombies. Zombies are queries received during that time slice that are > 30s, regardless of being duplicate or not
#
# we can implement this using the same software model as the other rsv metrics:
#
# parse the log file. 
# retain events of class 0du, records in { HTTPS, A, AAAA }
#
# For each UDI:
# - if first request for that UID, fill the "is first" indicator, and the first time.
# - if HTTPS, mark UID has "received HTTPS"
# - if not first request for that data type, fill the "is_dup" tabulation
#
# If UID known and delay > 30 sec, add to zombie count per AS and slice.
# - Maybe get different definition for known zombies
# 
# Compute delay between ad time and first query. Compute sum of delays for
# the period, and then average. 

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
import open_rsv
from rsv_recap import recap_log

def usage():
    print("Usage: python rsv_first_recap.py <csv_file> <log_file>\n")
    print("This script will parse the log file, extract data per AS and time slice,")
    print("and save the parsed data in the csv file.")





# Main program -- we will start by parsing the input files.
if __name__ == "__main__":
    time_start = time.time()
    if len(sys.argv) < 3:
        usage()
        exit(-1)

    output_file = sys.argv[1]

    source_files, has_error = rsv_arguments.parse_file_list(sys.argv[2:], [ ".bz2", ".log", ".txt"])
    if has_error:
        print("Invalid list of input files.")
        usage()
        exit(-1)
        
    source_path = Path(__file__).resolve()
    resolver_dir = source_path.parent
    auto_source_dir = resolver_dir.parent
    print("Auto source path is: " + str(auto_source_dir) + " (source: " + str(source_path) + ")")
    source_dir = os.path.join(auto_source_dir, "data") 
    ip2a4_file = os.path.join(source_dir, "ip2as.csv") 
    ip2a6_file = os.path.join(source_dir, "ip2asv6.csv")
    as_names_file = os.path.join(source_dir, "as_names.csv")   
    ip2a4 = ip2as.ip2as_table()
    ip2a4.load(ip2a4_file)
    ip2a6 = ip2as.ip2as_table()
    ip2a6.load(ip2a6_file)
    as_names = ip2as.asname()
    as_names.load(as_names_file)
    time_loaded = time.time()   

    print("Tables loaded at " + str(time_loaded - time_start) + " seconds.")

    with open(output_file, "w") as recap_file:
        rcl = recap_log(7200, 1800, ip2a4, ip2a6, as_names, recap_file)
        for source_file in source_files:
            nb_events = rcl.load_recap_log(source_file, time_start=time_start)
            print(source_file + ": loaded " + str(nb_events) + " events at " + str(time.time() - time_start))
            sys.stdout.flush()
        rcl.save_and_close()
        print("Saved output in " + output_file)


