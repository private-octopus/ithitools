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
import traceback
import top_as
import time
import bz2
from rsv_delay_class import delay_query_as
import rsv_arguments
import open_rsv
import time


delta_first_max = 0

PDNS_names = [
    'googlePDNS',
    'cloudflare',
    'opendns',
    'quad9',
    'level3',
    'neustar',
    'he',
    'onedns',
    'dnspai',
    '114dns',
    'dnspod',
    'cnnic',
    'twnic',
    'quad101',
    'greenteamdns',
    'freenom',
    'uncensoreddns',
    'vrsgn'
]

Prov_names = [
    'ISP4',
    'ISP6',
    'googlepdns',
    'cloudflare',
    'opendns',
    'quad9',
    'level3',
    'neustar',
    'he',
    'onedns',
    'dnspai',
    '114dns',
    'dnspod',
    'cnnic',
    'twnic',
    'quad101',
    'greenteamdns',
    'freenom',
    'uncensoreddns',
    'vrsgn',
    'same_CC',
    'others' ]

Prov_index = {
    'ISP':0,
    'ISP4':0,
    'ISP6':1,
    'googlepdns':2,
    'cloudflare':3,
    'opendns':4,
    'quad9':5,
    'level3':6,
    'neustar':7,
    'he':8,
    'onedns':9,
    'dnspai':10,
    '114dns':11,
    'dnspod':12,
    'cnnic':13,
    'twnic':14,
    'quad101':15,
    'greenteamdns':16,
    'freenom':17,
    'uncensoreddns':18,
    'vrsgn':19,
    'others':21,
    'Same_AS':0,
    'Same_group':0,
    'Same_CC':20,
    'same_CC':20,
    'same_cc':20,
    'Cloud':21,
    'Other_cc':21
}

Prov_index_others = 21
Prov_index_same_cc = 20

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
    "average_v4",
    "max_v4",
    "sum_v6",
    "average_v6",
    "max_v6",
    "average_v4v6",
    "max_v4v6"]

# OBJECTS USED FOR CAPTURE.

# PROV_CC_AS_RR_PROV_SLICE
#   One record per UID per Prov per RR per CC-AS in a time slice.
#   contains a set of pdns_uid_rr present in the AS for that time slice.
#
class prov_cc_as_rr_prov_uid:
    def __init__(self, query_time, resolver_IP, resolver_key=""):
        self.first_time = query_time
        self.addr = set()
        self.addr.add(resolver_IP)
        self.query_times = [ query_time ]
        self.resolver_keys = [ resolver_key ]
        self.n_first = 0

    def add(self, query_time, resolver_IP, resolver_key=""):
        if query_time < self.first_time:
            first_time = query_time;
            self.n_first = len(self.query_times)
        self.query_times.append(query_time)
        self.resolver_keys.append(resolver_key)
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

    def add_delays(self, prov_slice, rr_query_time):
        for q in range(0, len(self.query_times)):
            query_time = self.query_times[q]
            rkey = self.resolver_keys[q]
            # first, compute the relative delta_time
            delta_time = query_time - self.first_time
            for i in range(0, len(delta_range)):
                if delta_time <= delta_range[i] and \
                    (i > 0 or q == self.n_first):
                    prov_slice.time_slices[i] += 1
                    if rkey:
                        prov_slice.time_slice_origins[i][rkey] = \
                            prov_slice.time_slice_origins[i].get(rkey, 0) + 1
                    break
            # add computation of absolute slices.
            abs_time = query_time - rr_query_time
            for i in range(0, len(delta_range)):
                if abs_time <= delta_range[i]:
                    if q == self.n_first:
                        prov_slice.abs_slices[i] += 1
                        if rkey:
                            prov_slice.abs_slice_origins[i][rkey] = \
                                prov_slice.abs_slice_origins[i].get(rkey, 0) + 1
                    else:
                        prov_slice.rep_slices[i] += 1
                        if rkey:
                            prov_slice.rep_slice_origins[i][rkey] = \
                                prov_slice.rep_slice_origins[i].get(rkey, 0) + 1
                    break

# PROV_CC_AS_RR_PROV_SLICE
#   One record per Prov per RR per CC-AS in a time slice.
#   contains a set of pdns_uid_rr present in the AS for that time slice.
#
class prov_cc_as_rr_prov_slice:
    def __init__(self):
        self.uids = dict()
        self.time_slices = [ 0, 0, 0, 0, 0, 0, 0, 0, 0 ]
        self.zombies = 0
        self.zombie_origins = {}
        self.sum_v4 = 0
        self.max_v4 = 0
        self.average_v4 = 0
        self.sum_v6 = 0
        self.max_v6 = 0
        self.average_v6 = 0
        self.max_v4v6 = 0
        self.average_v4v6 = 0
        self.abs_slices = [ 0, 0, 0, 0, 0, 0, 0, 0, 0 ]
        self.rep_slices = [ 0, 0, 0, 0, 0, 0, 0, 0, 0 ]
        self.time_slice_origins = [ {} for _ in range(9) ]
        self.abs_slice_origins  = [ {} for _ in range(9) ]
        self.rep_slice_origins  = [ {} for _ in range(9) ]

    def add_query(self, uid, query_time, resolver_IP, uid_first_time, resolver_key=""):
        if uid_first_time > query_time:
            print("Error, uid_first_time - query_time = " + str(uid_first_time - query_time))
            exit(-1)
        is_first = False
        if not uid in self.uids:
            is_first = True
            self.uids[uid] = prov_cc_as_rr_prov_uid(query_time, resolver_IP, resolver_key)
        else:
            self.uids[uid].add(query_time, resolver_IP, resolver_key)
        return is_first

    def add_slice(self, other):
        for i_x in range(0, len(delta_range)):
            self.time_slices[i_x] += other.time_slices[i_x]
            self.abs_slices[i_x] += other.abs_slices[i_x]
            self.rep_slices[i_x] += other.rep_slices[i_x]

        self.zombies += other.zombies
        for key, count in other.zombie_origins.items():
            self.zombie_origins[key] = self.zombie_origins.get(key, 0) + count
        for i in range(9):
            for key, count in other.time_slice_origins[i].items():
                self.time_slice_origins[i][key] = self.time_slice_origins[i].get(key, 0) + count
            for key, count in other.abs_slice_origins[i].items():
                self.abs_slice_origins[i][key] = self.abs_slice_origins[i].get(key, 0) + count
            for key, count in other.rep_slice_origins[i].items():
                self.rep_slice_origins[i][key] = self.rep_slice_origins[i].get(key, 0) + count
        if len(other.uids) == 0:
            self.sum_v4 += other.sum_v4
            if other.max_v4 > self.max_v4:
                self.max_v4 = other.max_v4
            self.sum_v6 += other.sum_v6
            if other.max_v6 > self.max_v6:
                self.max_v6 = other.max_v6
            if other.max_v4v6 > self.max_v4v6:
                self.max_v4v6 = other.max_v4v6
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
                if n_tot > self.max_v4v6:
                    self.max_v4v6 = n_tot

    null_time_slices = [ 0, 0, 0, 0, 0, 0, 0, 0, 0 ]
    def get_slice_json(t_slice, slice_origins, compact, F):
        jrange_names = [ "0ms", "u10ms", "u30ms",
                "u100ms", "u300ms", "u1s",
                "u3s", "u10s", "u30s" ]
        F.write("{")
        is_first = True
        for i in range(0, len(jrange_names)):
            if t_slice[i] != 0 or not compact:
                if not is_first:
                    F.write(",")
                is_first = False
                origins = slice_origins[i] if slice_origins else {}
                if origins:
                    top = sorted(origins.items(), key=lambda x: -x[1])[:5]
                    F.write("\"" + jrange_names[i] + "\":{\"count\":" + str(t_slice[i]) + ",\"origins\":[")
                    first_o = True
                    for key, cnt in top:
                        if not first_o:
                            F.write(",")
                        first_o = False
                        cc, asn = key.split("-", 1)
                        F.write("{\"cc\":\"" + cc + "\",\"as\":\"" + asn + "\",\"count\":" + str(cnt) + "}")
                    F.write("]}")
                else:
                    F.write("\"" + jrange_names[i] + "\":" + str(t_slice[i]))
        F.write("}")

    def get_json(self, compact, F):
        F.write("{\n\"rel\":")
        prov_cc_as_rr_prov_slice.get_slice_json(self.time_slices, self.time_slice_origins, compact, F)
        F.write(",\n\"abs\":")
        prov_cc_as_rr_prov_slice.get_slice_json(self.abs_slices, self.abs_slice_origins, compact, F)
        F.write(",\n\"rep\":")
        prov_cc_as_rr_prov_slice.get_slice_json(self.rep_slices, self.rep_slice_origins, compact, F)
        if self.zombie_origins:
            top = sorted(self.zombie_origins.items(), key=lambda x: -x[1])[:5]
            F.write(",\n\"zombies\":{\"count\":" + str(self.zombies) + ",\"origins\":[")
            first_o = True
            for key, count in top:
                if not first_o:
                    F.write(",")
                first_o = False
                cc, asn = key.split("-", 1)
                F.write("{\"cc\":\"" + cc + "\",\"as\":\"" + asn + "\",\"count\":" + str(count) + "}")
            F.write("]}")
        else:
            F.write(",\n\"zombies\":" + str(self.zombies))
        F.write(",\n\"stats\":{")
        if self.time_slices[0] == 0:
            av4 = 0
            av6 = 0
        else:
            av4 = self.sum_v4/self.time_slices[0]
            av6 = self.sum_v6/self.time_slices[0]
        F.write("\"sum_v4\":" + str(self.sum_v4))
        F.write(",\"max_v4\":" + str(self.max_v4))
        F.write(",\"average_v4\":" + str(av4))
        F.write(",\"sum_v6\":" + str(self.sum_v6))
        F.write(",\"max_v6\":" + str(self.max_v6))
        F.write(",\"average_v6\":" + str(av6))
        F.write(",\"max_v4v6\":" + str(self.max_v4v6))
        F.write(",\"average_v4v6\":" + str(av4 + av6) + "}}")

    def load_json(self, j):
        jrange_names = ["0ms", "u10ms", "u30ms", "u100ms", "u300ms", "u1s", "u3s", "u10s", "u30s"]
        for group, t_slice, slice_origins in [
            ("rel", self.time_slices, self.time_slice_origins),
            ("abs", self.abs_slices,  self.abs_slice_origins),
            ("rep", self.rep_slices,  self.rep_slice_origins),
        ]:
            group_j = j.get(group, {})
            for i, name in enumerate(jrange_names):
                val = group_j.get(name, 0)
                if isinstance(val, dict):
                    t_slice[i] = val.get("count", 0)
                    for origin in val.get("origins", []):
                        cc  = origin.get("cc", "")
                        asn = origin.get("as", "")
                        if cc and asn:
                            key = cc + "-" + asn
                            slice_origins[i][key] = slice_origins[i].get(key, 0) + origin.get("count", 0)
                else:
                    t_slice[i] = val
        zombie_j = j.get("zombies", 0)
        if isinstance(zombie_j, dict):
            self.zombies = zombie_j.get("count", 0)
            for origin in zombie_j.get("origins", []):
                cc  = origin.get("cc", "")
                asn = origin.get("as", "")
                if cc and asn:
                    key = cc + "-" + asn
                    self.zombie_origins[key] = self.zombie_origins.get(key, 0) + origin.get("count", 0)
        else:
            self.zombies = zombie_j
        stats             = j.get("stats", {})
        self.sum_v4       = stats.get("sum_v4", 0)
        self.max_v4       = stats.get("max_v4", 0)
        self.average_v4   = stats.get("average_v4", 0)
        self.sum_v6       = stats.get("sum_v6", 0)
        self.max_v6       = stats.get("max_v6", 0)
        self.average_v6   = stats.get("average_v6", 0)
        self.max_v4v6     = stats.get("max_v4v6", 0)
        self.average_v4v6 = stats.get("average_v4v6", 0)

# PROV_CC_AS_RR_SLICE
#   One record per RR per CC-AS in a time slice.
#   contains a set of pdns_uid_rr present in the AS for that time slice.
#
class prov_cc_as_rr_slice:
    def __init__(self):
        self.prov = []
        for p in Prov_names:
            self.prov.append(None)
        self.uids = dict()
        self.nb_uids = 0
        self.uids_isp = set()
        self.nb_uids_isp = 0
        self.delta_first_max = 0
        self.sum_prov = 0
        self.average_prov = 0

    def add_query(self, uid, query_time, query_ad_time, prov, resolver_IP, resolver_key=""):
        if prov in Prov_index:
            p_index = Prov_index[prov]
            if p_index == 0:
                if ":" in resolver_IP:
                    p_index = 1;
                if not uid in self.uids_isp and \
                    query_time <= query_ad_time + 30:
                    self.uids_isp.add(uid)
        else:
            #print("Folding " + prov + " into others.")
            p_index = Prov_index_others
        if self.prov[p_index] == None:
            self.prov[p_index] = prov_cc_as_rr_prov_slice()
        if query_time > query_ad_time + 30:
            self.prov[p_index].zombies += 1
            if resolver_key:
                self.prov[p_index].zombie_origins[resolver_key] = self.prov[p_index].zombie_origins.get(resolver_key, 0) + 1
            #print("Zombies + 1 for prov " + str(p_index))
        else:
            if not uid in self.uids:
                self.uids[uid] = query_time
            elif self.uids[uid] > query_time:
                self.uids[uid] = query_time
            key_for_prov = resolver_key if p_index in (Prov_index_same_cc, Prov_index_others) else ""
            if self.prov[p_index].add_query(uid, query_time, resolver_IP, self.uids[uid], key_for_prov):
                self.sum_prov += 1

    def summarize_delays(self):
        for uid in self.uids:
            query_time = self.uids[uid]
            for p_x in range(0, len(Prov_names)):
                if self.prov[p_x] != None:
                    if uid in self.prov[p_x].uids:
                        self.prov[p_x].uids[uid].add_delays(self.prov[p_x], query_time)

    def add_slice(self, other):
        other.summarize_delays()
        self.nb_uids += other.nb_uids + len(other.uids)
        self.nb_uids_isp += other.nb_uids_isp + len(other.uids_isp)
        self.sum_prov += other.sum_prov
        if self.nb_uids > 0:
            self.average_prov = self.sum_prov / self.nb_uids
        for p_x in range(0, len(Prov_names)):
            if other.prov[p_x] != None:
                if self.prov[p_x] == None:
                    self.prov[p_x] = prov_cc_as_rr_prov_slice()
                self.prov[p_x].add_slice(other.prov[p_x])

    def get_average_v4v6(self):
        average_v4v6 = 0
        if self.nb_uids_isp > 0:
            sum_v4v6 = 0
            if self.prov[0] != None:
                # prov[0] corresponds to ISP4
                sum_v4v6 += self.prov[0].sum_v4
            if self.prov[1] != None:
                # prov[1] corresponds to ISP6
                sum_v4v6 += self.prov[1].sum_v6
            average_v4v6 = sum_v4v6/self.nb_uids_isp
        return average_v4v6

    def get_json(self, compact, F):
        F.write("{" + \
            "\"uids\":" + str(self.nb_uids) + "," + \
            "\"sum_prov\":" + str(self.sum_prov) + "," + \
            "\"average_prov\":" + str(self.average_prov) + "," + \
            "\"uids_isp\":" + str(self.nb_uids_isp) + "," + \
            "\"average_v4v6\":" + str(self.get_average_v4v6()))
        F.write(",\n\"providers\":{")
        is_first_prov = True
        for p_x in range(0, len(Prov_names)):
            if self.prov[p_x] != None:
                if not is_first_prov:
                    F.write(",")
                is_first_prov = False
                F.write("\n\"" + Prov_names[p_x] + "\":")
                self.prov[p_x].get_json(compact, F)
        F.write("}}")

    def load_json(self, j):
        self.nb_uids      = j.get("uids", 0)
        self.sum_prov     = j.get("sum_prov", 0)
        self.average_prov = j.get("average_prov", 0)
        self.nb_uids_isp  = j.get("uids_isp", 0)
        for prov_name, prov_j in j.get("providers", {}).items():
            if prov_name in Prov_index:
                p_x = Prov_index[prov_name]
                self.prov[p_x] = prov_cc_as_rr_prov_slice()
                self.prov[p_x].load_json(prov_j)

# PROV_CC_AS_SLICE
#   One record per CC-AS in a time slice.
#   contains a set of pdns_uid_rr present in the AS for that time slice.
#   also contains
#

was_folded = set()

class prov_cc_as_slice:
    def __init__(self, query_cc, query_AS):
        self.query_cc = query_cc
        self.query_AS = query_AS
        self.rr = [None, None, None]
        self.uids = set()
        self.nb_uids = 0

    def add_query(self, uid, query_time, query_ad_time, query_rr, prov, resolver_IP, resolver_key=""):
        if query_time <= query_ad_time + 30 and not uid in self.uids:
            self.uids.add(uid)
        r_x = rr_index[query_rr]
        if self.rr[r_x] == None:
            self.rr[r_x] = prov_cc_as_rr_slice()
        if not prov in Prov_index:
            old_prov = prov
            if resolver_key.startswith(self.query_cc):
                prov = "same_CC"
            else:
                prov = "others"
            if not old_prov in was_folded:
                was_folded.add(old_prov)
                print("Folding " + old_prov + " into " + prov + " (cc: " + self.query_cc + ")")

        self.rr[r_x].add_query(uid, query_time, query_ad_time, prov, resolver_IP, resolver_key)

    def add_slice(self, other):
        self.nb_uids += other.nb_uids + len(other.uids)
        for r_x in range(0, len(rr_names)):
            if other.rr[r_x] != None:
                if self.rr[r_x] == None:
                    self.rr[r_x] = prov_cc_as_rr_slice()
                self.rr[r_x].add_slice(other.rr[r_x])

    def get_json(self, compact, F):
        F.write("\n{\"cc\":\"" + self.query_cc + "\"," + \
            "\"as\":\"" + self.query_AS + "\"," +  \
            "\"uids\":" + str(self.nb_uids) + "," + \
            "\"rr\":{")
        first_rr = True
        for r_x in range(0, len(rr_names)):
            if self.rr[r_x] != None:
                if not first_rr:
                    F.write(",")
                first_rr = False
                F.write("\n\"" + rr_names[r_x] + "\":")
                first_rr = False
                self.rr[r_x].get_json(compact, F)
        F.write("}}")

    def load_json(self, j):
        self.nb_uids = j.get("uids", 0)
        for rr_name, rr_j in j.get("rr", {}).items():
            if rr_name in rr_index:
                r_x = rr_index[rr_name]
                self.rr[r_x] = prov_cc_as_rr_slice()
                self.rr[r_x].load_json(rr_j)

# PROV_SLICE
#   Contains the dict of CC_AS present in the time slice
#
class prov_slice:
    def __init__(self, query_time):
        self.start_time = query_time
        self.cc_as = dict()
        self.uids = dict()
        self.query_time = 0

    def add_query(self, uid, query_cc, query_AS, query_time, query_ad_time, query_rr, prov, resolver_IP, resolver_key=""):
        key = str(query_cc) + "-" + str(query_AS)
        if not key in self.cc_as:
            self.cc_as[key] = prov_cc_as_slice(query_cc, query_AS)
        self.cc_as[key].add_query(uid, query_time, query_ad_time, query_rr, prov, resolver_IP, resolver_key)

    def add_slice(self, other):
        if self.query_time == 0:
            self.query_time = other.query_time
        for key in other.cc_as:
            if not key in self.cc_as:
                self.cc_as[key] = prov_cc_as_slice(
                    other.cc_as[key].query_cc, other.cc_as[key].query_AS)
            self.cc_as[key].add_slice(other.cc_as[key])

    def get_json(self, F, compact=True):
        F.write("{\"asns\": [")
        first_as = True
        for key in self.cc_as:
            if not first_as:
                F.write(",")
            first_as = False
            r = self.cc_as[key].get_json(compact, F)
        F.write("]}\n")

    def load_json_file(self, json_file):
        import json
        if json_file.endswith(".bz2"):
            F = bz2.open(json_file, "rt")
        else:
            F = open(json_file, "r")
        with F:
            data = json.load(F)
        for asn_j in data.get("asns", []):
            query_cc = asn_j.get("cc", "ZZ")
            query_AS = asn_j.get("as", "AS0")
            key = str(query_cc) + "-" + str(query_AS)
            if key not in self.cc_as:
                self.cc_as[key] = prov_cc_as_slice(query_cc, query_AS)
            self.cc_as[key].load_json(asn_j)

class prov_parse:
    def __init__(self, ip2a4, ip2a6, as_names):
        self.ip2a4 = ip2a4
        self.ip2a6 = ip2a6
        self.as_names = as_names
        self.previous = prov_slice(0)
        self.current = prov_slice(0)
        self.summary = prov_slice(0)
        self.zombie_max = 30
        self.uxx_nb = 0
        self.uxx_report = 1

    def summarize(self, query_time):
        self.summary.add_slice(self.previous)
        self.previous = self.current
        self.current = prov_slice(query_time)

    def add_query(self, uid, query_cc, query_AS, query_time, query_ad_time, query_rr, prov, resolver_IP, resolver_key=""):
        if self.current.query_time == 0:
            self.current.query_time = query_time

        query_cc = str(query_cc)
        if len(query_cc) != 2:
            query_cc = 'ZZ'

        #if query_time > query_ad_time + self.zombie_max:
        #    self.zombie_max = query_time - query_ad_time
        #    print("Zombie: " + str(self.zombie_max))

        if uid in self.previous.uids:
            self.previous.add_query(uid, query_cc, query_AS, query_time, query_ad_time, query_rr, prov, resolver_IP, resolver_key)
        else:
            self.current.add_query(uid, query_cc, query_AS, query_time, query_ad_time, query_rr, prov, resolver_IP, resolver_key)

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
                if x.filter(rr_types=['A', 'AAAA', 'HTTPS'], experiment=['0du'], query_delay=1000000000, check_dotnxdomain=True):
                    if x.resolver_AS == "" or x.resolver_cc == "":
                        x.set_resolver_AS(self.ip2a4, self.ip2a6, self.as_names)
                    if x.resolver_AS != 'AS0':
                        seconds_in_day = int(x.query_ad_time) % (24*3600)
                        if seconds_in_day < 30 or \
                            seconds_in_day >= (24*3600) - 30:
                            # TODO: add another output file containing the transactions that are dropped here,
                            # so they can be processed in a second pass.
                            pass
                        else:
                            resolver_key = x.resolver_cc + "-" + x.resolver_AS
                            self.add_query(x.query_user_id, x.query_cc, x.query_AS, x.query_time, x.query_ad_time, x.rr_type, x.resolver_tag, x.resolver_IP, resolver_key)

                    nb_events += 1
                    if (nb_events%lth) == 0:
                        new_time = time.time() - time_start
                        print(log_file + ": loaded " + str(nb_events) + " events at " + str(new_time))
                        sys.stdout.flush()
                        if lth < 1000000:
                            lth *= 2
        return nb_events

    def get_summary_slice(self):
        self.summarize(0)
        self.summarize(0)
        return self.summary

