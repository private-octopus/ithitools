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

Pdns_index = {
    'googlepdns':1,
    'cloudflare':2,
    'opendns':3,
    'quad9':4,
    'level3':5,
    'neustar':6,
    'he':7
}

Prov_index_others = 10
Prov_index_same_cc = 9

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

def get_prov(resolver_cc, resolver_AS, resolver_tag):
    if resolver_tag in Pdns_index:
        prov = resolver_tag
    else:
        resolver_cc = str(resolver_cc)
        if len(resolver_cc) > 2:
            resolver_cc = 'ZZ'
        prov = resolver_cc + '-' + resolver_AS
    return prov

class zombie_cc_as_rr_prov:
    def __init__(self, resolver_cc, resolver_AS, resolver_tag):
        self.resolver_cc = resolver_cc
        self.resolver_AS = resolver_AS
        self.resolver_tag = resolver_tag
        self.nb = 0

class zombie_cc_as_rr:
    def __init__(self):
        self.total = 0
        self.prov = dict()
        self.nb = 0

    def add(self, resolver_cc, resolver_AS, resolver_tag, nb):
        self.nb += nb
        resolver_cc = str(resolver_cc)
        if len(resolver_cc) != 2:
            resolver_cc = 'ZZ'
        prov = resolver_cc + '-' + resolver_AS + '-' + resolver_tag
        if not prov in self.prov:
            self.prov[prov] = zombie_cc_as_rr_prov(resolver_cc, resolver_AS, resolver_tag)
        self.prov[prov].nb += nb

class zombie_cc_as:
    def __init__(self, query_cc, query_AS):
        self.total = 0
        self.query_cc = query_cc
        self.query_AS = query_AS
        self.nb = 0
        self.rr = dict()

    def add(self, rr_type, resolver_cc, resolver_AS, resolver_tag, nb):
        self.nb += nb
        if not rr_type in self.rr:
            self.rr[rr_type] = zombie_cc_as_rr()
        self.rr[rr_type].add(resolver_cc, resolver_AS, resolver_tag, nb)

    # add all the lines for the rr_types to the table
    def get_df(self, query_cc, query_as):
        pass


class zombie_parse:
    def __init__(self, ip2a4, ip2a6, as_names):
        self.ip2a4 = ip2a4
        self.ip2a6 = ip2a6
        self.as_names = as_names
        self.zombie_AS = dict()
        self.zombie_max = 30

    def add(self, query_cc, query_AS, rr_type, resolver_cc, resolver_AS, resolver_tag, nb):
        if len(str(query_cc)) != 2:
            query_cc = 'ZZ'
        else:
            query_cc = str(query_cc);

        key = str(query_cc) + '-' + query_AS
        if not key in self.zombie_AS:
            self.zombie_AS[key] = zombie_cc_as(query_cc, query_AS)
        self.zombie_AS[key].add(rr_type, resolver_cc, resolver_AS, resolver_tag, nb)

    def load_log(self, log_file, log_threshold=15625, time_start=0):
        nb_events = 0
        nb_zombies = 0
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
                if x.filter(rr_types=['A', 'AAAA', 'HTTPS'], experiment=['0du'], query_delay=1000000000, check_dotnxdomain=True):
                    x.set_resolver_AS(self.ip2a4, self.ip2a6, self.as_names)
                    if x.resolver_AS == "" or x.resolver_cc == "":
                        x.set_resolver_AS(self.ip2a4, self.ip2a6, self.as_names)
                    if x.resolver_AS != 'AS0':
                        seconds_in_day = int(x.query_ad_time) % (24*3600)
                        if seconds_in_day < 30 or \
                            seconds_in_day >= (24*3600) - 30:
                            # TODO: add another output file containing the transactions that are dropped here,
                            # so they can be processed in a second pass.
                            pass
                        elif x.query_time - x.query_ad_time >= self.zombie_max:
                            self.add(x.query_cc, x.query_AS, x.rr_type, x.resolver_cc, x.resolver_AS, x.resolver_tag, 1)
                            nb_zombies += 1
                    nb_events += 1
                    if (nb_events%lth) == 0:
                        new_time = time.time() - time_start
                        print(log_file + ": loaded " + str(nb_events) + " events at " + str(new_time))
                        sys.stdout.flush()
                        if lth < 1000000:
                            lth *= 2
        print(log_file + ": found " + str(nb_zombies) + " zombies in " + str(nb_events) + " events.");
        sys.stdout.flush()
        return nb_events

    def get_df(self):
        t = []
        for key in self.zombie_AS:
            for rr_type in self.zombie_AS[key].rr:
                for prov in self.zombie_AS[key].rr[rr_type].prov:
                    r = [ self.zombie_AS[key].query_cc, self.zombie_AS[key].query_AS, rr_type,
                          self.zombie_AS[key].rr[rr_type].prov[prov].resolver_cc,
                          self.zombie_AS[key].rr[rr_type].prov[prov].resolver_AS,
                          self.zombie_AS[key].rr[rr_type].prov[prov].resolver_tag,
                          self.zombie_AS[key].rr[rr_type].prov[prov].nb ]
                    t.append(r)

        print("Found " + str(len(t)) + " zombie ASes/rr/prov.")
        df = pd.DataFrame(t, columns=['CC', 'AS', 'rr_type', 'resolver_cc', 'resolver_AS', 'resolver_tag', 'count' ])
        return df

    def save_and_close(self, output_file):
        df = self.get_df()
        df.to_csv(output_file)

    def add_row(self, row):
        self.add(row['CC'], row['AS'], row['rr_type'], row['resolver_cc'], row['resolver_AS'], row['resolver_tag'], row['count'])

    def add_summary(self, csv_file):
        df = pd.read_csv(csv_file, sep=",", skipinitialspace=True)
        print(csv_file + ": " + str(df.shape[0]) + " lines.")
        df.apply(lambda row: self.add_row(row),axis=1)
        print("After loading: " + str(len(self.zombie_AS)) + " CC/AS.")

    def to_json(self, F):
        F.write("[")
        is_first_key = True
        for key in self.zombie_AS:
            if not is_first_key:
                F.write(",")
            is_first_key = False
            F.write("\n    { \"CC\": \"" + self.zombie_AS[key].query_cc +
                    "\", \"AS\": \"" + self.zombie_AS[key].query_AS +
                    "\", \"nb\": " + str(self.zombie_AS[key].nb) +  ", \"rrs\": [")
            is_first_rr = True
            for rr_type in self.zombie_AS[key].rr:
                if not is_first_rr:
                    F.write(",")
                is_first_rr = False
                F.write("\n        { \"RR\": \"" + rr_type + 
                        "\", \"nb\": " + str(self.zombie_AS[key].rr[rr_type].nb) + ",\"provs\":[")
                is_first_prov = True
                for prov in self.zombie_AS[key].rr[rr_type].prov:
                    if not is_first_prov:
                        F.write(",")
                    is_first_prov = False
                    F.write("{" +
                            "\"CC\":\"" + self.zombie_AS[key].rr[rr_type].prov[prov].resolver_cc + "\"," +
                            "\"AS\":\"" + self.zombie_AS[key].rr[rr_type].prov[prov].resolver_AS + "\"," +
                            "\"tag\":\"" + self.zombie_AS[key].rr[rr_type].prov[prov].resolver_tag + "\"," +
                            "\"nb\":" + str(self.zombie_AS[key].rr[rr_type].prov[prov].nb) + "}")
                F.write("]}")
            F.write("]}")
        F.write("]")





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
            zp = zombie_parse(self.ip2a4, self.ip2a6, self.as_names)
            nb_events = zp.load_log(self.source_file, time_start=self.time_start)
            print(self.source_file + ": loaded " + str(nb_events) + " events at " + str(time.time() - self.time_start))
            sys.stdout.flush()
            zp.save_and_close(self.output_file)
        else:
            print(self.output_file + ": already exists.")
            sys.stdout.flush()

def load_bucket(bucket):
    bucket.load()
    return True

# usage

def usage():
    print("Usage: rsv_zombie_p output_dir temp_dir <input_files>")



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
        output_file = os.path.join(temp_dir, "zombies-" + item + ".csv")
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
    
    zombie_p_total = zombie_parse(ip2a4, ip2a6, as_names)
    for bucket in bucket_list:
        zombie_p_total.add_summary(bucket.output_file)

    # zombie_p_file = os.path.join(output_dir, "zombies.csv")
    # df = zombie_p_total.get_df()
    # df.to_csv(zombie_p_file)

    zombies_json_file = os.path.join(output_dir, "zombies.json.bz2")
    with bz2.open(zombies_json_file, "wt") as F:
        zombie_p_total.to_json(F)