# Create a csv file with one row per ASN or group of ASN with the two hours
# buckets starting at 00:30 UTC with buckets, retaining only queries that are
# - Query time less than 30 seconds after AD time stamps
# - Query RR = A, AAAA or HTTPS
# - Not coming from AS0
#
# For each query, we need RR_TYPE, and sender type: ISP, PDNS or Other.
# At this stage, we do not distinguish between different PDNS
#
# We will tabulate per ISP (CC + AS), and for each ISP per 2 hours
# time slice, the second time slice starting at 00:30 UTC the first day.
#
# We want to tabulate 9 columns, 3 each for each record type:
# - Number of UIDs for which we receive a query for that RR_type from ISP
# - Number of UIDs for which we receive a query for that RR_type from PDNS
# - Number of UIDs for which we receive a query for that RR_type from Other.
# We count each UID at most once in each category. For example, if we receive
# 3 queries for RR_Type=A from the ISP, that counts for just 1 UID for A from ISP.
#
# We also want to count the total number of UIDs overall, and two sets
# of totals:
#
# - Total number of UIDs with at least one Query for the RR_type, per RR_Type
#   (this will be lower than the sum of ISP+PDNS+Other for that RR_type,
#    because it is an OR, not a Plus)
# - Total number of UIDs with at least one query from a given source
#   (this will be lower than the sum of A-AAAA-HTTPS for teh source,
#    because it is an OR, not a Plus)
#
# We can present these totals as a 4x4 matrix:
#  from    | All          | ISP      | PDNS      | Others
#  --------|--------------|----------|-----------|-----------
#  overall | nb_uids      | uids_ISP | uids_PDNS |uids_others
#  A       | nb_uids_A    | A_ISP    | A_PDNS    |A_others
#  AAAA    | nb_uids_AAAA | AAAA_ISP | AAAA_PDNS |AAAA_others
#  HTTPS   | nb_uids_HTTPS| HTTPS_ISP| HTTPS_PDNS|HTTPS_others
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
import open_rsv

def usage():
    print("Usage: python rsv_first_flux.py <csv_file> <log_file>\n")
    print("This script will parse the log file, extract data per AS and time slice,")
    print("and save the parsed data in the csv file.")

prov_names = [ 'isp', 'pdns', 'others' ]
rr_names = [ 'A', 'AAAA', 'HTTPS' ]

class flux_uid_rr:
    def __init__(self):
        self.has_prov = [ False, False, False ]
        self.has_rr = False

class flux_uid:
    def __init__(self):
        self.per_RR = [ flux_uid_rr(), flux_uid_rr(), flux_uid_rr() ]
        self.has_prov = [ False, False, False ]

    def update(self, rr_type, resolver_tag):
        i_rr = 2
        if rr_type == 'A':
            i_rr = 0
        elif rr_type == 'AAAA':
            i_rr = 1

        i_prov = 2
        if resolver_tag in rsv_log_parse.tag_isp_set:
            i_prov = 0
        elif resolver_tag in rsv_log_parse.tag_public_set:
            i_prov = 1
        is_new_rr = not self.per_RR[i_rr].has_rr
        is_new_prov = not self.has_prov[i_prov]
        is_new_prov_rr = not self.per_RR[i_rr].has_prov[i_prov]
        self.per_RR[i_rr].has_rr = True
        self.has_prov[i_prov] = True
        self.per_RR[i_rr].has_prov[i_prov] = True
        return i_rr, i_prov, is_new_rr, is_new_prov, is_new_prov_rr

class flux_cc_as_rr:
    def __init__(self):
        self.per_prov = [ 0, 0, 0 ]
        self.total = 0

class flux_cc_as:
    def __init__(self, query_cc, query_AS, slice_start, slice_duration, flux_file, is_first=False):
        self.query_cc = query_cc
        self.query_AS = query_AS
        self.slice_start = slice_start
        self.slice_duration = slice_duration
        self.flux_file = flux_file
        self.slice_number = 0
        self.saved_slices = 0
        self.saved_uids = 0
        self.uids = dict()
        self.per_prov = [ 0, 0, 0 ]
        self.per_rr = [ flux_cc_as_rr(), flux_cc_as_rr(), flux_cc_as_rr() ]


        if is_first:
            self.previous_slice = flux_cc_as(self.query_cc, self.query_AS,
                                             self.slice_start - self.slice_duration, 
                                             self.slice_duration, flux_file)
            self.previous_slice.should_save = False
            self.slice_number += 1

    def init_next_slice(self):
        # Copy the values in the previous slice, 
        # because we may update the "border" uids in the next 30 seconds
        self.previous_slice.slice_start = self.slice_start
        for i_prov in range(0,3):
            self.previous_slice.per_prov[i_prov] = self.per_prov[i_prov]
        for i_rr in range(0,3):
            self.previous_slice.per_rr[i_rr].total = \
                self.per_rr[i_rr].total
            for i_prov in range(0,3):
                self.previous_slice.per_rr[i_rr].per_prov[i_prov] = \
                    self.per_rr[i_rr].per_prov[i_prov]
        self.previous_slice.uids = self.uids
        self.previous_slice.should_save = True
        self.previous_slice.slice_number = self.slice_number

        # Reset the values of the current slice to zero
        self.slice_start += self.slice_duration
        for i_prov in range(0,3):
            self.per_prov[i_prov] = 0
        for i_rr in range(0,3):
            self.per_rr[i_rr].total = 0
            for i_prov in range(0,3):
                self.per_rr[i_rr].per_prov[i_prov] = 0
        self.uids = dict()
        self.should_save = True
        self.slice_number += 1

    def get_header():
        s = "CC,AS,start,uids"
        for i_prov in range(0,3):
            s += ',' + "nb_" + prov_names[i_prov]
        for i_rr in range(0,3):
            prefix = "nb_" + rr_names[i_rr]
            s += ',' + prefix
            prefix += '_'
            for i_prov in range(0,3):
                s += ',' + prefix + prov_names[i_prov] 
        s += "\n"
        return s

    def save_to_file(self):
        self.saved_slices += 1
        self.saved_uids += len(self.uids)

        s = self.query_cc + "," + self.query_AS + "," + str(self.slice_start) + ","
        s += str(len(self.uids))
        for i_prov in range(0,3):
            s += ',' + str(self.per_prov[i_prov])
        for i_rr in range(0,3):
            s += ',' + str(self.per_rr[i_rr].total)
            for i_prov in range(0,3):
                s += ',' + str(self.per_rr[i_rr].per_prov[i_prov])
        s += "\n"
        self.flux_file.write(s)

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

    def tabulate_known_query(self, r_uid, rr_type, resolver_tag):
        i_rr, i_prov, is_new_rr, is_new_prov, is_new_prov_rr = r_uid.update(rr_type, resolver_tag)
        if is_new_prov:
            self.per_prov[i_prov] += 1
        if is_new_rr:
            self.per_rr[i_rr].total += 1
        if is_new_prov_rr:
            self.per_rr[i_rr].per_prov[i_prov] += 1

    def add_query(self, uid, query_time, rr_type, resolver_tag):
        if query_time >= self.slice_start + self.slice_duration:
            self.save_to_file_and_rotate()
        if uid in self.previous_slice.uids:
            self.previous_slice.tabulate_known_query(self.previous_slice.uids[uid], rr_type, resolver_tag)
        else:
            if not uid in self.uids:
                self.uids[uid] = flux_uid()
            self.tabulate_known_query(self.uids[uid], rr_type, resolver_tag)

class flux_log:
    def __init__(self, slice_duration, initial_gap, ip2a4, ip2a6, as_names, flux_file):
        self.cc_as_dict=dict()
        self.first_slice_start = 0
        self.initial_gap = initial_gap
        self.slice_duration = slice_duration
        self.ip2a4 = ip2a4
        self.ip2a6 = ip2a6
        self.as_names = as_names
        self.flux_file = flux_file
        s = flux_cc_as.get_header()
        flux_file.write(s)

    def add_cc_as(self, query_cc, query_AS):
        key = query_cc + "-" + query_AS
        if not key in self.cc_as_dict:
            self.cc_as_dict[key] = flux_cc_as(query_cc, query_AS, self.first_slice_start, 
                                               self.slice_duration, self.flux_file, is_first=True)
        return key

    def add_query(self, uid, query_time, query_cc, query_AS, resolver_tag, rr_type):
        if self.first_slice_start == 0:
            slice_nb = int(query_time/self.slice_duration)
            if self.initial_gap > 0:
                self.first_slice_start = (slice_nb - 1)*self.slice_duration + self.initial_gap
            else:
                self.first_slice_start = slice_nb*self.slice_duration
            print("Start time: " + str(self.first_slice_start))
        key = self.add_cc_as(query_cc, query_AS)
        self.cc_as_dict[key].add_query(uid, query_time, rr_type, resolver_tag)

    def load_flux_log(self, log_file, log_threshold=15625, time_start=0):
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
                    if x.resolver_AS != 'AS0':
                        self.add_query(x.query_user_id, x.query_time, x.query_cc, x.query_AS, x.resolver_tag, x.rr_type)
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
        self.flux_file.close()


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

    with open(output_file, "w") as flux_file:
        rcl = flux_log(7200, 1800, ip2a4, ip2a6, as_names, flux_file)
        for source_file in source_files:
            nb_events = rcl.load_flux_log(source_file, time_start=time_start)
            print(source_file + ": loaded " + str(nb_events) + " events at " + str(time.time() - time_start))
            sys.stdout.flush()
        rcl.save_and_close()
        print("Saved output in " + output_file)


