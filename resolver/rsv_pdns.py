# Create a csv file with one row per ASN or group of ASN with the two hours
# buckets starting at 00:30 UTC with buckets:
# Country, AS
# NbUIDs, and then columns for the ISP traffic and each tracked PDNS traffic,
# plus one for "others" so the totals match.
# In the first pass, we will only do that for A records (discuss)
# Each group of columns will show:
# - the total number of unique UIDs (time slice 0)
# - the spread by time slice (0, 0.01, 0.03, 0.3, 1, 3, 10, 30)
# - the number of IPv4 addresses per UID, split by:
#   - sum: sum of number of IP per specific UID for all UIDs
#   - average: average number
#   - max number.
# 
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
    'others':8,
    'Same_AS':0,
    'Same_group':0,
    'Same_CC':8,
    'Cloud':8,
    'Other_cc':8
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

# OBJECTS USED FOR CAPTURE.

# PROV_CC_AS_RR_PROV_SLICE
#   One record per UID per Prov per RR per CC-AS in a time slice.
#   contains a set of pdns_uid_rr present in the AS for that time slice.
# 
class prov_cc_as_rr_prov_uid:
    def __init__(self, query_time, resolver_IP):
        self.first_time = query_time
        self.addr = set()
        self.addr.add(resolver_IP)

    def add(self, query_time, resolver_IP):
        if not resolver_IP in self.addr:
            self.addr.add(resolver_IP)

    def address_count(self):
        n4 = 0
        n6 = 0
        for addr in self.addr:
            if ":" in addr:
                n6 += 1
            else:
                n4 += 1
        return n4, n6

# PROV_CC_AS_RR_PROV_SLICE
#   One record per Prov per RR per CC-AS in a time slice.
#   contains a set of pdns_uid_rr present in the AS for that time slice.
# 
class prov_cc_as_rr_prov_slice:
    def __init__(self):
        self.uids = dict()
        self.time_slices = [ 0, 0, 0, 0, 0, 0, 0, 0, 0 ]
        self.sum_v4 = 0
        self.max_v4 = 0
        self.average_v4 = 0
        self.sum_v6 = 0
        self.max_v6 = 0
        self.average_v6 = 0
        self.max_v4_v6 = 0


    def add_query(self, uid, query_time, resolver_IP):
        if not uid in self.uids:
            self.time_slices[0] += 1
            self.uids[uid] = prov_cc_as_rr_prov_uid(query_time, resolver_IP)
        else:
            self.uids[uid].add(query_time, resolver_IP)
            delta_time = query_time - self.uids[uid].first_time
            for i in range(1, len(delta_range)):
                if delta_time <= delta_range[i]:
                    self.time_slices[i] += 1
                    break

    def add_slice(self, other):
        for i_x in range(0, len(delta_range)):
            self.time_slices[i_x] += other.time_slices[i_x]
        if len(other.uids) == 0:
            self.sum_v4 += other.sum_v4
            if other.max_v4 > self.max_v4:
                self.max_v4 = other.max_v4
            self.sum_v6 += other.sum_v6
            if other.max_v6 > self.max_v6:
                self.max_v6 = other.max_v6
            if other.max_v4_v6 > self.max_v4_v6:
                self.max_v4_v6 = other.max_v4_v6
        else:
            for uid in other.uids:
                n4, n6 = other.uids[uid].address_count()
                self.sum_v4 += n4
                if n4 > self.max_v4:
                    self.max_v4 = n4
                self.sum_v6 += n6
                if n6 > self.max_v6:
                    self.max_v6 = n6
                n_tot = n4 + n6
                if n_tot > self.max_v4_v6:
                    self.max_v4_v6 = n_tot
    
    def get_headers(h):
        for rgn in range_names:
            h.append(rgn)
        for ads in addr_stats:
            h.append(ads)
        return h

    def get_row(self, row_prefix):
        r = []
        for x in row_prefix:
            r.append(x)
        for sl in self.time_slices:
            r.append(sl)
        if self.time_slices[0] == 0:
            av4 = 0
            av6 = 0
        else:
            av4 = self.sum_v4/self.time_slices[0]
            av6 = self.sum_v6/self.time_slices[0]
        r.append(self.sum_v4)
        r.append(self.max_v4)
        r.append(av4)
        r.append(self.sum_v6)
        r.append(self.max_v6)
        r.append(av6)
        r.append(self.max_v4_v6)
        
        return r
    
    def load_row(self, x):
        self.time_slices[0] = x['nb_0ms']
        self.time_slices[1] = x['nb_u10ms']
        self.time_slices[2] = x['nb_u30ms']
        self.time_slices[3] = x['nb_u100ms']
        self.time_slices[4] = x['nb_u300ms']
        self.time_slices[5] = x['nb_u1s']
        self.time_slices[6] = x['nb_u3s']
        self.time_slices[7] = x['nb_u10s']
        self.time_slices[8] = x['nb_u30s']
        self.sum_v4 = x['sum_v4']
        self.max_v4 = x['max_v4']
        self.average_v4 = x['average_v4']
        self.sum_v6 = x['sum_v6']
        self.max_v6 = x['max_v6']
        self.average_v6 = x['average_v6']
        self.max_v4_v6 = x['max_v4v6']



# PROV_CC_AS_RR_SLICE
#   One record per RR per CC-AS in a time slice.
#   contains a set of pdns_uid_rr present in the AS for that time slice.
# 
class prov_cc_as_rr_slice:
    def __init__(self):
        self.prov = [ None, None, None, None, None, None, None, None, None ]
        self.uids = set()
        self.nb_uids = 0

    def add_query(self, uid, query_time, prov, resolver_IP):
        if not uid in self.uids:
            self.uids.add(uid)
        if prov in Prov_index:
            p_index = Prov_index[prov]
        else:
            print("Folding " + prov + " into others.")
            p_index = Prov_index_others
        if self.prov[p_index] == None:
            self.prov[p_index] = prov_cc_as_rr_prov_slice()
        self.prov[p_index].add_query(uid, query_time, resolver_IP)

    def add_slice(self, other):
        self.nb_uids += other.nb_uids + len(other.uids)
        for p_x in range(0, len(Prov_names)):
            if other.prov[p_x] != None:
                if self.prov[p_x] == None:
                    self.prov[p_x] = prov_cc_as_rr_prov_slice() 
                self.prov[p_x].add_slice(other.prov[p_x])
        
    def get_headers(h):
        h.append("uids_rr")
        h.append("prov")
        return prov_cc_as_rr_prov_slice.get_headers(h)

    def get_rows(self, row_prefix):
        t = []
        for p_x in range(0, len(Prov_names)):
            if self.prov[p_x] != None:
                rp = []
                for x in row_prefix:
                    rp.append(x)
                rp.append(self.nb_uids)
                rp.append(Prov_names[p_x])
                t.append(self.prov[p_x].get_row(rp))
        return t

    def load_row(self, x):
        self.nb_uids = x['uids_rr']
        prov = x['prov']
        p_x = Prov_index[prov]
        if self.prov[p_x] != None:
            print("Duplicate provider " + prov + " for " + x['CC'] +
               '-' + x['AS'] + '-' + x['rr_type'])
            exit(-1)
        else:
            self.prov[p_x] = prov_cc_as_rr_prov_slice()
            self.prov[p_x].load_row(x)

# PROV_CC_AS_SLICE
#   One record per CC-AS in a time slice.
#   contains a set of pdns_uid_rr present in the AS for that time slice.
#   also contains 
# 
class prov_cc_as_slice:
    def __init__(self, query_cc, query_AS):
        self.query_cc = query_cc
        self.query_AS = query_AS
        self.rr = [None, None, None]
        self.uids = set()
        self.nb_uids = 0

    def add_query(self, uid, query_time, query_rr, prov, resolver_IP):
        if not uid in self.uids:
            self.uids.add(uid)
        nb_uids = 0
        r_x = rr_index[query_rr]
        if self.rr[r_x] == None:
            self.rr[r_x] = prov_cc_as_rr_slice()
        self.rr[r_x].add_query(uid, query_time, prov, resolver_IP)

    def add_slice(self, other):
        self.nb_uids += other.nb_uids + len(other.uids)
        for r_x in range(0, len(rr_names)):
            if other.rr[r_x] != None:
                if self.rr[r_x] == None:
                    self.rr[r_x] = prov_cc_as_rr_slice() 
                self.rr[r_x].add_slice(other.rr[r_x])

    def get_headers(h):
        h.append("uids")
        h.append("rr_type")
        return prov_cc_as_rr_slice.get_headers(h)

    def get_rows(self, row_prefix):
        t = []
        for r_x in range(0, len(rr_names)):
            if self.rr[r_x] != None:
                rp = []
                for x in row_prefix:
                    rp.append(x)
                rp.append(self.nb_uids)
                rp.append(rr_names[r_x])
                rows = self.rr[r_x].get_rows(rp)
                for row in rows:
                    t.append(row)
        return t

    def load_row(self, x):
        query_rr = x['rr_type']
        r_x = rr_index[query_rr]
        if self.rr[r_x] == None:
            self.rr[r_x] = prov_cc_as_rr_slice()
        self.rr[r_x].load_row(x)

# PROV_SLICE
#   Contains the dict of CC_AS present in the time slice
#
class prov_slice:
    def __init__(self, query_time):
        self.start_time = query_time
        self.cc_as = dict()
        self.uids = dict()
        self.query_time = 0

    def add_query(self, uid, query_cc, query_AS, query_time, query_rr, prov, resolver_IP):
        key = str(query_cc) + "-" + str(query_AS)
        if not key in self.cc_as:
            self.cc_as[key] = prov_cc_as_slice(query_cc, query_AS)
        self.cc_as[key].add_query(uid, query_time, query_rr, prov, resolver_IP)

    def add_slice(self, other):
        if self.query_time == 0:
            self.query_time = other.query_time
        for key in other.cc_as:
            if not key in self.cc_as:
                self.cc_as[key] = prov_cc_as_slice(
                    other.cc_as[key].query_cc, other.cc_as[key].query_AS)
            self.cc_as[key].add_slice(other.cc_as[key])

    def get_headers():
        h = [ "CC", "AS" ]
        return prov_cc_as_slice.get_headers(h)

    def get_df(self):
        t = []

        for key in self.cc_as:
            row_prefix = [ self.cc_as[key].query_cc, self.cc_as[key].query_AS]
            t_as = self.cc_as[key].get_rows(row_prefix)
            for row in t_as:
                t.append(row)
        df = pd.DataFrame(t, columns = prov_slice.get_headers())

        return df

    def load_row(self, x):
        query_cc = x['CC']
        query_AS = x['AS']
        key = str(query_cc) + "-" + str(query_AS)
        if not key in self.cc_as:
            self.cc_as[key] = prov_cc_as_slice(query_cc, query_AS)
            self.cc_as[key].nb_uids = x['uids']
        self.cc_as[key].load_row(x)

    def load_file(self, csv_file):
        df = pd.read_csv(csv_file)
        for index, row in df.iterrows():
            self.load_row(row)

class prov_parse:
    def __init__(self, ip2a4, ip2a6, as_names):
        self.ip2a4 = ip2a4 
        self.ip2a6 = ip2a6
        self.as_names = as_names
        self.previous = prov_slice(0)
        self.current = prov_slice(0)
        self.summary = prov_slice(0)

    def summarize(self, query_time):
        self.summary.add_slice(self.previous)
        self.previous = self.current
        self.current = prov_slice(query_time)

    def add_query(self, uid, query_cc, query_AS, query_time, query_rr, prov, resolver_IP):
        if self.current.query_time == 0:
            self.current.query_time = query_time

        if uid in self.previous.uids:
            self.previous.add_query(uid, query_cc, query_AS, query_time, query_rr, prov, resolver_IP)
        else:
            self.current.add_query(uid, query_cc, query_AS, query_time, query_rr, prov, resolver_IP)
        
        if query_time > (self.current.query_time + 60):
            self.summarize(query_time)

    def load_prov_log(self, log_file, log_threshold=15625, time_start=0):
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
                if x.filter(rr_types=['A', 'AAAA', 'HTTPS'], experiment=['0du'], query_delay=30000, check_dotnxdomain=True):
                    if x.resolver_AS == "" or x.resolver_cc == "":
                        x.set_resolver_AS(self.ip2a4, self.ip2a6, self.as_names)
                    if x.resolver_AS != 'AS0' and (x.query_time - x.query_ad_time) < 30:
                        self.add_query(x.query_user_id, x.query_cc, x.query_AS, x.query_time, x.rr_type, x.resolver_tag, x.resolver_IP)

                    nb_events += 1
                    if (nb_events%lth) == 0:
                        new_time = time.time() - time_start
                        print(log_file + ": loaded " + str(nb_events) + " events at " + str(new_time))
                        sys.stdout.flush()
                        if lth < 1000000:
                            lth *= 2
        return nb_events

    def get_df(self):
        self.summarize(0)
        self.summarize(0)
        return self.summary.get_df()

    def save_and_close(self, output_file):
        df = self.get_df()
        df.to_csv(output_file)

