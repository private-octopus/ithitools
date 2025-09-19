
# HTTPS Duplicate
#
# We need to separate plausible causes:
# - Multi-homed clients (mobile and Wifi)
# - ISP resolvers duplicating the requests
# Find a metric for each category: Total duplicate (per record type) over Total queries
#   -- with duplicate defined as "several queries to different providers for a given record type and unique ID."
# Compute that for all ISPs. We expect to find a distribution, with most ISP getting a low level
# of duplicates (a few %, due to multi-homed clients or clients deliberately configured with multiple resolvers),
# but some ISP getting a high level (because they duplicate the requests).
# Look at the distribution, maybe find a cut point? 

import sys
import os
from pathlib import Path
import ip2as
import rsv_log_parse
import pandas as pd
import traceback
import top_as
import time
import calendar
import csv
import rsv_arguments

def usage():
    print("Usage: python rsv_dups_metric.py <output_dir> [<list of AS] <csv_file> ... <csv_file>\n")
    print("This script will load the csv files, and produce output files:")
    print("   <output_dir>/duplicate_metrics.csv: table of duplicate usage per 1 hour interval.")
    print("   <output_dir>/duplicate_ASnnnn.csv: table of duplicate usage by 5 minute interval for ASnnnn.")
    print("   <output_dir>/duplicate_as_list.csv: table of duplicate usage per AS.")


def get_time_hour(first_time):
    fth = int(first_time/3600)
    ft = fth*3600
    return ft

rr_name = ['HTTPS', 'A', 'AAAA']

class duplicate_slice:
    def __init__(self):
        self.count = 0
        self.total = [ 0, 0, 0]
        self.dups = [ 0, 0, 0]


class duplicate_query:
    def __init__(self, uid, query_time, query_AS, query_cc):
        self.uid = uid
        self.query_time = query_time
        self.query_AS = query_AS
        self.query_cc = query_cc
        self.records = [ "", "", "" ]

    def add_query(self, rr_type, resolver_AS, resolver_tag):
        if rr_type == 'HTTPS':
            rr_rank = 0
        elif rr_type == 'A':
            rr_rank = 1
        elif rr_type == 'AAAA':
            rr_rank = 2
        else:
            # should never happen!
            return
        if self.records[rr_rank] != resolver_tag:
            if self.records[rr_rank] == "":
                self.records[rr_rank] = resolver_tag
            else:
                self.records[rr_rank] = "*"

def get_slice_time(query_time, first_time, slice_duration, debug=False):
    slice_nb = int((query_time - first_time)/slice_duration)
    if debug:
        print("(" + str(query_time) + " - " + str(first_time) + ") / " + str(slice_duration) + " = " + str(slice_nb))
    slice_time = first_time + slice_nb*slice_duration
    return slice_time

class duplicate_slices:
    def __init__(self, slice_duration, query_AS):
        self.has_AS = len(query_AS) > 0
        self.query_AS = query_AS
        self.slice_duration = slice_duration
        self.slices = dict()
        self.first_time = 0

    def add_event(self, event):
        if self.has_AS and event.query_AS != self.query_AS:
            return False
        slice_time = get_slice_time(event.query_time, self.first_time, self.slice_duration)
        if not slice_time in self.slices:
            self.slices[slice_time] = duplicate_slice()
        for i in range(0, 3):
            if event.records[i] != "":
                self.slices[slice_time].total[i] += 1
                if event.records[i] == "*":
                    self.slices[slice_time].dups[i] += 1
        return True

    def get_df(self):
        v = []
        st = list(self.slices.keys())
        st.sort()
        for slt in st:
            found = False
            for i in range(0, 3):
                if self.slices[slt].total[i] > 0:
                    dups_metric = self.slices[slt].dups[i]/self.slices[slt].total[i]
                    x = [ slt, rr_name[i], dups_metric, self.slices[slt].total[i], self.slices[slt].dups[i] ]
                    v.append(x)

        df = pd.DataFrame(v, columns=["slice_time", "rr_name", "dups_metric", "uids", "dups"])

        return df


class duplicate_as_details:
    def __init__(self, slice_duration, as_list):
        self.as_dict = dict()
        for asn in as_list:
            if not asn in self.as_dict:
                self.as_dict[asn] = duplicate_slices(slice_duration, asn)

    def add_event(self, event):
        ret = False
        if event.query_AS in self.as_dict:
            ret = self.as_dict[event.query_AS].add_event(event)
        return ret

    def save_files(self, output_dir):
        for asn in self.as_dict:
            asn_file = os.path.join(output_dir, "duplicate_" + asn + "_" + str(self.as_dict[asn].slice_duration) + ".csv" )
            asn_df = self.as_dict[asn].get_df()
            asn_df.to_csv(asn_file, sep=",")
            print("Saved: " + str(asn_df.shape[0]) + " time slices in " + asn_file)

class duplicate_AS_list:
    def __init__(self):
        self.key_list = dict()

    def nb_slices(self):
        return len(self.key_list)

    def add_event(self, event):
        key = event.query_cc + "-" + event.query_AS
        if not key in self.key_list:
            self.key_list[key] =  duplicate_slice()   
        for i in range(0, 3):
            if event.records[i] != "":
                self.key_list[key].total[i] += 1
                if event.records[i] == "*":
                    self.key_list[key].dups[i] += 1
        return True

    def get_df(self, threshold = 10000):
        key_metric = []
        max_t = 0

        ks = []
        for key in self.key_list:
            km = 0
            for i in range(0, 3):
                km += self.key_list[key].total[i]
            ks.append([key, km])
            if km > max_t:
                max_t = km

        ks.sort(key=lambda x:x[1], reverse=True)
        if max_t < threshold:
            threshold = int(max_t/2)

        v = []
        for k in ks:
            key = k[0]
            if k[1] >= threshold:
                for i in range(0,3):
                    if self.key_list[key].total[i] > 0:
                        dups_metric = self.key_list[key].dups[i]/self.key_list[key].total[i]
                        x = [ key[0:2], key[3:], rr_name[i], dups_metric, self.key_list[key].total[i], self.key_list[key].dups[i] ]
                        v.append(x)

        df = pd.DataFrame(v, columns=["query_cc", "query_AS", "rr_type", "dups_metric", "uids", "dups"])

        return df

class duplicate_queries:
    def __init__(self):
        self.uid_list = dict()
        self.first_time = 0
        self.tried = 0

    def add_query(self, uid, query_time, query_AS, query_cc, rr_type, resolver_AS, resolver_tag):
        if self.first_time == 0 or query_time < self.first_time:
            first_hour = int(query_time/3600)
            self.first_time = first_hour*3600

        if not uid in self.uid_list:
            self.uid_list[uid] = duplicate_query(uid, query_time, query_AS, query_cc)
        self.uid_list[uid].add_query(rr_type, resolver_AS, resolver_tag)

    # load the input files
    def load_csv_log(self, saved_file):
        nb_events = 0
        with open(saved_file, newline='') as csvfile:
            rsv_reader = csv.reader(csvfile, delimiter=',', quotechar='"')
            is_first = True
            is_second = True
            header_row = [ 'query_time', 'query_AS', 'query_user_id', 'resolver_tag', 'resolver_AS', 'rr_type', 'query_cc' ]
            header_index = [ -1, -1, -1, -1, -1, -1, -1 ]

            for row in rsv_reader:
                if is_first:
                    # print(",".join(row))
                    for i in range(0, len(header_row)):
                        for x in range(0, len(row)):
                            if row[x] == header_row[i]:
                            #   print("row[" + str(x) + "](" + str(row[x]) + ") == " + header_row[i])
                                header_index[i] = x
                                break
                            #else:
                            #    print(row[x] + " != " + header_row[i])
                        if header_index[i] < 0:
                            print("Could not find " + header_row[i] + " in " + ','.join(row))
                            exit(-1)
                    is_first = False
                else:
                    if (is_second):
                        #print(",".join(row))
                        #for i in range(0, len(header_row)):
                        #    print(str(i) + ": x[" + header_row[i] + "] = " + str(row[header_index[i]]))
                        is_second = False
                    query_time = float(row[header_index[0]])
                    query_AS = row[header_index[1]]
                    uid = row[header_index[2]]
                    resolver_tag = row[header_index[3]]
                    resolver_AS = row[header_index[4]]
                    rr_type = row[header_index[5]]
                    query_cc = row[header_index[6]]
                    self.add_query(uid, query_time, query_AS, query_cc, rr_type, resolver_tag, resolver_AS)
                    nb_events += 1

        return nb_events

    def add_slices(self, slice_list):
        nb_events = 0
        nb_processed = 0

        for uid in self.uid_list:
            event = self.uid_list[uid]
            nb_events += 1
            if slice_list.add_event(event):
                nb_processed += 1

        print("Processed " + str(nb_processed) + " out of " + str(nb_events) + " events.")

# Main program
if __name__ == "__main__":
    time_start = time.time()
    if len(sys.argv) < 3:
        usage()
        exit(-1)

    output_dir = sys.argv[1]
    if not os.path.isdir(output_dir):
        print("Invalid output dir: " + output_dir)
        usage()
        exit(-1)
    as_list = rsv_arguments.parse_AS_list(sys.argv[2:])
    csv_files, has_error = rsv_arguments.parse_file_list(sys.argv[2 + len(as_list):], [ ".csv"])
    if has_error:
        print("Invalid list of input files.")
        usage()
        exit(-1)


    dups_list = [ duplicate_slices(7200, "") , duplicate_AS_list(), duplicate_as_details(7200, as_list) ]

    first_time = 0
    for csv_file in csv_files:
        dq = duplicate_queries()
        nb_events = dq.load_csv_log(csv_file)
        print(csv_file + ": " + str(nb_events) + " events, " + str(len(dq.uid_list)) + " unique ids.")
        if first_time == 0:
            first_time = dq.first_time
            for dups in dups_list:
                dups.first_time = first_time
        for dups in dups_list:
            dq.add_slices(dups)

    metric_df = dups_list[0].get_df()
    metric_file = os.path.join(output_dir, "duplicate_metric.csv" )
    metric_df.to_csv(metric_file, sep=",")
    print("Saved: " + str(metric_df.shape[0]) + " time slices in " + metric_file)

    as_df = dups_list[1].get_df()
    as_file = os.path.join(output_dir, "duplicate_as_list.csv" )
    as_df.to_csv(as_file, sep=",")
    print("Saved: " + str(as_df.shape[0]) + " AS in " + as_file)

    dups_list[2].save_files(output_dir)



