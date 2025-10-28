# Study of query IP addresses.
# the report has a line per <query_cc, query_AS, query_IP, nb_uids>
# where nb_uids is the number of unique UIDs seen for the IP address

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
import csv
import random
import ipaddress
import rsv_arguments
import open_rsv
import bz2
import concurrent.futures
import ipaddress

def get_subnet(resolver_IP):
    try:
        addr = ipaddress.ip_address(resolver_IP)
        if addr.version == 6:
            subnet = ipaddress.IPv6Network(resolver_IP + "/48", strict=False)
        else:
            subnet = ipaddress.IPv4Network(resolver_IP + "/24", strict=False)
        txt = str(subnet)
    except Exception as exc:
        txt = str(resolver_IP)
    return txt

# The main difficulty in this program is to avoid exploding the memory,
# which we end up doing if we keep an entry per UID and per subnet.
# We may want to implement a two step filter:
# - keep UID in a "recent" list.
# - consider that UID are only kept as long as the time from ad is < 30sec.
# - consider that the time in logs is increasing.
# - every 30 sec or so, summarize the UID into a cc_as "count".
# 
# We could also regularly write the cc_as list to the specified file.
# That means we may end up with multiple instances to the same
# key/subnet pair in the extraction program, but that's OK because
# we can add these up in the next step.

class as0_subnet_uid_data:
    def __init__(self, subnet, query_cc, query_AS, ad_time):
        self.subnet = subnet
        self.ad_time = ad_time
        self.query_cc = query_cc
        self.query_AS = query_AS

class as0_subnet_data:
    def __init__(self):
        self.cc_as = dict()

    def add_query(self, query_cc, query_AS): 
        key = query_cc + '-' + query_AS
        if not key in self.cc_as:
            self.cc_as[key] = 1
        else:
            self.cc_as[key] += 1

    subnet_header = [ 'subnet', 'key', 'uids' ]

    def get_table(self, as0_subnet):
        total = 0
        t = []
        for key in self.cc_as:
            n = self.cc_as[key]
            r = [ str(as0_subnet), key, n ]
            t.append(r)
        return t;
# TODO: split in "get stats" and "get tables"

class rsv_as0_log:
    def __init__(self, ip2a4, ip2a6, as_names):
        self.as0_subnets = dict()
        self.ip2a4 = ip2a4
        self.ip2a6 = ip2a6
        self.as_names = as_names
        self.recent_uids = dict()
        self.last_time = 0
    
    def flush_queries(self, ad_time):
        retained = dict()
        for uid in self.recent_uids:
            if ad_time != 0 and self.recent_uids[uid].ad_time > ad_time:
                retained[uid] = self.recent_uids[uid]
            else:
                if not self.recent_uids[uid].subnet in self.as0_subnets:
                    self.as0_subnets[self.recent_uids[uid].subnet] = as0_subnet_data()
                self.as0_subnets[self.recent_uids[uid].subnet].add_query(
                    self.recent_uids[uid].query_cc, self.recent_uids[uid].query_AS)
        self.recent_uids = retained

    def add_query(self, query_cc, query_AS, as0_subnet, uid, ad_time):
        if not uid in self.recent_uids:
            self.recent_uids[uid] = as0_subnet_uid_data(as0_subnet, query_cc, query_AS, ad_time)
    
    def load_as0_log(self, log_file, log_threshold=15625, time_start=0):
        nb_events = 0
        lth = log_threshold;
        t = []
        old_time = 0
        print("Opening: " + log_file)
        if log_file.endswith(".bz2"):
            F = bz2.open(log_file, "rt")
        else:
            F = open(log_file, "r")
        for line in F:
            parsed = True
            try:
                x = rsv_log_parse.rsv_log_line()
                parsed = x.parse_line(line)
            except Exception as exc:
                traceback.print_exc()
                print('\nCode generated an exception: %s' % (exc))
                print("Cannot parse:\n" + line + "\n")
                parsed = False
            if parsed:
                if x.filter(rr_types=['A', 'AAAA', 'HTTPS'], experiment=['0du'], query_delay=30):
                    x.set_resolver_AS(self.ip2a4, self.ip2a6, self.as_names)
                    as0_subnet = get_subnet(x.resolver_IP)
                    self.last_time = x.query_time
                    self.add_query(x.query_cc, x.query_AS, as0_subnet, x.query_user_id, x.query_ad_time)
                    nb_events += 1
                    if (nb_events%lth) == 0:
                        ad_time = int(self.last_time) - 30
                        self.flush_queries(ad_time)
                        new_time = int(time.time() - time_start)
                        print(log_file + ": loaded " + str(nb_events) +  " events, " + 
                              str(len(self.as0_subnets)) + " subnets at " + str(new_time) + "s.")
                        sys.stdout.flush()
                        if lth < 1000000:
                            lth *= 2
        self.flush_queries(0)
        return nb_events

    def get_df(self):
        t = []
        for as0_subnet in self.as0_subnets:
            m = self.as0_subnets[as0_subnet].get_table(as0_subnet)
            for r in m:
                t.append(r)
        print("T has " + str(len(t)) + " lines, " + str(len(t[0])) + " columns.")
        df = pd.DataFrame(t, columns=as0_subnet_data.subnet_header)
        return df


class file_bucket:
    def __init__(self, ip2a4, ip2a6, as_names):
        self.input_files = []
        self.file_path = ""
        self.bucket_id = 0
        self.as0_log = rsv_as0_log(ip2a4, ip2a6, as_names)

    def load(self):
        # load the AS list
        time_start = time.time()
        for input_file in self.input_files:
            print("Bucket[" + str(self.bucket_id) + "]: loading " + input_file + " into " + self.file_path)
            nb_events = self.as0_log.load_as0_log(input_file, time_start=time_start)
            new_time = int(time.time() - time_start)
            print("Bucket[" + str(self.bucket_id) + "]: " + input_file + ": found " + str(nb_events) + " events, " +
                  str(len(self.as0_log.as0_subnets)) + " subnets at " + str(new_time) + "s.")

    def save(self):
        print("Bucket[" + str(self.bucket_id) + "]: summarize " + str(len(self.as0_log.as0_subnets)) + " subnets.")
        df = self.as0_log.get_df()
        print("Bucket[" + str(self.bucket_id) + "]: saving " + str(df.shape[0]) + " subnets/key to " + self.file_path)
        df.to_csv(self.file_path)

def load_bucket(bucket):
    bucket.load()
    bucket.save()
    return True

class rsv_as0_summary:
    def __init__(self, ip2a4, ip2a6, as_names):
        self.as0_subnets = dict()
        self.ip2a4 = ip2a4
        self.ip2a6 = ip2a6
        self.as_names = as_names

    def add_subnet_entry(self, x):
        # subnet_header = [ 'subnet', 'key', 'uids' ]
        as0_subnet = x['subnet']
        key = x['key']
        uids = x['uids']
        if not as0_subnet in self.as0_subnets:
            self.as0_subnets[as0_subnet] = dict()
        if not key in self.as0_subnets[as0_subnet]:
            self.as0_subnets[as0_subnet][key] = uids
        else:
            self.as0_subnets[as0_subnet][key] += uids

    def load_partial_result(self, result_file_name):
        df = pd.read_csv(result_file_name)
        df.apply(lambda x: self.add_subnet_entry(x), axis=1)
    
    flat_header = [ 'subnet', 'uids', 'cc1', 'AS1', 'name1', 'uids1', 
                   'cc2', 'AS2', 'name2', 'uids2', 'cc3', 'AS3', 'name3',  'uids3']

    def flatten(self, as0_subnet):
        total = 0
        t = []
        as_sn = self.as0_subnets[as0_subnet]
        for key in as_sn:
            n = as_sn[key]
            total += n
            r = [ key, n ]
            t.append(r)
        t.sort(key=lambda x:x[1], reverse=True)
        
        v = [ as0_subnet, total ]
        for k in range(0, 3):
            if k < len(t):
                r = t[k]
                parts = r[0].split('-')
                v += [ parts[0], parts[1], self.as_names.name(parts[1]), r[1] ]
            else:
                v += [ "", "", "", 0 ]
        return(v)

    def get_df(self):
        t = []
        for as0_subnet in self.as0_subnets:
            t.append(self.flatten(as0_subnet))
        t.sort(key=lambda x:x[1], reverse=True)
        df = pd.DataFrame(t, columns=rsv_as0_summary.flat_header)
        return(df)

def usage():
    print("Usage: python rsv_as0_stats.py  <output_dir>  <input_file> ... <input_file>\n")
    print("This script will load the specified log files,")
    print("extract the subnets mapped to AS0 in each file,")
    print("and the create a summary list of these subnets.")

# main

if __name__ == "__main__":
    time_start = time.time()
    if len(sys.argv) < 3:
        usage()
        exit(-1)

    # parse the arguments
    output_dir = sys.argv[1]
    
    input_files, has_error = rsv_arguments.parse_file_list(sys.argv[2:], [ ".txt", ".bz2" ])
    if has_error:
        print("Invalid list of input files.")
        usage()
        exit(-1)

    # get the as name tables
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
    
    nb_process = os.cpu_count()
    print("Aiming for " + str(nb_process) + " processes")
    process_left = nb_process

    bucket_list = []
    bucket_first = 0
    bucket_id = 0
    while bucket_first < len(input_files):
        bucket = file_bucket(ip2a4, ip2a6, as_names)
        step = int((len(input_files) - bucket_first + process_left - 1)/process_left)
        print("step: " + str(step))
        process_left -= 1
        bucket_next = min(bucket_first+step, len(input_files))
        bucket.input_files = input_files[bucket_first:bucket_next]
        print("bucket: " + str(bucket_first) + "," + str(bucket_next))
        bucket.bucket_id = bucket_id
        bucket.file_path = os.path.join(output_dir, "tmp_subnets_" + str(bucket.bucket_id) + ".csv" )
        bucket_list.append(bucket)
        bucket_id += 1
        bucket_first = bucket_next

    nb_process = min(nb_process, len(bucket_list))
    # print("Will use " + str(nb_process) + " processes, " + str(len(bucket_list)) + " buckets")
    total_files = 0
    for bucket in bucket_list:
        total_files += len(bucket.input_files)
    print("%d files in %d buckets (%d .. %d), vs %d" 
          %(total_files, len(bucket_list), len(bucket_list[0].input_files),
           len(bucket_list[len(bucket_list)-1].input_files), len(input_files)))

    start_time = time.time()
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

    # collate the results
    rsum = rsv_as0_summary(ip2a4, ip2a6, as_names)
    for bucket in bucket_list:
        print("From bucket[" + str(bucket.bucket_id) + "] loading " + bucket.file_path)
        rsum.load_partial_result(bucket.file_path)

    # save the results
    df = rsum.get_df()
    rsum_file = os.path.join(output_dir, "as0_summary.csv" )
    df.to_csv(rsum_file)
    print("Report saved in " + rsum_file)