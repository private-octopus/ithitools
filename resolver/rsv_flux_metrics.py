# Create a csv file with one row per ASN or group of ASN with the two hours
# buckets starting at 00:30 UTC with buckets, retaining only queries that are
# - Query time less than 30 seconds after AD time stamps
# - Query RR = A, AAAA or HTTPS
# - Not coming from AS0
#
# For each query, we need RR_TYPE, and sender type: ISP, PDNS or Other.
# At this stage, we do not distinguish between different PDNS
#
# We will tabulate per ISP (CC + AS), and for each ISP per 2 hours
# time slice, the second time slice starting at 00:30 UTC the first day.
#
# We want to tabulate 9 columns, 3 each for each record type:
# - Number of UIDs for which we receive a query for that RR_type from ISP
# - Number of UIDs for which we receive a query for that RR_type from PDNS
# - Number of UIDs for which we receive a query for that RR_type from Other.
# We count each UID at most once in each category. For example, if we receive
# 3 queries for RR_Type=A from the ISP, that counts for just 1 UID for A from ISP.
#
# We also want to count the total number of UIDs overall, and two sets
# of totals:
#
# - Total number of UIDs with at least one Query for the RR_type, per RR_Type
#   (this will be lower than the sum of ISP+PDNS+Other for that RR_type,
#    because it is an OR, not a Plus)
# - Total number of UIDs with at least one query from a given source
#   (this will be lower than the sum of A-AAAA-HTTPS for teh source,
#    because it is an OR, not a Plus)
#
# We can present these totals as a 4x4 matrix:
#  from    | All          | ISP      | PDNS      | Others
#  --------|--------------|----------|-----------|-----------
#  overall | nb_uids      | uids_ISP | uids_PDNS |uids_others
#  A       | nb_uids_A    | A_ISP    | A_PDNS    |A_others
#  AAAA    | nb_uids_AAAA | AAAA_ISP | AAAA_PDNS |AAAA_others
#  HTTPS   | nb_uids_HTTPS| HTTPS_ISP| HTTPS_PDNS|HTTPS_others
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
import open_rsv
from rsv_flux import flux_log

def usage():
    print("Usage: python rsv_first_flux.py <csv_file> <log_file>\n")
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

    with open(output_file, "w") as flux_file:
        rcl = flux_log(7200, 1800, ip2a4, ip2a6, as_names, flux_file)
        for source_file in source_files:
            nb_events = rcl.load_flux_log(source_file, time_start=time_start)
            print(source_file + ": loaded " + str(nb_events) + " events at " + str(time.time() - time_start))
            sys.stdout.flush()
        rcl.save_and_close()
        print("Saved output in " + output_file)


