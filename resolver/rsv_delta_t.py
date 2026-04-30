# The log files provided by APNIC are almost sorted by query time, 
# but "almost" matter. We know that in some cases, there is as much
# as -150ms delay between the query time encountered for the first
# event for a UID and the subsequent event with the earliest query time.
# We want to quantify that, so we write script.
# 
# Usage: python rsv_delta_t.py <result> <input files>


import sys
import os
from pathlib import Path
import ip2as
import rsv_log_parse
import pandas as pd
import traceback
import top_as
import time
import bz2
from rsv_delay_class import delay_query_as
import rsv_arguments
import open_rsv
import time
import concurrent.futures


delta_first_max = 0

PDNS_names = [
    'googlePDNS',
    'cloudflare',
    'opendns',
    'quad9',
    'level3',
    'neustar',
    'he'
]

Prov_names = [ 'ISP', 
    'googlepdns',
    'cloudflare',
    'opendns',
    'quad9',
    'level3',
    'neustar',
    'he',
    'other_pdns',
    'same_CC',
   'others' ]

Prov_index = {
    'ISP':0,
    'googlepdns':1,
    'cloudflare':2,
    'opendns':3,
    'quad9':4,
    'level3':5,
    'neustar':6,
    'he':7,
    'other_pdns':8,
    'others':10,
    'Same_AS':0,
    'Same_group':0,
    'Same_CC':9,
    'same_CC':9,
    'Cloud':10,
    'Other_cc':10
}

Prov_index_others = 8

rr_names = [ 'A', 'AAAA', 'HTTPS' ]

rr_index = {
    'A':0, 'AAAA':1, 'HTTPS':2
}

delta_range = [ 0, 0.01, 0.03, 0.1, 0.3, 1, 3, 10, 30 ]

range_names = [
    "nb_0ms",
    "nb_u10ms",
    "nb_u30ms",
    "nb_u100ms",
    "nb_u300ms",
    "nb_u1s",
    "nb_u3s",
    "nb_u10s",
    "nb_u30s"]

addr_stats = [
    "sum_v4",
    "max_v4",
    "average_v4",
    "sum_v6",
    "max_v6",
    "average_v6",
    "max_v4v6"]

delta_set = [ 0, 0.001, 0.003, 0.01, 0.03, 0.1, 0.3, 30 ]
delta_names = [ "uids_0", "uids_1ms", "uids_3ms", "uids_10ms", "uids_30ms",
              "uids_100ms", "uids_300ms", "uids_larger" ]

# compute the delta t for a specific RR type and a specific UID
class delta_t_rr_uid:
    def __init__(self, query_time):
        self.first_time = query_time
        self.earliest_time = query_time
        self.nb_early = 0
        self.nb_total = 1
    def check(self, query_time):
        self.nb_total += 1
        if query_time < self.first_time:
            self.nb_early += 1
            if query_time < self.earliest_time:
                self.earliest_time = query_time

# rr_slice: summarize the delta_t for a given RR type
class delta_t_rr_slice:
    def __init__(self):
        self.uids = dict()
        self.uids_previous = dict()
        self.nb_uids = 0
        self.uids_early = 0
        self.nb_early = 0
        self.nb_total = 0
        self.max_delta = 0
        self.sum_delta = 0
        self.average_delta = 0
        self.delta_set = [ 0, 0, 0, 0, 0, 0, 0, 0 ]

    def summarize(self):
        for uid in self.uids_previous:
            uid_v = self.uids_previous[uid]
            delta_t = uid_v.first_time - uid_v.earliest_time
            self.nb_uids += 1
            if uid_v.nb_early > 0:
                self.uids_early += 1
                self.nb_early += uid_v.nb_early
            self.nb_total += uid_v.nb_total
            if self.max_delta < delta_t:
                self.max_delta = delta_t
            self.sum_delta += delta_t
            for i in range(0, len(delta_set)):
                if delta_t <= delta_set[i]:
                    self.delta_set[i] += 1
                    break
        if self.nb_uids > 0:
            self.average_delta = self.sum_delta / self.nb_uids
            
        self.uids_previous = self.uids
        self.uids = dict()

    def add_query(self, uid, query_time):
        if uid in self.uids_previous:
            self.uids_previous[uid].check(query_time)
        else:
            if not uid in self.uids:
                self.uids[uid] = delta_t_rr_uid(query_time)
            self.uids[uid].check(query_time)

    def get_headers():
        r = [ "nb_uids", "uids_early", "nb_total", "nb_early",
              "max_delta", "sum_delta", "average_delta" ]
        for dn in delta_names:
            r.append(dn)
        return r

    def get_row(self):
        r = [ self.nb_uids, self.uids_early, self.nb_total,
            self.nb_early, self.max_delta, self.sum_delta, self.average_delta ]
        for i in range(0, len(delta_set)):
            r.append(self.delta_set[i])
        return r

class delta_t_parse:
    def __init__(self, ip2a4, ip2a6, as_names):
        self.ip2a4 = ip2a4 
        self.ip2a6 = ip2a6
        self.as_names = as_names
        self.rr_deltas = [ delta_t_rr_slice(), delta_t_rr_slice(), delta_t_rr_slice() ]
        self.current_first_time = 0

    def summarize(self):
        for i_rr in range(0,3):
            self.rr_deltas[i_rr].summarize()

    def add_query(self, uid, query_time, query_ad_time, query_rr):
        seconds_in_day = int(query_ad_time) % (24*3600)
        #print (uid + ": " + str(query_time) + ", " + str(query_ad_time) + ", " + str(query_rr))
        if seconds_in_day < 30 or \
            seconds_in_day > (24*3600) - 30 or \
            query_time > query_ad_time + 30:
            pass
        else:
            if self.current_first_time == 0:
                self.current_first_time = query_time
            if query_time > (self.current_first_time + 60):
                self.summarize()
            i_rr = 2
            if query_rr == "A":
                i_rr = 0
            elif query_rr == "AAAA":
                i_rr = 1
            self.rr_deltas[i_rr].add_query(uid, query_time)

    def load_log(self, log_file, log_threshold=15625, time_start=0):
        nb_events = 0
        lth = log_threshold;
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
                if x.filter(rr_types=['A', 'AAAA', 'HTTPS'], experiment=['0du'], query_delay=1000000000, check_dotnxdomain=True):
                    if x.resolver_AS == "" or x.resolver_cc == "":
                        x.set_resolver_AS(self.ip2a4, self.ip2a6, self.as_names)
                    if x.resolver_AS != 'AS0':
                        self.add_query(x.query_user_id, x.query_time, x.query_ad_time, x.rr_type)

                    nb_events += 1
                    if (nb_events%lth) == 0:
                        new_time = time.time() - time_start
                        print(log_file + ": loaded " + str(nb_events) + " events at " + str(new_time))
                        sys.stdout.flush()
                        if lth < 1000000:
                            lth *= 2
        self.summarize()
        self.current_first_time = 0
        return nb_events

    def get_df(self):
        self.summarize()
        self.summarize()
        headers = [ 'query_RR' ] + delta_t_rr_slice.get_headers()

        t = []
        for i in range(0,3):
            r = [ rr_names[i] ] + self.rr_deltas[i].get_row()
            t.append(r)

        df = pd.DataFrame(t, columns=headers)
        return df

    def save_and_close(self, output_file):
        df = self.get_df()
        df.to_csv(output_file)

class delta_t_rr_summary:
    def __init__(self):
        self.nb_uids = 0
        self.uids_early = 0
        self.nb_early = 0
        self.nb_total = 0
        self.max_delta = 0
        self.sum_delta = 0
        self.average_delta = 0
        self.delta_set = [ 0, 0, 0, 0, 0, 0, 0, 0 ]

    def add_row(self, x):
        self.nb_uids += x['nb_uids']
        self.uids_early += x['uids_early']
        self.nb_early += x['nb_early']
        self.nb_total += x['nb_total']
        if x['max_delta'] > self.max_delta:
            self.max_delta = x['max_delta']
        self.sum_delta += x['sum_delta']
        for i in range(0, len(delta_names)):
            self.delta_set[i] += x[delta_names[i]]
        if self.nb_uids > 0:
            self.average_delta = self.sum_delta / self.nb_uids

    def get_row(self):
        r = [ self.nb_uids, self.uids_early, self.nb_total,
            self.nb_early, self.max_delta, self.sum_delta, self.average_delta ]
        for i in range(0, len(delta_set)):
            r.append(self.delta_set[i])
        return r

class delta_t_summary:
    def __init__(self):
        self.rr_deltas = [ delta_t_rr_summary(), delta_t_rr_summary(), delta_t_rr_summary() ]
        self.current_first_time = 0

    def add_row(self, x):
        i_rr = 2
        if x["query_RR"] == "A":
            i_rr = 0
        elif x["query_RR"] == "AAAA":
            i_rr = 1
        self.rr_deltas[i_rr].add_row(x)
        
    def load(self, fn):
        df = pd.read_csv(fn)
        df.apply(lambda row: self.add_row(row),axis=1)

    def get_df(self):
        headers = [ 'query_RR' ] + delta_t_rr_slice.get_headers()

        t = []
        for i in range(0,3):
            r = [ rr_names[i] ] + self.rr_deltas[i].get_row()
            t.append(r)

        df = pd.DataFrame(t, columns=headers)
        return df

#parallel


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
            dtp = delta_t_parse(self.ip2a4, self.ip2a6, self.as_names)
            nb_events = dtp.load_log(self.source_file, time_start=self.time_start)
            print(self.source_file + ": loaded " + str(nb_events) + " events at " + str(time.time() - self.time_start))
            sys.stdout.flush()
            dtp.save_and_close(self.output_file)
        else:
            print(self.output_file + ": already exists.")
            sys.stdout.flush()

def load_bucket(bucket):
    bucket.load()
    return True


# main
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

    # prepare the buckets
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
        output_file = os.path.join(temp_dir, "delta_t-" + item + ".csv")
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

    # Summarize all the results 
    
    delta_t_total = delta_t_summary()
    for bucket in bucket_list:
        delta_t_total.load(bucket.output_file)

    delta_t_file = os.path.join(output_dir, "delta_t.csv")
    df = delta_t_total.get_df()
    df.to_csv(delta_t_file)