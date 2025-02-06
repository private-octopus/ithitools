# APNIC test.
#
# Load an APNIC trace and create graphs for the specified AS
# 
# Usage: python rsv_as_study.py <log_file> <ASxxxx> <source_directory>

import sys
import os
from pathlib import Path
import ip2as
import rsv_log_parse
#import rsv_both_graphs
import pandas as pd
import traceback

def usage():
    print("Usage: python rsv_as_study.py <image_dir> <log_file> <ASxxxx>\n")
    print("This script will parse the log file, extract data for the specified ASes,")
    print("and write plot and histogram images in the specied image directory.")
    print("If no AS is specified, retains all ASes with more than 1000 UIDs.")

# Main program
if __name__ == "__main__":
    if len(sys.argv) < 3:
        usage()
        exit(-1)

    image_dir = sys.argv[1]
    log_file = sys.argv[2]
    target_ASes = sys.argv[3:]
    
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

    # load the experiment and RR types subset of the log file as a data frame. 
    rsv_table = rsv_log_parse.rsv_log_file()
    rsv_table.load(log_file, ip2a4, ip2a6, as_names, experiment=['0du'], \
       rr_types = [ 'A', 'AAAA', 'HTTPS' ], query_ASes = target_ASes )
    df = rsv_table.get_frame()
    print("Data frame has " + str(df.shape[0]) + " rows.")
    target_threshold = 1000
    if df.shape[0] < 100000:
        target_threshold = 50

    # pivot per unique id, get a frame per unique ID with delta-T per (class of) resolver
    # the delta_t is set to 1000000 (1000 seconds) if no reply arrives before that
    ppq = rsv_log_parse.pivoted_per_query()
    ppq.process_log(df)
    print("Pivoted to " + str(len(ppq.ASes)) + " ASes.")
    ppq.compute_delta_t()
    print("Computed delays")

    # if the AS list is empty, look at all ASes
    as_res = []
    as_list = target_ASes
    if len(as_list) == 0:
        as_list = ppq.AS_list()
    # get the summaries
    summary_df = ppq.get_summaries(as_list, False);
    summary_file = os.path.join(image_dir, "summary.csv" )
    summary_df.to_csv(summary_file, sep=",")
    print("Published summaries for " + str(len(as_list)) + "Ases" + " in " + summary_file)
    # Analyse the target AS
    nb_published = 0
    for target_AS in as_list:
        if len(target_ASes) > 0 or ppq.ASes[target_AS].nb_both > target_threshold:
            dot_df = ppq.get_delta_t_both(target_AS)
            plot_delay_file = os.path.join(image_dir, target_AS + "_plot_delays" )
            rsv_log_parse.do_graph(target_AS, dot_df, plot_delay_file, x_delay=True, log_y=True)
            host_delay_files = os.path.join(image_dir, target_AS + "_hist_delays" )
            rsv_log_parse.do_hist(target_AS, dot_df, image_file=host_delay_files)
            nb_published += 1
            if (nb_published%100) == 0:
                print("Published " + str(nb_published) + "AS graphs")
    print("Done publishing " + str(nb_published) + "AS graphs")

    exit(0)


