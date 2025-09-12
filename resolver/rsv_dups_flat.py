# 
# rsv_cloud_flat.py cloud_metric.csv
#
# From a cloud metric file, create a cloud_metric_flat.csv,
# with all three A, AAAA and HTTPS metrics on the same line.
#


import sys
import os
from pathlib import Path
import csv
import pandas as pd

class dups_flat_slice:
    def __init__(self):
        self.metric_a = 0
        self.uids_a = 0
        self.dups_a = 0
        self.metric_aaaa = 0
        self.uids_aaaa = 0
        self.dups_aaaa = 0
        self.metric_https = 0
        self.uids_https = 0
        self.dups_https = 0

    def add_metric(self, rr_name, dups_metric, uids, dups):
        if rr_name == 'A':
            self.metric_a = dups_metric
            self.uids_a = uids
            self.dups_a = dups
        elif rr_name == 'AAAA':
            self.metric_aaaa = dups_metric
            self.uids_aaaa = uids
            self.dups_aaaa = dups
        elif rr_name == 'HTTPS':
            self.metric_https = dups_metric
            self.uids_https = uids
            self.dups_https = dups

    def get_row(self, slice_time):
        return [ slice_time, self.metric_a, self.uids_a, self.dups_a,
                self.metric_aaaa, self.uids_aaaa, self.dups_aaaa,
                self.metric_https, self.uids_https, self.dups_https ]

    def columns():
        return [ 'slice_time', 'metric_a', 'uids_a', 'dups_a', 
                'metric_aaaa', 'uids_aaaa', 'dups_aaaa',
                'metric_https', 'uids_https', 'dups_https' ]

class dups_entries:
    def __init__(self):
        self.slices = dict()

    def add_metric(self, time_slice, rr_name, dups_metric, uids, dups):
        if not time_slice in self.slices:
            self.slices[time_slice] = dups_flat_slice()
        self.slices[time_slice].add_metric(rr_name, dups_metric, uids, dups)

    # load the input file
    def load_metric_csv(self, cloud_file):
        slices = dict()
        nb_in = 0
        with open(cloud_file, newline='') as csvfile:
            rsv_reader = csv.reader(csvfile, delimiter=',', quotechar='"')
            is_first = True
            is_second = True
            header_row = [ 'time_slice', 'rr_name', 'dups_metric', 'uids', 'dups' ]
            header_index = [ 1, 2, 3, 4, 5 ]

            for row in rsv_reader:
                if is_first:
                    is_first = False
                else:
                    if (is_second):
                        #print(",".join(row))
                        #for i in range(0, len(header_row)):
                        #    print(str(i) + ": x[" + header_row[i] + "] = " + str(row[header_index[i]]))
                        is_second = False
                    time_slice = float(row[header_index[0]])
                    rr_name = row[header_index[1]]
                    dups_metric = row[header_index[2]]
                    uids = row[header_index[3]]
                    dups = row[header_index[4]]
                    self.add_metric(time_slice, rr_name, dups_metric, uids, dups)
                    nb_in += 1
        return nb_in

    def get_df(self):
        v = []
        st = list(self.slices.keys())
        st.sort()
        for slice_time in st:
            x = self.slices[slice_time].get_row(slice_time)
            v.append(x)

        df = pd.DataFrame(v, columns=dups_flat_slice.columns())

        return df

def usage():
    print("Usage: python rsv_dups_flat.py <path-to-duplicate_metric.csv>")

# main

if len(sys.argv) != 2:
    usage()
    exit(-1)

srce_path = sys.argv[1]
if srce_path.endswith("_metric.csv"):
    dest_path = srce_path[:-4] + "_flat.csv"
else:
    dest_path = os.path.join(os.path.dirname(srce_path), "duplicate_metric_flat.csv")

dups_list = dups_entries()

if dups_list.load_metric_csv(srce_path) <= 0:
    print("Could not find any item in " + srce_path)
    exit(-1)

dups_df = dups_list.get_df()
dups_df.to_csv(dest_path)
print("saved " + str(dups_df.shape[0]) + " lines in " + dest_path)



