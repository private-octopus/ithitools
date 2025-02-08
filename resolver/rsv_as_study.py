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
import top_as
import time

def usage():
    print("Usage: python rsv_as_study.py <image_dir> <log_file> <ASxxxx>\n")
    print("This script will parse the log file, extract data for the specified ASes,")
    print("and write plot and histogram images in the specied image directory.")
    print("If no AS is specified, retains all ASes with more than 1000 UIDs.")

# Main program
if __name__ == "__main__":
    time_start = time.time()
    if len(sys.argv) < 3:
        usage()
        exit(-1)

    image_dir = sys.argv[1]
    log_file = sys.argv[2]
    target_ASes = sys.argv[3:]
    if len(target_ASes) == 1 and target_ASes[0] == "TopAS":
        target_ASes = top_as.top_as_list()
    
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

    # The call to "quicker_load" will parse the log file line by line, and filter
    # the relevant events according to experiment ID, record type and query AS number.
    # Filtering reduces the number of events by a factor of maybe 5 to 10. The events
    # are immediately stacked by AS and by UID using hash tables (python dictionary),
    # which operate in O(1) per event.
    #
    # This is the longer part of the process, typically limited by the rate of data
    # transfer. On a laptop, that is about 25MB/s, which means a 24GB file is read in
    # about 15 minutes. This could maybe be sped up by working directly on
    # the compressed file.
    #
    # The combination of filtering, immediate processing and hash tables helps us
    # limit the memory requirement. Processing the 24MB file only requires about
    # 1.5GB of memory.
    ppq = rsv_log_parse.pivoted_per_query()
    nb_events = ppq.quicker_load(log_file, ip2a4, ip2a6, as_names, experiment=['0du'], \
        rr_types = [ 'A', 'AAAA', 'HTTPS' ], query_ASes = target_ASes, \
        time_start=time_start)
    print("Quick load of " + str(len(ppq.ASes)) + " ASes with " + str(nb_events) + " events.")
    time_file_read = time.time()
    print("File read at " + str(time_file_read - time_start) + " seconds.")

    # Based on the number of event, we set a threshold on the number of "both" events
    # required to start the delay analysis and the graphs. The tests in practice recognizes
    # if we are running with a test file, and lowers the threshold so that we can
    # exercise the code, event if there are few points per graph.
    target_threshold = 1000
    if nb_events < 100000:
        target_threshold = 30

    # Once all events have been loaded, we compute for each UID the delay between the
    # arrival of the first event for that UID and the arrival of the first event in
    # each of the categories of resolvers.
    ppq.compute_delta_t()
    time_delays_computed = time.time()
    print("Delays computed at " + str(time_file_read - time_start) + " seconds.")
    # Compute the list of ASes for which we have data to graph, i.e. the intersection
    # of the list that we filter and the list of ASes present in the log file.
    if len(target_ASes) == 0:
        as_list = ppq.AS_list()
    else:
        as_list = []
        for asn in target_ASes:
            if asn in ppq.ASes:
                as_list.append(asn)
        
    # get the summaries per AS
    summary_df = ppq.get_summaries(as_list, False);
    summary_file = os.path.join(image_dir, "summary.csv" )
    summary_df.to_csv(summary_file, sep=",")
    print("Published summaries for " + str(len(as_list)) + " Ases" + " in " + summary_file)
    time_summaries_computed = time.time()
    print("Summaries computed at " + str(time_summaries_computed - time_start) + " seconds.")

    # get the subnets used for each AS
    subnet_df = ppq.get_subnets()
    subnet_file = os.path.join(image_dir, "subnets.csv" )
    subnet_df.to_csv(subnet_file, sep=",")
    print("Published summaries for " + str(subnet_df.shape[0]) + " subnets" + " in " + subnet_file)
    time_summaries_published = time.time()
    print("Summaries published at " + str(time_summaries_published - time_start) + " seconds.")

    # Analyse the spread of delays for the AS that have a sufficient share of UID with events
    # from both ISP resolvers and public resolvers. 
    nb_published = 0
    for target_AS in as_list:
        if ppq.ASes[target_AS].nb_both > target_threshold:
            dot_df = ppq.get_delta_t_both(target_AS)
            plot_delay_file = os.path.join(image_dir, target_AS + "_plot_delays" )
            rsv_log_parse.do_graph(target_AS, dot_df, plot_delay_file, x_delay=True, log_y=True)
            host_delay_files = os.path.join(image_dir, target_AS + "_hist_delays" )
            rsv_log_parse.do_hist(target_AS, dot_df, image_file=host_delay_files)
            nb_published += 1
            if (nb_published%100) == 0:
                print("Published " + str(nb_published) + " AS graphs")
    print("Done publishing " + str(nb_published) + " AS graphs")
    time_finished = time.time()
    print("Finished at " + str(time_finished - time_start) + " seconds.")

    exit(0)


