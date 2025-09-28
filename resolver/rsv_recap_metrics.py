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

def usage():
    print("Usage: python rsv_first_recap.py <csv_file> <log_file>\n")
    print("This script will parse the log file, extract data per AS and time slice,")
    print("and save the parsed data in the csv file.")

pdns_dict = {
    'googlepdns':0,
    'cloudflare':1,
    'opendns':2,
    'quad9':3,
    'level3':4,
    'neustar':5,
    'he':6
}
pdns_names = [
    'googlepdns',
    'cloudflare',
    'opendns',
    'quad9',
    'level3',
    'neustar',
    'he'
]

class recap_uid:
    def __init__(self, query_time, rr_type, resolver_tag):
        self.first_time = query_time
        self.has_https = False
        self.has_A = False
        self.has_AAAA = False
        self.has_ISP_for_A = False
        self.has_PNDS_for_A = False
        self.has_other_for_A = False

        if rr_type == 'A':
            self.has_A = True
            if resolver_tag in rsv_log_parse.tag_isp_set:
                self.has_ISP_for_A = True
            elif resolver_tag in rsv_log_parse.tag_public_set:
                self.has_PNDS_for_A = True
            else:
                has_other_for_A = True

        elif rr_type == 'AAAA':
            self.has_AAAA = True
        elif rr_type == 'HTTPS':
            self.has_HTTPS = True

    def update(self, query_time, rr_type, resolver_tag):
        delta_t = query_time - self.first_time
        is_dup = False
        dup_index = 0
        if delta_t < 30 and rr_type == 'A':
            self.has_A = True
            is_dup = True
            if delta_t < 0.5:
                if resolver_tag in rsv_log_parse.tag_isp_set:
                    self.has_ISP_for_A = True
                elif resolver_tag in rsv_log_parse.tag_public_set:
                    self.has_PNDS_for_A = True
                else:
                    self.has_other_for_A = True
                if self.has_other_for_A:
                    dup_index = 3
                elif self.has_PNDS_for_A:
                    if self.has_ISP_for_A:
                        dup_index = 2
                    else:
                        dup_index = 1
                else:
                    dup_index = 0
            else:
                dup_index = 4
            
        return delta_t, is_dup, dup_index

class recap_cc_as:
    def __init__(self, query_cc, query_AS, slice_start, slice_duration, recap_file):
        self.query_cc = query_cc
        self.query_AS = query_AS
        self.slice_start = slice_start
        self.slice_duration = slice_duration
        self.recap_file = recap_file
        self.first_by_isp = 0
        self.first_by_pdns = [ 0, 0, 0, 0, 0, 0, 0 ]
        self.first_by_others = 0
        self.nb_https = 0
        self.nb_A = 0
        self.dups_A =  [ 0, 0, 0, 0, 0]
        self.zombie_1 = 0
        self.zombie_2 = 0
        self.sum_first_delay = 0
        self.max_first_delay = 0
        
        self.uids = dict()
        self.old_uids = dict()

    def init_next_slice(self):
        self.slice_start += self.slice_duration
        self.first_by_isp = 0
        self.first_by_pdns = [ 0, 0, 0, 0, 0, 0, 0 ]
        self.first_by_others = 0
        self.nb_https = 0
        self.nb_A = 0
        self.dups_A =  [ 0, 0, 0, 0, 0]
        self.zombie_1 = 0
        self.zombie_2 = 0
        self.sum_first_delay = 0
        self.max_first_delay = 0
        self.old_uids = self.uids
        self.uids = dict()

    def get_header():
        s = "CC,AS,start,uids,first_isp,"
        for pnds_name in pdns_names:
            s += pnds_name  + ','
        s += 'first_others,nb_https,nb_A,dups_isp,dups_pdns,dups_both,dups_others,dups_long,'
        s += 'zombie_1,zombie_2,sum_delay,max_delay' + '\n'
        return s

    def save_to_file(self):
        s = self.query_cc + "," + self.query_AS + "," + str(self.slice_start) + ","
        s += str(len(self.uids)) + ','
        s += str(self.first_by_isp) + ','
        for pnds_total in self.first_by_pdns:
            s += str(pnds_total) + ','
        s += str(self.first_by_others) + ',' + str(self.nb_https) + ','
        s += str(self.nb_A) + ','
        for dups_total in self.dups_A:
            s += str(dups_total) + ','
        s += str(self.zombie_1) + ',' + str(self.zombie_2) + ','
        s += str(self.sum_first_delay) + ',' + str(self.max_first_delay) + '\n'
        self.recap_file.write(s)

    def add_first_tag(self, resolver_tag):
        if resolver_tag in rsv_log_parse.tag_isp_set:
            self.first_by_isp += 1
        elif resolver_tag in pdns_dict:
            self.first_by_pdns[pdns_dict[resolver_tag]] += 1
        else:
            self.first_by_others += 1

    def add_query(self, uid, query_time, rr_type, resolver_tag, query_ad_time):
        if query_time >= self.slice_start + self.slice_duration:
            self.save_to_file()
            self.init_next_slice()
        is_dup = False
        delta_t = 0
        is_processed = False
        if uid in self.old_uids:
            has_https = self.old_uids[uid].has_https
            if rr_type == 'HTTPS' and not has_https:
                self.nb_https += 1
            delta_t, is_dup, dup_index = self.old_uids[uid].update(query_time, rr_type, resolver_tag)
        elif not uid in self.uids:
            is_processed = True
            delta_first = query_time - query_ad_time
            if delta_first < 30:
                self.uids[uid] = recap_uid(query_time, rr_type, resolver_tag)
                self.add_first_tag(resolver_tag)
                self.sum_first_delay += delta_first
                if self.max_first_delay < delta_first:
                    self.max_first_delay = delta_first
                if rr_type == 'A':
                    self.nb_A += 1
                if rr_type == 'HTTPS':
                    self.nb_https += 1
            else:
                self.zombie_2 += 1
        else:
            has_https = self.uids[uid].has_https
            if rr_type == 'HTTPS' and not has_https:
                self.nb_https += 1
            delta_t, is_dup, dup_index = self.uids[uid].update(query_time, rr_type, resolver_tag)

        if not is_processed:
            if delta_t < 30:
                if rr_type == 'A':
                    self.nb_A += 1
                    if is_dup:
                        self.dups_A[dup_index] += 1
            else:
                self.zombie_1 += 1

class recap_log:
    def __init__(self, slice_duration, initial_gap, ip2a4, ip2a6, as_names, recap_file):
        self.cc_as_dict=dict()
        self.first_slice_start = 0
        self.initial_gap = initial_gap
        self.slice_duration = slice_duration
        self.ip2a4 = ip2a4
        self.ip2a6 = ip2a6
        self.as_names = as_names
        self.recap_file = recap_file
        s = recap_cc_as.get_header()
        recap_file.write(s)

    def add_cc_as(self, query_cc, query_AS):
        key = query_cc + "-" + query_AS
        if not key in self.cc_as_dict:
            # print("Adding: " + key)
            self.cc_as_dict[key] = recap_cc_as(query_cc, query_AS, self.first_slice_start, 
                                               self.slice_duration, self.recap_file)
        return key

    def add_query(self, uid, query_time, query_cc, query_AS, resolver_tag, rr_type, query_ad_time):
        if self.first_slice_start == 0:
            slice_nb = int(query_time/self.slice_duration)
            if self.initial_gap > 0:
                self.first_slice_start = (slice_nb - 1)*self.slice_duration + self.initial_gap
            else:
                self.first_slice_start = slice_nb*self.slice_duration
        key = self.add_cc_as(query_cc, query_AS)
        self.cc_as_dict[key].add_query(uid, query_time, rr_type, resolver_tag, query_ad_time)

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
                if x.filter(rr_types=['A', 'AAAA', 'HTTPS'], experiment=['0du'], query_delay=1000000000):
                    x.set_resolver_AS(self.ip2a4, self.ip2a6, self.as_names)
                    self.add_query(x.query_user_id, x.query_time, x.query_cc, x.query_AS, x.resolver_tag, x.rr_type, x.query_ad_time)
                    nb_events += 1
                    if (nb_events%lth) == 0:
                        new_time = time.time() - time_start
                        print(source_file + ": loaded " + str(nb_events) + " events at " + str(new_time))
                        sys.stdout.flush()
                        if lth < 1000000:
                            lth *= 2
        return nb_events

    def save_and_close(self):
        for key in self.cc_as_dict:
            self.cc_as_dict[key].save_to_file()
        self.recap_file.close()


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


