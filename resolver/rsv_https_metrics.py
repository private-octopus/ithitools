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
        self.has_HTTPS = False
        self.has_dup_HTTPS = False
        self.first_time_HTTPS = 0
        self.has_ISP_for_HTTPS = False
        self.has_PDNS_for_HTTPS = False
        self.has_other_for_HTTPS = False

    def update(self, query_time, rr_type, resolver_tag):
        delta_t = query_time - self.first_time
        is_dup = False
        dup_index = 0
        if delta_t < 30 and rr_type == 'HTTPS':
            self.has_HTTPS = True
            is_dup = True
            if delta_t < 0.5:
                if resolver_tag in rsv_log_parse.tag_isp_set:
                    self.has_ISP_for_HTTPS = True
                elif resolver_tag in rsv_log_parse.tag_public_set:
                    self.has_PDNS_for_HTTPS = True
                else:
                    self.has_other_for_HTTPS = True
                if self.has_other_for_HTTPS:
                    dup_index = 3
                elif self.has_PDNS_for_HTTPS:
                    if self.has_ISP_for_HTTPS:
                        dup_index = 2
                    else:
                        dup_index = 1
                else:
                    dup_index = 0
            else:
                dup_index = 4
        return delta_t, is_dup, dup_index

class recap_cc_as:
    def __init__(self, query_cc, query_AS, slice_start, slice_duration, recap_file, is_first=False):
        self.query_cc = query_cc
        self.query_AS = query_AS
        self.slice_start = slice_start
        self.slice_duration = slice_duration
        self.recap_file = recap_file
        self.first_by_isp = 0
        self.first_by_pdns = [ 0, 0, 0, 0, 0, 0, 0 ]
        self.first_by_others = 0
        self.nb_HTTPS = 0
        self.nb_HTTPS_dup = 0
        self.nb_dup_HTTPS = 0
        # vector of duplicates:
        # ISP only
        # PDNS only
        # ISP + PNDS
        # ISP + other
        # others
        # long, if t > 0.5
        self.dups_HTTPS =  [ 0, 0, 0, 0, 0, 0 ]
        self.zombie_1 = 0
        self.zombie_2 = [ 0, 0, 0, 0]
        self.first_3s = 0
        self.first_10s = 0
        self.sum_first_delay = 0
        self.max_first_delay = 0
        self.uids = dict()
        self.should_save = True
        self.slice_number = 0
        if is_first:
            self.previous_slice = recap_cc_as(self.query_cc, self.query_AS,
                                             self.slice_start - self.slice_duration, 
                                             self.slice_duration, recap_file)
            self.previous_slice.should_save = False
            self.slice_number += 1

        self.saved_slices = 0
        self.saved_uids = 0


    def init_next_slice(self):
        # Copy the values in the previous slice, 
        # because we may update the "border" uids in the next 30 seconds
        self.previous_slice.slice_start = self.slice_start
        self.previous_slice.first_by_isp = self.first_by_isp
        for i in range(0,7):
            self.previous_slice.first_by_pdns[i] = self.first_by_pdns[i]
        self.previous_slice.first_by_others = self.first_by_others
        self.previous_slice.nb_HTTPS = self.nb_HTTPS
        self.previous_slice.nb_HTTPS_dup = self.nb_HTTPS_dup
        self.previous_slice.nb_dup_HTTPS = self.nb_dup_HTTPS
        for i in range(0,6):
            self.previous_slice.dups_HTTPS[i] =  self.dups_HTTPS[i]
        self.previous_slice.zombie_1 = self.zombie_1
        for i in range(0,4):
            self.previous_slice.zombie_2[i] = self.zombie_2[i]
        self.previous_slice.first_3s = self.first_3s
        self.previous_slice.first_10s = self.first_10s
        self.previous_slice.sum_first_delay = self.sum_first_delay
        self.previous_slice.max_first_delay = self.max_first_delay
        self.previous_slice.uids = self.uids
        self.previous_slice.slice_number = self.slice_number
        self.previous_slice.should_save = True

        # Reset the values of the current slice to zero
        self.slice_start += self.slice_duration
        self.first_by_isp = 0
        self.first_by_pdns = [ 0, 0, 0, 0, 0, 0, 0 ]
        self.first_by_others = 0
        self.nb_HTTPS = 0
        self.nb_HTTPS_dup = 0
        self.dups_HTTPS =  [ 0, 0, 0, 0, 0, 0]
        self.zombie_1 = 0
        self.zombie_2 = [ 0, 0, 0, 0]
        self.first_3s = 0
        self.first_10s = 0
        self.sum_first_delay = 0
        self.max_first_delay = 0
        self.uids = dict()
        self.should_save = True
        self.slice_number += 1

    def get_header():
        s = "CC,AS,start,uids,first_isp,"
        for pnds_name in pdns_names:
            s += pnds_name  + ','
        s += 'first_others,nb_HTTPS,nb_HTTPS_dup,nb_dup_HTTPS,dups_isp,dups_pdns,isp_pdns,isp_others,dups_others,dups_long,'
        s += 'zombie_1,zombie_2,z_ISP,z_PDNS,z_others,first_3s,first_10s,sum_delay,max_delay' + '\n'
        return s

    def save_to_file(self):
        self.saved_slices += 1
        self.saved_uids += len(self.uids)

        s = self.query_cc + "," + self.query_AS + "," + str(self.slice_start) + ","
        s += str(len(self.uids)) + ','
        s += str(self.first_by_isp) + ','
        for pnds_total in self.first_by_pdns:
            s += str(pnds_total) + ','
        s += str(self.first_by_others) + ',' + str(self.nb_HTTPS) + ','
        s += str(self.nb_HTTPS_dup) + ','
        s += str(self.nb_dup_HTTPS) + ','
        for dups_total in self.dups_HTTPS:
            s += str(dups_total) + ','
        s += str(self.zombie_1) + ','
        for z in self.zombie_2:
            s += str(z) + ','
        s += str(self.first_3s) + ',' + str(self.first_10s) + ','
        s += str(self.sum_first_delay) + ',' + str(self.max_first_delay) + '\n'
        self.recap_file.write(s)

    def save_to_file_and_rotate(self):
        if self.previous_slice.should_save:
            self.previous_slice.save_to_file()
        elif self.previous_slice.slice_number != 0:
            print("For " + self.query_cc + "-" + self.query_AS + ", skipped slice " + str(self.previous_slice.slice_number))
        elif self.previous_slice.slice_number + 1 != self.slice_number:
            print("For " + self.query_cc + "-" + self.query_AS + ", bad slice, " +
                 str(self.previous_slice.slice_number) + ", " +
                 str(self.slice_number))

        self.init_next_slice()

    def flush_to_file(self):
        if self.previous_slice.should_save:
            self.previous_slice.save_to_file()
        elif self.previous_slice.slice_number != 0:
            print("For " + self.query_cc + "-" + self.query_AS + ", skipped slice " + str(self.previous_slice.slice_number))
        self.save_to_file()

    def add_first_tag(self, resolver_tag):
        if resolver_tag in rsv_log_parse.tag_isp_set:
            self.first_by_isp += 1
        elif resolver_tag in pdns_dict:
            self.first_by_pdns[pdns_dict[resolver_tag]] += 1
        else:
            self.first_by_others += 1

    def tabulate_duplicate_HTTPS(self, r_uid, delta_t, resolver_tag):
        # duplicate A query for that UID
        self.nb_dup_HTTPS += 1
        if delta_t > 0.5:
            # tabulate in the long category, 0.5 to 30s
            self.dups_HTTPS[5] += 1
        else:
            # tabulate in one of the short categories
            if resolver_tag in rsv_log_parse.tag_isp_set:
                r_uid.has_ISP_for_HTTPS = True
            elif resolver_tag in rsv_log_parse.tag_public_set:
                r_uid.has_PDNS_for_HTTPS = True
            else:
                r_uid.has_other_for_HTTPS = True

            if r_uid.has_other_for_HTTPS:
                if r_uid.has_ISP_for_HTTPS and not r_uid.has_PDNS_for_HTTPS:
                    # if no PNDS but ISP and other, tabulate as isp_other
                    self.dups_HTTPS[3] += 1
                else:
                    # catch all category.
                    # if other and PDNS, or other and ISP and PDNS, or other alone
                    self.dups_HTTPS[4] += 1
            elif r_uid.has_PDNS_for_HTTPS:
                if r_uid.has_ISP_for_HTTPS:
                    # if no others but PDNS and ISP, tabulate as isp_pdns
                    self.dups_HTTPS[2] += 1
                else:
                    # if no others and no ISP, tabulate as dups_pdns
                    self.dups_HTTPS[1] += 1
            else:
                # if no others and no PDNS, tabulate as dups_isp
                self.dups_HTTPS[0] += 1

    def tabulate_known_query(self, r_uid, query_time, rr_type, resolver_tag):
        # Tabulate all the queries for which we saw the first UID request
        # To make sure totals add to 100, they are tabulated in the slice
        # where the UID first appeared.
        delta_first = query_time - r_uid.first_time
        if delta_first > 30:
            self.zombie_1 += 1
        if rr_type == 'HTTPS':
            # Detect whether this is new query or a repeat
            if not r_uid.has_HTTPS:
                r_uid.has_HTTPS = True
                self.nb_HTTPS += 1
                r_uid.first_time_HTTPS = query_time
                is_dup = False
                delta_t = 0
            else:
                is_dup = True
                delta_t = query_time - r_uid.first_time_HTTPS
            # Regardless of duplicate, set the flags indicating
            # which kind of ISP was received
            if resolver_tag in rsv_log_parse.tag_isp_set:
                r_uid.has_ISP_for_HTTPS = True
            elif resolver_tag in rsv_log_parse.tag_public_set:
                r_uid.has_PDNS_for_HTTPS = True
            else:
                r_uid.has_other_for_HTTPS = True
            # process the duplicates
            if is_dup:
                if not r_uid.has_dup_HTTPS:
                    r_uid.has_dup_HTTPS = True
                    self.nb_HTTPS_dup += 1
                self.tabulate_duplicate_HTTPS(r_uid, delta_t, resolver_tag)

    def add_query(self, uid, query_time, rr_type, resolver_tag, query_ad_time):
        is_counted = False

        if query_time >= self.slice_start + self.slice_duration:
            self.save_to_file_and_rotate()
        delta_first = query_time - query_ad_time
        if delta_first >= 30:
            self.zombie_2[0] += 1
            if resolver_tag in rsv_log_parse.tag_isp_set:
                self.zombie_2[1] += 1
            elif resolver_tag in rsv_log_parse.tag_public_set:
                self.zombie_2[2] += 1
            else:
                self.zombie_2[3] += 1

        if uid in self.previous_slice.uids:
            self.previous_slice.tabulate_known_query(self.previous_slice.uids[uid], query_time, rr_type, resolver_tag)
        elif delta_first < 30:
            if not uid in self.uids:
                self.uids[uid] = recap_uid(query_time, rr_type, resolver_tag)
                self.add_first_tag(resolver_tag)
                self.sum_first_delay += delta_first
                if self.max_first_delay < delta_first:
                    self.max_first_delay = delta_first
                if delta_first > 10:
                    self.first_10s += 1
                elif delta_first > 3:
                    self.first_3s += 1
            self.tabulate_known_query(self.uids[uid], query_time, rr_type, resolver_tag)

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
            self.cc_as_dict[key] = recap_cc_as(query_cc, query_AS, self.first_slice_start, 
                                               self.slice_duration, self.recap_file, is_first=True)
        return key

    def add_query(self, uid, query_time, query_cc, query_AS, resolver_tag, rr_type, query_ad_time):
        if self.first_slice_start == 0:
            slice_nb = int(query_time/self.slice_duration)
            if self.initial_gap > 0:
                self.first_slice_start = (slice_nb - 1)*self.slice_duration + self.initial_gap
            else:
                self.first_slice_start = slice_nb*self.slice_duration
            print("Start time: " + str(self.first_slice_start))
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
                if x.filter(rr_types=['HTTPS'], experiment=['0du'], query_delay=1000000000):
                    x.set_resolver_AS(self.ip2a4, self.ip2a6, self.as_names)
                    if x.resolver_AS != 'AS0' and x.rr_type == 'HTTPS':
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
            self.cc_as_dict[key].flush_to_file()
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


