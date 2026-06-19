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
from rsv_pdns import prov_parse, prov_slice
import concurrent.futures

def usage():
    print("Usage: python rsv_prov_files.py <output_dir> <temp_dir> <log_files>\n")
    print("This script will parse the log files and create for each of them a ")
    print("pdns_summary file in the temporary folder.\n")
    print("This script will parse these files,")
    print("and then create a summary file.")


class file_bucket:
    def __init__(self, ip2a4, ip2a6, as_names, output_file, source_file, bucket_id, time_start):
        self.ip2a4 = ip2a4
        self.ip2a6 = ip2a6
        self.as_names = as_names
        self.output_file = output_file
        self.source_file = source_file
        self.bucket_id = bucket_id
        self.time_start = time_start

    def load(self):

        if not os.path.exists(self.output_file):
            pp = prov_parse(self.ip2a4, self.ip2a6, self.as_names)
            nb_events = pp.load_prov_log(self.source_file, time_start=self.time_start)
            print(self.source_file + ": loaded " + str(nb_events) + " events at " + str(time.time() - self.time_start))
            sys.stdout.flush()
            ps = pp.get_summary_slice()
            with bz2.open(self.output_file, "wt") as F:
                ps.get_json(F, compact=False)
        else:
            print(self.output_file + ": already exists.")
            sys.stdout.flush()

def load_bucket(bucket):
    bucket.load()
    return True

# Main program -- we will start by parsing the input files.
if __name__ == "__main__":
    time_start = time.time()
    if len(sys.argv) < 4:
        usage()
        exit(-1)

    output_dir = sys.argv[1]
    if not os.path.isdir(output_dir):
        print("Invalid output folder.")
        usage()
        exit(-1)

    temp_dir = sys.argv[2]
    if not os.path.isdir(temp_dir):
        print("Invalid temporary folder.")
        usage()
        exit(-1)

    source_files, has_error = rsv_arguments.parse_file_list(sys.argv[3:], [ ".bz2", ".log", ".txt"])
    if has_error:
        print("Invalid list of input files.")
        usage()
        exit(-1)

    # load the IP mapping tables
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

    # Prepare the parallel buckets
    nb_process = len(source_files)
    bucket_list = []
    bucket_id = 0
    for source_file in source_files:
        item = os.path.basename(source_file)
        item = item[:-4]
        if item.startswith("queries"):
            item = item[7:]
        output_file = os.path.join(temp_dir, "prov-" + item + ".json.bz2")
        bucket = file_bucket(ip2a4, ip2a6, as_names, output_file, source_file, bucket_id, time_loaded)
        bucket_list.append(bucket)
        bucket_id += 1

    nb_process = len(bucket_list)
    print("Will use " + str(nb_process) + " processes.")

    with concurrent.futures.ProcessPoolExecutor(max_workers = nb_process) as executor:
        future_to_bucket = {executor.submit(load_bucket, bucket):bucket for bucket in bucket_list }
        for future in concurrent.futures.as_completed(future_to_bucket):
            bucket = future_to_bucket[future]
            try:
                data = future.result()
                print('Bucket %d complete' % (bucket.bucket_id))
            except Exception as exc:
                traceback.print_exc()
                print('Bucket %d generated an exception: %s' % (bucket.bucket_id, exc))
    bucket_time = time.time()

    # All the buckets have been processed. Now, create the tables.

    prov_total = prov_slice(0)
    for bucket in bucket_list:
        prov_bucket = prov_slice(0)
        prov_bucket.load_json_file(bucket.output_file)
        prov_total.add_slice(prov_bucket)

    summary_file = os.path.join(output_dir, "prov-summary.csv")
    df = prov_total.get_df()
    df.to_csv(summary_file)
    print("Saved " + str(df.shape[0]) + " CC/AS summaries in " + summary_file)

    # summary_flat_file = os.path.join(output_dir, "prov-flat-summary.csv")
    # flat_df = prov_total.get_flat_df()
    # flat_df.to_csv(summary_flat_file)
    # print("Saved " + str(flat_df.shape[0]) + " CC/AS flat summaries in " + summary_flat_file)

    summary_json_file = os.path.join(output_dir, "prov-flat-summary.json.bz2")
    with bz2.open(summary_json_file,"wt") as F:
         prov_total.get_json(F, compact=False)
    print("Saved long summary in " + summary_json_file)