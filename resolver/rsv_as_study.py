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
import rsv_both_graphs
import pandas as pd
import traceback

def usage():
    print("Usage: python rsv_as_study.py <image_dir> <ASxxxx> <log_file>\n")
    print("This script will parse the log file, extract data for the specified AS,")
    print("and write plot and histogram images in the specied image directory.")

# Main program
if __name__ == "__main__":
    if len(sys.argv) != 4:
        usage()
        exit(-1)

    image_dir = sys.argv[1]
    target_AS = sys.argv[2]
    log_file = sys.argv[3]
    
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
       rr_types = [ 'A', 'AAAA', 'HTTPS' ], query_ASes = [ target_AS ] )
    df = rsv_table.get_frame()

    # pivot per unique id, get a frame per unique ID with delta-T per (class of) resolver
    # the delta_t is set to 1000000 (1000 seconds) if no reply arrives before that
    ppq = rsv_log_parse.pivoted_per_query()
    ppq.process_log(df)
    dfdt = ppq.get_frame_delta_t()

    # Analyse the target AS
    as_res = []
    rbg = rsv_both_graphs.per_as_analysis(target_AS, dfdt)
    rbg.compute_both()
    l = rbg.to_list()
    as_res.append(l)
    plot_delay_file = os.path.join(image_dir, target_AS + "_plot_delays" )
    rbg.do_graph(plot_delay_file, x_delay=True, log_y=True)
    host_delay_files = os.path.join(image_dir, target_AS + "_hist_delays" )
    rbg.do_hist(image_file=host_delay_files)

    as_df = pd.DataFrame(as_res,columns=rsv_both_graphs.per_as_analysis.list_headers())
    as_csv_file = os.path.join(image_dir, target_AS + "_summary.csv" )
    as_df.to_csv(as_csv_file, sep=",")


    exit(0)


