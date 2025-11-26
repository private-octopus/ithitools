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
                    if x.resolver_AS == "" or x.resolver_cc == "":
                        x.set_resolver_AS(self.ip2a4, self.ip2a6, self.as_names)
                    if x.resolver_AS != 'AS0':
                        self.add_query(x.query_user_id, x.query_time, x.query_cc, x.query_AS, x.resolver_tag, x.rr_type)
                        nb_events += 1
                        if (nb_events%lth) == 0:
                            new_time = time.time() - time_start
                            print(log_file + ": loaded " + str(nb_events) + " events at " + str(new_time))
                            sys.stdout.flush()
                            if lth < 1000000:
                                lth *= 2
        return nb_events

    def save_and_close(self):
        for key in self.cc_as_dict:
            self.cc_as_dict[key].flush_to_file()
        self.flux_file.close()


flux_columns = [
    "CC", "AS", "start", "uids", "nb_isp", "nb_pdns", "nb_others",
    "nb_A", "nb_A_isp", "nb_A_pdns", "nb_A_others",
    "nb_AAAA", "nb_AAAA_isp", "nb_AAAA_pdns", "nb_AAAA_others",
    "nb_HTTPS", "nb_HTTPS_isp", "nb_HTTPS_pdns", "nb_HTTPS_others",
]

class flux_row:
    def __init__(self, row):
        self.query_cc = row['CC']
        self.query_AS = row['AS']
        self.start = row['start']
        self.total_uids = row['uids']
        self.nb_isp = row['nb_isp']
        self.nb_pdns = row['nb_pdns']
        self.nb_others = row['nb_others']
        self.nb_A = row['nb_A']
        self.nb_A_isp = row['nb_A_isp']
        self.nb_A_pdns = row['nb_A_pdns']
        self.nb_A_others = row['nb_A_others']
        self.nb_AAAA = row['nb_AAAA']
        self.nb_AAAA_isp = row['nb_AAAA_isp']
        self.nb_AAAA_pdns = row['nb_AAAA_pdns']
        self.nb_AAAA_others = row['nb_AAAA_others']
        self.nb_HTTPS = row['nb_HTTPS']
        self.nb_HTTPS_isp = row['nb_HTTPS_isp']
        self.nb_HTTPS_pdns = row['nb_HTTPS_pdns']
        self.nb_HTTPS_others = row['nb_HTTPS_others']

    def add_row(self, row):
        self.total_uids += row['uids']
        self.nb_isp += row['nb_isp']
        self.nb_pdns += row['nb_pdns']
        self.nb_others += row['nb_others']
        self.nb_A += row['nb_A']
        self.nb_A_isp += row['nb_A_isp']
        self.nb_A_pdns += row['nb_A_pdns']
        self.nb_A_others += row['nb_A_others']
        self.nb_AAAA += row['nb_AAAA']
        self.nb_AAAA_isp += row['nb_AAAA_isp']
        self.nb_AAAA_pdns += row['nb_AAAA_pdns']
        self.nb_AAAA_others += row['nb_AAAA_others']
        self.nb_HTTPS += row['nb_HTTPS']
        self.nb_HTTPS_isp += row['nb_HTTPS_isp']
        self.nb_HTTPS_pdns += row['nb_HTTPS_pdns']
        self.nb_HTTPS_others += row['nb_HTTPS_others']

class flux_cc_as2:
    def __init__(self, query_cc, query_AS):
        self.query_cc = query_cc
        self.query_AS = query_AS
        self.slices = dict()
        self.total_uids = 0

    def add_row(self, row):
        self.total_uids += row['uids']
        start = row['start']
        if not start in self.slices:
            self.slices[start] = flux_row(row)
        else:
            self.slices[start].add_row(row)

    def get_columns():
        return flux_columns

    def save_file(self, file_name):
        with open(file_name, "wt") as F:
            headers = flux_cc_as2.get_columns()
            s = ""
            for header in headers:
                s += header + ","
            s += "\n"
            F.write(s)
            slice_list = list(self.slices.keys())
            slice_list.sort()
            for start in slice_list:
                r_row = self.slices[start]
                s = ""
                s += str(r_row.query_cc) + ","
                s += str(r_row.query_AS) + ","
                s += str(r_row.start) + ","
                s += str(r_row.total_uids) + ","
                s += str(r_row.nb_isp) + ","
                s += str(r_row.nb_pdns) + ","
                s += str(r_row.nb_others) + ","
                s += str(r_row.nb_A) + ","
                s += str(r_row.nb_A_isp) + ","
                s += str(r_row.nb_A_pdns) + ","
                s += str(r_row.nb_A_others) + ","
                s += str(r_row.nb_AAAA) + ","
                s += str(r_row.nb_AAAA_isp) + ","
                s += str(r_row.nb_AAAA_pdns) + ","
                s += str(r_row.nb_AAAA_others) + ","
                s += str(r_row.nb_HTTPS) + ","
                s += str(r_row.nb_HTTPS_isp) + ","
                s += str(r_row.nb_HTTPS_pdns) + ","
                s += str(r_row.nb_HTTPS_others) + ","
                s += "\n"
                F.write(s)

    def summary_row(self):
        time_start = 0
        total_uids = 0
        nb_isp = 0
        nb_pdns = 0
        nb_others = 0
        nb_A = 0
        nb_A_isp = 0
        nb_A_pdns = 0
        nb_A_others = 0
        nb_AAAA = 0
        nb_AAAA_isp = 0
        nb_AAAA_pdns = 0
        nb_AAAA_others = 0
        nb_HTTPS = 0
        nb_HTTPS_isp = 0
        nb_HTTPS_pdns = 0
        nb_HTTPS_others = 0

        for start in self.slices:
            if start < time_start or time_start == 0:
                time_start = start
            r_row = self.slices[start]
            total_uids += r_row.total_uids
            nb_isp += r_row.nb_isp
            nb_pdns += r_row.nb_pdns
            nb_others += r_row.nb_others
            nb_A += r_row.nb_A
            nb_A_isp += r_row.nb_A_isp
            nb_A_pdns += r_row.nb_A_pdns
            nb_A_others += r_row.nb_A_others
            nb_AAAA += r_row.nb_AAAA
            nb_AAAA_isp += r_row.nb_AAAA_isp
            nb_AAAA_pdns += r_row.nb_AAAA_pdns
            nb_AAAA_others += r_row.nb_AAAA_others
            nb_HTTPS += r_row.nb_HTTPS
            nb_HTTPS_isp += r_row.nb_HTTPS_isp
            nb_HTTPS_pdns += r_row.nb_HTTPS_pdns
            nb_HTTPS_others += r_row.nb_HTTPS_others

        row = [ 
            self.query_cc,
            self.query_AS,
            time_start,
            total_uids,
            nb_isp,
            nb_pdns,
            nb_others,
            nb_A,
            nb_A_isp,
            nb_A_pdns,
            nb_A_others,
            nb_AAAA,
            nb_AAAA_isp,
            nb_AAAA_pdns,
            nb_AAAA_others,
            nb_HTTPS,
            nb_HTTPS_isp,
            nb_HTTPS_pdns,
            nb_HTTPS_others
        ]

        return row


class flux_lines:
    def __init__(self):
        self.cc_as_list = dict()

    def add_row(self, row):
        cc = str(row['CC'])
        asn = str(row['AS'])
        key = cc + '-' + asn
        if not key in self.cc_as_list:
            self.cc_as_list[key] = flux_cc_as2(cc, asn)
        self.cc_as_list[key].add_row(row)

    def load_flux(self, file_name):
        df = pd.read_csv(file_name, sep=",", skipinitialspace=True)
        print(file_name + ": " + str(df.shape[0]) + " lines.")
        df.apply(lambda row: self.add_row(row),axis=1)
        print("After loading " + file_name + ", " + str(len(self.cc_as_list)) + " CC/AS.")

    def summary_df(self):
        t = []
        for key in self.cc_as_list:
            t.append(self.cc_as_list[key].summary_row())
        t.sort(key=lambda x:x[3], reverse=True)
        df = pd.DataFrame(t, columns=flux_cc_as2.get_columns())
        return df
