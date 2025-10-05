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

class as_data:
    def __init__(self):
        self.total_uids = 0
        self.uids = set()
        self.r_as_list = dict()
        self.r_as_total = dict()

    def add_query(self, resolver_AS, uid):
        if not uid in self.uids:
            self.uids.add(uid)
        if not resolver_AS in self.r_as_list:
            self.r_as_list[resolver_AS] = set()
        if not uid in self.r_as_list[resolver_AS]:
            self.r_as_list[resolver_AS].add(uid)

    def flatten(self):
        for resolver_AS in self.r_as_list:
            if not resolver_AS in self.r_as_total:
                self.r_as_total[resolver_AS] = len(self.r_as_list[resolver_AS])
            else:
                self.r_as_total[resolver_AS] += len(self.r_as_list[resolver_AS])
        self.r_as_list = dict()
        self.total_uids += len(self.uids)
        self.uids = set()
        
    def add_summary(self, resolver_AS, nb_uid):
        if not resolver_AS in self.r_as_total:
            self.r_as_total[resolver_AS] = nb_uid
        else:
            self.r_as_total[resolver_AS] += nb_uid

    def get_summary_headers():
        return [ 'CC', 'AS', 'resolver_AS', 'nb_uids' ]
    
    
    def get_summary_table(self, query_cc, query_AS):
        t = []
        for resolver_AS in self.r_as_total:
            r = [ query_cc, query_AS, resolver_AS, self.r_as_total[resolver_AS] ]
            t.append(r)
        return t

    def get_headers():
        return [ 'CC', 'AS', 'AS_name', 'uids',
            'RAS_1', 'RAS_1_name', 'RAS_1_uids',
            'RAS_2', 'RAS_2_name', 'RAS_2_uids',
            'RAS_3', 'RAS_3_name', 'RAS_3_uids',
            'others' ]


    def get_row(self, key, as_names):
        kp = key.split('-')
        if len(kp) != 2:
            print("What? " + key)
            exit(-1)
        r = [ kp[0] , kp[1] ]
        name = as_names.name(kp[1])
        r.append(name)
        r.append(self.total_uids)

        t = []
        for resolver_AS in self.r_as_total:
            t.append([ resolver_AS, self.r_as_total[resolver_AS]])
        t.sort(key=lambda x:x[1], reverse=True)

        for i in range(0, 3):
            if i < len(t):
                l = t[i]
                r.append(l[0])
                r_name = as_names.name(l[0])
                r.append(r_name)
                r.append(l[1])
            else:
                r.append("")
                r.append("")
                r.append(0)

        others = 0
        if len(t) > 3:
            for i in range(3,len(t)):
                l = t[i]
                others += l[1]
        r.append(others)
        return r

rsv_spike_tags = [ "Cloud", "Same_CC", "Other_cc" ]
rsv_spike_set = set(rsv_spike_tags)

class rsv_spike_log:
    def __init__(self,ip2a4, ip2a6, as_names):
        self.as_list = dict()
        self.ip2a4 = ip2a4
        self.ip2a6 = ip2a6
        self.as_names = as_names

    def add_cc_as(self, x):
        key = str(x['CC']) + "-" + str(x['AS'])
        if not key in self.as_list:
            self.as_list[key] = as_data()

    def load_cc_as_list(self, csv_file):
        df = pd.read_csv(csv_file,skipinitialspace=True)
        df.apply(lambda x: self.add_cc_as(x), axis=1)

    def add_query(self, query_cc, query_AS, resolver_AS, uid):
        key = query_cc + '-' + query_AS
        if key in self.as_list:
            self.as_list[key].add_query(resolver_AS, uid)

    def load_log(self, saved_file):
        nb_events = 0
        with open(saved_file, newline='') as csvfile:
            rsv_reader = csv.reader(csvfile, delimiter=',', quotechar='"')
            is_first = True
            is_second = True
            header_row = [ 'query_cc', 'query_AS', 'query_user_id', 'resolver_AS', 'resolver_tag' ]
            header_index = [ -1, -1, -1, -1, -1 ]

            for row in rsv_reader:
                if is_first:
                    # print(",".join(row))
                    for i in range(0, len(header_row)):
                        for x in range(0, len(row)):
                            if row[x] == header_row[i]:
                                # print("row[" + str(x) + "](" + str(row[x]) + ") == " + header_row[i])
                                header_index[i] = x
                                break
                            #else:
                            #    print(row[x] + " != " + header_row[i])
                        if header_index[i] < 0:
                            print("Could not find " + header_row[i] + " in " + ','.join(row))
                            exit(-1)
                    is_first = False
                else:
                    nb_events += 1
                    if (is_second):
                        #print(",".join(row))
                        #for i in range(0, len(header_row)):
                        #    print(str(i) + ": x[" + header_row[i] + "] = " + str(row[header_index[i]]))
                        is_second = False
                    query_cc = row[header_index[0]]
                    query_AS = row[header_index[1]]
                    uid = row[header_index[2]]
                    resolver_AS = row[header_index[3]]
                    resolver_tag = row[header_index[4]]
                    if resolver_AS == "AS13335":
                        resolver_tag = "cloudflare"
                    if resolver_tag in rsv_spike_set:
                        self.add_query(query_cc, query_AS, resolver_AS, uid)
        return nb_events

    
    def load_recap_log(self, log_file, log_threshold=15625, time_start=0):
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
                    resolver_tag = x.resolver_tag
                    if not (resolver_tag in rsv_log_parse.tag_isp_set) and \
                       not (resolver_tag in rsv_log_parse.tag_public_set):
                        # classify as other 
                            self.add_query(x.query_cc, x.query_AS, x.resolver_AS, x.query_user_id)
                nb_events += 1
                if (nb_events%lth) == 0:
                    new_time = time.time() - time_start
                    print(log_file + ": loaded " + str(nb_events) + " events at " + str(new_time))
                    sys.stdout.flush()
                    if lth < 1000000:
                        lth *= 2
        return nb_events

    def flatten(self):
        for key in self.as_list:
            self.as_list[key].flatten()
            # print(key + ": " + str(len(self.as_list[key].r_as_total)) + ", " + str(self.as_list[key].total_uids))

    def get_df(self):
        t = []
        for key in self.as_list:
            if len(self.as_list[key].r_as_total) > 0:
                t.append(self.as_list[key].get_row(key, self.as_names))
            else:
                print(key + " r_as_total = " + str(len(self.as_list[key].r_as_total)))
        df = pd.DataFrame(t, columns=as_data.get_headers())
        return df

    def save_raw_data(self, file_path):
        t = []
        for key in self.as_list:
            kp = key.split('-')
            t += self.as_list[key].get_summary_table(kp[0], kp[1])
        df = pd.DataFrame(t, columns=as_data.get_summary_headers())
        df.to_csv(file_path)
        print("Saved " + str(df.shape[0]) + " lines to " + file_path)

    def add_raw_cc_as(self, x):
        key = str(x['CC']) + "-" + str(x['AS'])
        if not key in self.as_list:
            self.as_list[key] = as_data()
        self.as_list[key].add_summary(x['resolver_AS'], x['nb_uids'])


    def load_raw_data(self, csv_file):
        df = pd.read_csv(csv_file,skipinitialspace=True)
        print("Got " + str(df.shape[0]) + " records from " + csv_file)
        df.apply(lambda x: self.add_raw_cc_as(x), axis=1)

class file_bucket:
    def __init__(self, ip2a4, ip2a6, as_names, as_csv):
        self.as_csv = as_csv
        self.input_files = []
        self.file_path = ""
        self.rsl = rsv_spike_log(ip2a4, ip2a6, as_names)


    def load(self):
        # load the AS list
        time_start = time.time()
        self.rsl.load_cc_as_list(self.as_csv)
        print("Loaded: " + str(len(self.rsl.as_list)) + " ASes ")
        if len(self.rsl.as_list) == 0:
            exit(-1)
        for input_file in self.input_files:
            if input_file.endswith(".csv"):
                nb_events =self.rsl.load_log(input_file)
            else:
                nb_events = self.rsl.load_recap_log(input_file, time_start=time_start)
            print(input_file + ": found " + str(nb_events) + " events.")
            self.rsl.flatten()

    def save(self):
        self.rsl.save_raw_data(self.file_path)
 

def load_bucket(bucket):
    bucket.load()
    bucket.save()
    return True

def usage():
    print("Usage: python rsv_other_spike_stats.py  <output_dir>  <as_spike-csv> <input_file> ... <input_file>\n")
    print("This script will load the csv files,")
    print("and create an IP list for the ASes listed in as_spike-csv")

# main

if __name__ == "__main__":
    time_start = time.time()
    if len(sys.argv) < 4:
        usage()
        exit(-1)

    # parse the arguments
    output_dir = sys.argv[1]
    as_csv = sys.argv[2]
    
    input_files, has_error = rsv_arguments.parse_file_list(sys.argv[3:], [ ".csv", ".bz2" ])
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
        bucket = file_bucket(ip2a4, ip2a6, as_names, as_csv)
        step = int((len(input_files) - bucket_first + process_left - 1)/process_left)
        print("step: " + str(step))
        process_left -= 1
        bucket_next = min(bucket_first+step, len(input_files))
        bucket.input_files = input_files[bucket_first:bucket_next]
        print("bucket: " + str(bucket_first) + "," + str(bucket_next))
        bucket.bucket_id = bucket_id
        bucket.file_path = os.path.join(output_dir, "tmp_other_spike_" + str(bucket.bucket_id) + ".csv" )
        bucket_list.append(bucket)
        bucket_id += 1
        bucket_first = bucket_next

    nb_process = min(nb_process, len(bucket_list))
    # print("Will use " + str(nb_process) + " processes, " + str(len(bucket_list)) + " buckets")
    total_files = 0
    for bucket in bucket_list:
        total_files += len(bucket.input_files)
    print("%d files in %d buckets (%d .. %d), vs %d" %(total_files, len(bucket_list), len(bucket_list[0].input_files), len(bucket_list[len(bucket_list)-1].input_files), len(input_files)))


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
    rsl = rsv_spike_log(ip2a4, ip2a6, as_names)
    for bucket in bucket_list:
        rsl.load_raw_data(bucket.file_path)

    # save the results
    df = rsl.get_df()
    as_file = os.path.join(output_dir, "other_spike_as.csv" )
    df.to_csv(as_file)
    print("Report saved in " + as_file)