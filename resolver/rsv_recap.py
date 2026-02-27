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
import pandas as pd
import traceback
import top_as
import time
import bz2
from rsv_delay_class import delay_query_as
import rsv_arguments
import open_rsv

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

cutoff_delay = 0.3

class recap_uid:
    def __init__(self, query_time, rr_type, resolver_tag):
        self.first_time = query_time
        self.has_https = False
        self.has_AAAA = False
        self.has_A = False
        self.has_A_prov = [False, False, False]
        self.nb_A_prov = [0, 0, 0]
        self.nb_A_under = [[ 0, 0, 0 ],[ 0, 0, 0 ],[ 0, 0, 0 ],[ 0, 0, 0 ],[ 0, 0, 0 ]]
        self.first_time_A = 0

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
        self.nb_https = 0
        self.nb_AAAA = 0
        self.nb_A = 0
        # vector of 7 patterns (number 1 to 7 ]
        self.nb_A_pattern = [ 0, 0, 0, 0, 0, 0, 0 ]
        # vector of duplicates (ISP, PDNS, Other)
        self.nb_A_prov = [ 0, 0, 0 ]
        # vector of count (ISP, PDNS, Others) by time slices (0-300ms-1s-3s-10s-30s)
        self.nb_A_under = [[ 0, 0, 0 ],[ 0, 0, 0 ],[ 0, 0, 0 ],[ 0, 0, 0 ],[ 0, 0, 0 ]]
        self.zombie = [ 0, 0, 0, 0]
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
        self.previous_slice.nb_https = self.nb_https
        self.previous_slice.nb_AAAA = self.nb_AAAA
        self.previous_slice.nb_A = self.nb_A
        for i in range(0,7):
            self.previous_slice.nb_A_pattern[i] = self.nb_A_pattern[i]
        for i in range(0,3):
            self.previous_slice.nb_A_prov[i] = self.nb_A_prov[i]
        for i in range(0,5):
            for j in range(0,3):
                self.previous_slice.nb_A_under[i][j] = self.nb_A_under[i][j]
        for i in range(0,4):
            self.previous_slice.zombie[i] = self.zombie[i]
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
        self.nb_https = 0
        self.nb_AAAA = 0
        self.nb_A = 0
        for i in range(0,7):
            self.nb_A_pattern[i] = 0
        for i in range(0,3):
            self.nb_A_prov[i] = 0
        for i in range(0,5):
            for j in range(0,3):
                self.nb_A_under[i][j] = 0
        self.zombie = [ 0, 0, 0, 0]
        self.first_3s = 0
        self.first_10s = 0
        self.sum_first_delay = 0
        self.max_first_delay = 0
        self.uids = dict()
        self.should_save = True
        self.slice_number += 1

    def summarize(self):
        #print("Summarize slice: " + str(self.slice_number))
        for uid in self.uids:
            r_uid = self.uids[uid]
            
            if r_uid.has_https:
                self.nb_https += 1
            if r_uid.has_AAAA:
                self.nb_AAAA += 1
            if r_uid.has_A:
                self.nb_A += 1
                pattern_id = 0
                for i in range(0,3):
                    if r_uid.has_A_prov[i]:
                        pattern_id += (1<<i);
                    self.nb_A_prov[i] += r_uid.nb_A_prov[i]
                #print("Summarize pattern: " + str(pattern_id))
                if pattern_id > 0:
                    self.nb_A_pattern[pattern_id-1] += 1
                for i in range(0, 5):
                    for j in range(0,3):
                        self.nb_A_under[i][j] += r_uid.nb_A_under[i][j]

    def update_uid(self, r_uid, query_time, rr_type, resolver_tag):
        #update_uid is only called if the delat to time stamp is less than 30s.
        delta_range = [ 0.3, 1, 3, 10, 30 ]
        if rr_type == 'HTTPS':
            r_uid.has_https = True
        elif rr_type == 'AAAA':
            r_uid.has_AAAA = True
        elif rr_type == 'A':
            if not r_uid.has_A:
                r_uid.first_time_A = query_time
            delta_t = query_time - r_uid.first_time_A
            r_uid.has_A = True
            if resolver_tag in rsv_log_parse.tag_isp_set:
                prov_index = 0
            elif resolver_tag in rsv_log_parse.tag_public_set:
                prov_index = 1
            else:
                prov_index = 2
            if delta_t <= cutoff_delay:
                r_uid.has_A_prov[prov_index] = True
            for i in range(0, len(delta_range)):
                if delta_t <= delta_range[i]:
                    r_uid.nb_A_under[i][prov_index] += 1
                    r_uid.nb_A_prov[prov_index] += 1
                    break

    def get_header():
        s = "CC,AS,start,uids,first_isp,"
        for pdns_name in pdns_names:
            s += pdns_name  + ','
        s += 'first_others,nb_https,nb_AAAA,nb_A,'
        s += 'A_ISP_only,A_PDNS_only,A_ISP_PDNS,A_others_only,A_ISP_others,A_PDNS_others,A_all3,'
        s += 'nb_A_ISP,nb_A_PDNS,nb_A_others,'
        s += 'nb_A_u300ms_ISP,nb_A_u300ms_PDNS,nb_A_u300ms_others,'
        s += 'nb_A_u1s_ISP,nb_A_u1s_PDNS,nb_A_u1s_others,'
        s += 'nb_A_u3s_ISP,nb_A_u3s_PDNS,nb_A_u3s_others,'
        s += 'nb_A_u10s_ISP,nb_A_u10s_PDNS,nb_A_u10s_others,'
        s += 'nb_A_u30s_ISP,nb_A_u30s_PDNS,nb_A_u30s_others,'
        s += 'zombies,z_ISP,z_PDNS,z_others,first_3s,first_10s,sum_delay,max_delay' + '\n'
        return s

    def save_to_file(self):
        #print("Saving slice: " + str(self.slice_number))
        self.summarize()
        self.saved_slices += 1
        self.saved_uids += len(self.uids)
        s = self.query_cc + "," + self.query_AS + "," + str(self.slice_start) + ","
        s += str(len(self.uids)) + ','
        s += str(self.first_by_isp) + ','
        for pdns_total in self.first_by_pdns:
            s += str(pdns_total) + ','
        s += str(self.first_by_others) + ',' + str(self.nb_https) + ','
        s += str(self.nb_AAAA) + ','
        s += str(self.nb_A) + ','
        for pattern_total in self.nb_A_pattern:
            s += str(pattern_total) + ','
        for dups_total in self.nb_A_prov:
            s += str(dups_total) + ','
        for i in range(0,5):
            for prov_index in range(0,3):
                s += str(self.nb_A_under[i][prov_index]) + ','
        for z in self.zombie:
            s += str(z) + ','
        s += str(self.first_3s) + ',' + str(self.first_10s) + ','
        s += str(self.sum_first_delay) + ',' + str(self.max_first_delay) + '\n'
        self.recap_file.write(s)

    def save_to_file_and_rotate(self):
        #print("Save and rotate slice: " + str(self.slice_number))
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

    def add_query(self, uid, query_time, rr_type, resolver_tag, query_ad_time):
        if query_time >= self.slice_start + self.slice_duration:
            self.save_to_file_and_rotate()
        delta_first = query_time - query_ad_time
        if delta_first > 30:
            self.zombie[0] += 1
            if resolver_tag in rsv_log_parse.tag_isp_set:
                self.zombie[1] += 1
            elif resolver_tag in rsv_log_parse.tag_public_set:
                self.zombie[2] += 1
            else:
                self.zombie[3] += 1
        else:
            if uid in self.previous_slice.uids:
                self.previous_slice.update_uid(self.previous_slice.uids[uid], query_time, rr_type, resolver_tag)
            else:
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
                self.update_uid(self.uids[uid], query_time, rr_type, resolver_tag)

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
        self.max_slice = 0
        self.max_time = 0
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
            #print("Start time: " + str(self.first_slice_start))
        key = self.add_cc_as(query_cc, query_AS)
        self.cc_as_dict[key].add_query(uid, query_time, rr_type, resolver_tag, query_ad_time)
        # if self.cc_as_dict[key].slice_number > self.max_slice:
        #    self.max_slice = self.cc_as_dict[key].slice_number
        #    print("Start slice " + str(self.max_slice) + " for " + str(key))
        if query_time > self.max_time:
            self.max_time = query_time

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
                    if x.resolver_AS == "" or x.resolver_cc == "":
                        x.set_resolver_AS(self.ip2a4, self.ip2a6, self.as_names)
                    if x.resolver_AS != 'AS0':
                        self.add_query(x.query_user_id, x.query_time, x.query_cc, x.query_AS, x.resolver_tag, x.rr_type, x.query_ad_time)
                    nb_events += 1
                    if (nb_events%lth) == 0:
                        new_time = time.time() - time_start
                        print(log_file + ": loaded " + str(nb_events) + " events at " + str(new_time))
                        sys.stdout.flush()
                        if lth < 1000000:
                            lth *= 2
        return nb_events

    def save_and_close(self):
        max_slice = 0
        for key in self.cc_as_dict:
            # if self.cc_as_dict[key].slice_number > max_slice:
            #    max_slice = self.cc_as_dict[key].slice_number
            #    print("Flush slice " + str(max_slice) + " for " + key)
            self.cc_as_dict[key].flush_to_file()
        self.recap_file.close()
        #print ("Start time: " + str(self.first_slice_start))
        #print ("Max time: " + str(self.max_time))


recap_columns = [
    'CC', 'AS', 'start', 'uids', 'first_isp',
    'googlepdns', 'cloudflare', 'opendns', 'quad9', 'level3', 'neustar', 'he',
    'first_others', 'nb_https', 'nb_AAAA', 'nb_A',
    'A_ISP_only', 'A_PDNS_only', 'A_ISP_PDNS', 'A_others_only', 'A_ISP_others', 'A_PDNS_others', 'A_all3',
    'nb_A_ISP', 'nb_A_PDNS', 'nb_A_others',
    'nb_A_u300ms_ISP', 'nb_A_u300ms_PDNS', 'nb_A_u300ms_others',
    'nb_A_u1s_ISP', 'nb_A_u1s_PDNS', 'nb_A_u1s_others',
    'nb_A_u3s_ISP', 'nb_A_u3s_PDNS', 'nb_A_u3s_others',
    'nb_A_u10s_ISP', 'nb_A_u10s_PDNS', 'nb_A_u10s_others',
    'nb_A_u30s_ISP', 'nb_A_u30s_PDNS', 'nb_A_u30s_others',
    'zombies', 'z_ISP', 'z_PDNS', 'z_others',
    'first_3s', 'first_10s', 'sum_delay', 'max_delay'
]

recap_first_columns =  [ 
    'CC', 'AS', 'start', 'uids', 'first_isp' ]

recap_final_columns = [ 'nb_https', 'nb_AAAA', 'nb_A',
    'A_ISP_only', 'A_PDNS_only', 'A_ISP_PDNS', 'A_others_only', 'A_ISP_others', 'A_PDNS_others', 'A_all3',
    'nb_A_ISP', 'nb_A_PDNS', 'nb_A_others',
    'nb_A_u300ms_ISP', 'nb_A_u300ms_PDNS', 'nb_A_u300ms_others',
    'nb_A_u1s_ISP', 'nb_A_u1s_PDNS', 'nb_A_u1s_others',
    'nb_A_u3s_ISP', 'nb_A_u3s_PDNS', 'nb_A_u3s_others',
    'nb_A_u10s_ISP', 'nb_A_u10s_PDNS', 'nb_A_u10s_others',
    'nb_A_u30s_ISP', 'nb_A_u30s_PDNS', 'nb_A_u30s_others',
    'zombies', 'z_ISP', 'z_PDNS', 'z_others',
    'first_3s', 'first_10s', 'sum_delay'
]

recap_pdns = [
    'googlepdns', 'cloudflare', 'opendns', 'quad9', 'level3', 'neustar', 'he' ]

class recap_row:
    under_names = [ 
        ['nb_A_u300ms_ISP', 'nb_A_u300ms_PDNS', 'nb_A_u300ms_others'],
        ['nb_A_u1s_ISP', 'nb_A_u1s_PDNS', 'nb_A_u1s_others'],
        ['nb_A_u3s_ISP', 'nb_A_u3s_PDNS', 'nb_A_u3s_others'],
        ['nb_A_u10s_ISP', 'nb_A_u10s_PDNS', 'nb_A_u10s_others'],
        ['nb_A_u30s_ISP', 'nb_A_u30s_PDNS', 'nb_A_u30s_others']]
    def __init__(self, row):
        self.query_cc = row['CC']
        self.query_AS = row['AS']
        self.start = row['start']
        self.total_uids = row['uids']
        self.first_isp = row['first_isp']
        self.total_pdns = [ 0, 0, 0, 0, 0, 0, 0 ]
        for i in range(0,7):
            self.total_pdns[i] = row[recap_pdns[i]]
        self.first_others = row['first_others']
        self.nb_https = row['nb_https']
        self.nb_AAAA = row['nb_AAAA']
        self.nb_A = row['nb_A']
        self.A_ISP_only = row['A_ISP_only']
        self.A_PDNS_only = row['A_PDNS_only']
        self.A_ISP_PDNS = row['A_ISP_PDNS']
        self.A_others_only = row['A_others_only']
        self.A_ISP_others = row['A_ISP_others']
        self.A_PDNS_others = row['A_PDNS_others']
        self.A_all3 = row['A_all3']
        self.nb_A_ISP = row['nb_A_ISP']
        self.nb_A_PDNS = row['nb_A_PDNS']
        self.nb_A_others = row['nb_A_others']
        self.nb_A_under = [
            [ 0, 0, 0], [ 0, 0, 0], [ 0, 0, 0], [ 0, 0, 0], [ 0, 0, 0]]
        for i in range(0,5):
            for prov_index in range(0,3):
                self.nb_A_under[i][prov_index] = row[recap_row.under_names[i][prov_index]]
        self.zombies = row['zombies']
        self.z_ISP = row['z_ISP']
        self.z_PDNS = row['z_PDNS']
        self.z_others = row['z_others']
        self.first_3s = row['first_3s']
        self.first_10s = row['first_10s']
        self.sum_delay = row['sum_delay']
        self.max_delay = row['max_delay']

    def add_row(self, row):
        self.total_uids += row['uids']
        self.first_isp += row['first_isp']
        for i in range(0,7):
            self.total_pdns[i] += row[recap_pdns[i]]
        self.first_others += row['first_others']
        self.nb_https += row['nb_https']
        self.nb_AAAA += row['nb_AAAA']
        self.nb_A += row['nb_A']
        self.A_ISP_only += row['A_ISP_only']
        self.A_PDNS_only += row['A_PDNS_only']
        self.A_ISP_PDNS += row['A_ISP_PDNS']
        self.A_others_only += row['A_others_only']
        self.A_ISP_others += row['A_ISP_others']
        self.A_PDNS_others += row['A_PDNS_others']
        self.A_all3 += row['A_all3']
        self.nb_A_ISP += row['nb_A_ISP']
        self.nb_A_PDNS += row['nb_A_PDNS']
        self.nb_A_others += row['nb_A_others']
        for i in range(0,5):
            for prov_index in range(0,3):
                self.nb_A_under[1][prov_index] += row[recap_row.under_names[i][prov_index]]
        self.zombies += row['zombies']
        self.z_ISP += row['z_ISP']
        self.z_PDNS += row['z_PDNS']
        self.z_others += row['z_others']
        self.first_3s += row['first_3s']
        self.first_10s += row['first_10s']
        self.sum_delay += row['sum_delay']
        if self.max_delay < row['max_delay']:
            self.max_delay = row['max_delay']

class recap_cc_as2:
    def __init__(self, query_cc, query_AS):
        self.query_cc = query_cc
        self.query_AS = query_AS
        self.slices = dict()
        self.total_uids = 0
        self.total_pdns = [ 0, 0, 0, 0, 0, 0, 0 ]
        self.top_pdns = [ 0, 1, 2 ]
        self.skipped_pdns = [ ]
        self.nb_A_under = [[ 0, 0, 0 ],[ 0, 0, 0 ],[ 0, 0, 0 ],[ 0, 0, 0 ],[ 0, 0, 0 ]]

    def add_row(self, row):
        self.total_uids += row['uids']
        start = row['start']
        if not start in self.slices:
            self.slices[start] = recap_row(row)
        else:
            self.slices[start].add_row(row)


    def evaluate(self):
        self.total_pdns = [ 0, 0, 0, 0, 0, 0, 0 ]
        self.top_pdns = [ 0, 1, 2 ]
        self.skipped_pdns = [ ]
        for start in self.slices:
            for i in range(0, 7):
                self.total_pdns[i] += self.slices[start].total_pdns[i]
        t = []
        for i in range(0, 7):
            t.append([i, self.total_pdns[i]])
        t.sort(key=lambda x:x[1], reverse=True)
        for i in range(0,3):
            x = t[i]
            self.top_pdns[i] = x[0]
        top_set = set(self.top_pdns)
        for i in range(0,7):
            if not i in top_set:
                self.skipped_pdns.append(i)

    def get_columns(self):
        columns = []
        columns += recap_first_columns
        for i in range(0,3):
            columns.append(recap_pdns[self.top_pdns[i]])
        columns.append('first_others')
        columns += recap_final_columns
        columns.append('average_delay')
        return columns

    def save_file(self, file_name):
        self.evaluate()
        print("Saving " + file_name)
        with open(file_name, "wt") as F:
            headers = self.get_columns()
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
                s += str(r_row.first_isp) + ","
                for i in range(0,3):
                    s += str(r_row.total_pdns[self.top_pdns[i]]) + ","
                total_skipped = r_row.first_others
                for skipped in self.skipped_pdns:
                    total_skipped += r_row.total_pdns[skipped]
                s += str(total_skipped) + ","
                s += str(r_row.nb_https) + ","
                s += str(r_row.nb_AAAA) + ","
                s += str(r_row.nb_A) + "," 
                s += str(r_row.A_ISP_only) + ","
                s += str(r_row.A_PDNS_only) + ","
                s += str(r_row.A_ISP_PDNS) + ","
                s += str(r_row.A_others_only) + ","
                s += str(r_row.A_ISP_others) + ","
                s += str(r_row.A_PDNS_others) + ","
                s += str(r_row.A_all3) + ","
                s += str(r_row.nb_A_ISP) + ","
                s += str(r_row.nb_A_PDNS) + ","
                s += str(r_row.nb_A_others) + ","           
                for i in range(0,5):
                    for prov_index in range(0,3):
                        s += str(self.nb_A_under[1][prov_index])
                s += str(r_row.zombies) + ","
                s += str(r_row.z_ISP) + ","
                s += str(r_row.z_PDNS) + ","
                s += str(r_row.z_others) + ","
                s += str(r_row.first_3s) + ","
                s += str(r_row.first_10s) + ","
                s += str(r_row.max_delay) + ","
                uids = r_row.total_uids
                if uids <= 0:
                    s += "0,"
                else:
                    average_delay = r_row.sum_delay/uids
                    s += str(average_delay) + ","
                s += "\n"
                F.write(s)

    def summary_columns():
        columns = [ "CC", "AS", 'start', 'uids', 'first_isp',
            'googlepdns', 'cloudflare', 'opendns', 'quad9', 'level3', 'neustar', 'he',
            'first_others', 'nb_https', 'nb_AAAA', 'nb_A',
            'A_ISP_only', 'A_PDNS_only', 'A_ISP_PDNS', 'A_others_only', 'A_ISP_others', 'A_PDNS_others', 'A_all3',
            'nb_A_ISP', 'nb_A_PDNS', 'nb_A_others',
            'nb_A_u300ms_ISP', 'nb_A_u300ms_PDNS', 'nb_A_u300ms_others',
            'nb_A_u1s_ISP', 'nb_A_u1s_PDNS', 'nb_A_u1s_others',
            'nb_A_u3s_ISP', 'nb_A_u3s_PDNS', 'nb_A_u3s_others',
            'nb_A_u10s_ISP', 'nb_A_u10s_PDNS', 'nb_A_u10s_others',
            'nb_A_u30s_ISP', 'nb_A_u30s_PDNS', 'nb_A_u30s_others',
            'zombies', 'z_ISP', 'z_PDNS', 'z_others',
            'first_3s', 'first_10s', 'average_delay', 'max_delay' ]
        return columns


    def summary_row(self):
        time_start = 0
        total_uids = 0
        first_isp = 0
        total_pdns = [ 0, 0, 0, 0, 0, 0, 0 ]
        first_others = 0
        nb_https = 0
        nb_AAAA = 0
        nb_A = 0 
        A_ISP_only = 0
        A_PDNS_only = 0
        A_ISP_PDNS = 0
        A_others_only = 0
        A_ISP_others = 0
        A_PDNS_others = 0
        A_all3 = 0
        nb_A_ISP = 0
        nb_A_PDNS = 0
        nb_A_others = 0
        nb_A_under = [
            [0, 0, 0], [0, 0, 0], [0, 0, 0], [0, 0, 0], [0, 0, 0]]
        zombies = 0
        z_ISP = 0
        z_PDNS = 0
        z_others = 0
        first_3s = 0
        first_10s = 0
        max_delay = 0
        sum_delay = 0
        
        for start in self.slices:
            r_row = self.slices[start]
            if start < time_start or time_start == 0:
                time_start = start
            total_uids += r_row.total_uids
            first_isp += r_row.first_isp
            for i in range(0,7):
                total_pdns[i] += r_row.total_pdns[i]
            first_others += r_row.first_others
            nb_https += r_row.nb_https
            nb_AAAA += r_row.nb_AAAA
            nb_A += r_row.nb_A 
            A_ISP_only += r_row.A_ISP_only
            A_PDNS_only += r_row.A_PDNS_only
            A_ISP_PDNS += r_row.A_ISP_PDNS
            A_others_only += r_row.A_others_only
            A_ISP_others += r_row.A_ISP_others
            A_PDNS_others += r_row.A_PDNS_others
            A_all3 += r_row.A_all3
            nb_A_ISP += r_row.nb_A_ISP
            nb_A_PDNS += r_row.nb_A_PDNS
            nb_A_others += r_row.nb_A_others
            zombies += r_row.zombies
            z_ISP += r_row.z_ISP
            z_PDNS += r_row.z_PDNS           
            for i in range(0,5):
                for j in range(0,3):
                    nb_A_under[i][j] += r_row.nb_A_under[i][j]
            z_others += r_row.z_others
            first_3s += r_row.first_3s
            first_10s += r_row.first_10s
            if max_delay < r_row.max_delay:
                max_delay = r_row.max_delay
            sum_delay += r_row.sum_delay

        average_delay = 0
        if total_uids > 0:
            average_delay = sum_delay/total_uids

        row = [
            self.query_cc,
            self.query_AS,
            time_start,
            total_uids,
            first_isp,
            total_pdns[0],
            total_pdns[1],
            total_pdns[2],
            total_pdns[3],
            total_pdns[4],
            total_pdns[5],
            total_pdns[6],
            first_others,
            nb_https,
            nb_AAAA,
            nb_A, 
            A_ISP_only,
            A_PDNS_only,
            A_ISP_PDNS,
            A_others_only,
            A_ISP_others,
            A_PDNS_others,
            A_all3,
            nb_A_ISP,
            nb_A_PDNS,
            nb_A_others,
            nb_A_under[0][0],
            nb_A_under[0][1],
            nb_A_under[0][2],
            nb_A_under[1][0],
            nb_A_under[1][1],
            nb_A_under[1][2],
            nb_A_under[2][0],
            nb_A_under[2][1],
            nb_A_under[2][2],
            nb_A_under[3][0],
            nb_A_under[3][1],
            nb_A_under[3][2],
            nb_A_under[4][0],
            nb_A_under[4][1],
            nb_A_under[4][2],
            zombies,
            z_ISP,
            z_PDNS,
            z_others,
            first_3s,
            first_10s,
            average_delay,
            max_delay ]

        return row

class recap_lines:
    def __init__(self):
        self.cc_as_list = dict()

    def add_row(self, row):
        cc = str(row['CC'])
        asn = str(row['AS'])
        key = cc + '-' + asn
        if not key in self.cc_as_list:
            self.cc_as_list[key] = recap_cc_as2(cc, asn)
        self.cc_as_list[key].add_row(row)

    def load_recap(self, file_name):
        df = pd.read_csv(file_name, sep=",", skipinitialspace=True)
        print(file_name + ": " + str(df.shape[0]) + " lines.")
        df.apply(lambda row: self.add_row(row),axis=1)
        print("After loading " + file_name + ", " + str(len(self.cc_as_list)) + " CC/AS.")

    def summary_df(self):
        t = []
        for key in self.cc_as_list:
            t.append(self.cc_as_list[key].summary_row())
        t.sort(key=lambda x:x[3], reverse=True)
        df = pd.DataFrame(t, columns=recap_cc_as2.summary_columns())
        return df