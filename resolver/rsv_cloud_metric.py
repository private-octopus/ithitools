
# Cloud study
#
# We are proposing a new metric: share of cloud services.
# The share is defined as "fraction of transactions handled by a cloud service"
# 
#

import sys
import os
from pathlib import Path
import ip2as
import rsv_log_parse
from rsv_log_parse import get_time_hour, get_slice_time
import pandas as pd
import traceback
import top_as
import time
import calendar
import csv
import rsv_arguments

def usage():
    print("Usage: python rsv_cloud_metric.py <output_dir> [<list of AS] <csv_file> ... <csv_file>\n")
    print("This script will load the csv files, and produce output files:")
    print("   <output_dir>/cloud_metrics.csv: table of cloud usage per 1 hour interval.")
    print("   <output_dir>/cloud_ASnnnn.csv: table of cloud by 5 minute interval for ASnnnn.")
    print("   <output_dir>/cloud_as_list.csv: table of cloud usage per AS with more than 10000 queries.")
    print("   <output_dir>/cloud_list.csv: table of cloud usage per cloud AS.")


cloud_tracked = {
    'AS20940' : 0,
    'AS36692' : 1, 
    'AS36236' : 2,
    'AS54825' : 3,
    'AS16509' : 4 }

cloud_names = [ 'AKAMAI-ASN1', 'CISCO-UMBRELLA', 'NETACTUATE', 'PACKET' , 'AMAZON-02' ]

def get_time_hour(first_time):
    fth = int(first_time/3600)
    ft = fth*3600
    return ft

class cloud_slice:
    def __init__(self):
        self.nb_uid = 0
        self.nb_cloud = 0
        self.nb_both = 0
        self.nb_tracked = 0
        self.tracked = [0, 0, 0, 0, 0 ]
    
    def add_event(self, event):
        self.nb_uid += 1
        if event.has_cloud: 
            self.nb_cloud += 1
            if event.cloud_AS in cloud_tracked:
                self.tracked[cloud_tracked[event.cloud_AS]] += 1
            if event.has_other:
                self.nb_both += 1
        return True

class cloud_query:
    def __init__(self, uid, query_time, query_AS, query_cc):
        self.uid = uid
        self.query_time = query_time
        self.query_AS = query_AS
        self.query_cc = query_cc
        self.cloud_AS = ""
        self.has_cloud = False
        self.has_other = False

class cloud_slices:
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
            self.slices[slice_time] = cloud_slice()
        self.slices[slice_time].add_event(event)

        return True

    def get_df(self):
        v = []
        st = list(self.slices.keys())
        st.sort()
        for slice_time in st:
            if self.slices[slice_time].nb_uid > 0:
                cloud_metric = self.slices[slice_time].nb_cloud/self.slices[slice_time].nb_uid
                unique_uids = self.slices[slice_time].nb_cloud - self.slices[slice_time].nb_both
                unique_metric = unique_uids/self.slices[slice_time].nb_uid
                x = [ slice_time, cloud_metric, unique_metric, self.slices[slice_time].nb_uid,
                     self.slices[slice_time].nb_cloud, unique_uids]
                x += self.slices[slice_time].tracked
                v.append(x)

        v.sort(key=lambda x:x[0])

        headers = ["slice_time", "cloud_metric", "cloud_unique_metric", "uids", "cloud", "cloud_unique"]
        headers += cloud_names

        df = pd.DataFrame(v, columns=headers)

        return df

class cloud_cc_as_list:
    def __init__(self):
        self.AS_list = dict()

    def add_event(self, event):
        key = event.query_cc + event.query_AS
        if not key in self.AS_list:
            self.AS_list[key] =  cloud_slice()
        self.AS_list[key].add_event(event)
        return True

    def get_df(self, threshold=10000):
        v = []
        for key in self.AS_list:
            if self.AS_list[key].nb_uid >= threshold:
                cloud_metric = self.AS_list[key].nb_cloud/self.AS_list[key].nb_uid
                unique_uids = self.AS_list[key].nb_cloud - self.AS_list[key].nb_both
                unique_metric = unique_uids/self.AS_list[key].nb_uid
                x = [ key[0:2], key[2:], cloud_metric, unique_metric, self.AS_list[key].nb_cloud, unique_uids,
                    self.AS_list[key].nb_uid]
                x += self.AS_list[key].tracked
                v.append(x)

        v.sort(key=lambda x:x[4], reverse=True)

        headers = ["query_cc", "query_AS", "cloud_metric", "cloud_unique_metric", "uids", "cloud", "cloud_unique"]
        headers +=   cloud_names

        df = pd.DataFrame(v, columns=headers)

        return df

class cloud_share:
    def __init__(self):
        self.AS_list = dict()
        self.nb_cloud = 0
        self.nb_total = 0
        print("Cloud share created")

    def add_event(self, event):
        self.nb_total += 1
        cloud_AS = event.cloud_AS
        if event.has_cloud:
            self.nb_cloud += 1
        else:
            cloud_AS = ""
        if not cloud_AS in self.AS_list:
            self.AS_list[cloud_AS] =  0

        self.AS_list[cloud_AS] += 1
        return True

    def get_df(self, threshold=10000):        
        v = []
        for cloud_AS in self.AS_list:
            cloud_name = ""
            if cloud_AS in top_as.CloudAS:
                cloud_name = top_as.CloudAS[cloud_AS]
            resolver_share = self.AS_list[cloud_AS] / self.nb_total
            if cloud_AS.startswith("AS"):
                cloud_share = self.AS_list[cloud_AS] / self.nb_cloud
            else:
                cloud_share = 0.0
            x = [ cloud_AS, cloud_name, resolver_share, cloud_share, self.AS_list[cloud_AS]]
            v.append(x)
                
        v.sort(key=lambda x:x[3], reverse=True)

        df = pd.DataFrame(v, columns=["cloud_AS", "cloud_name", "resolver_share", "cloud_share", "uids"])

        return df

class cloud_queries:
    def __init__(self):
        self.uid_list = dict()
        self.first_time = 0
        self.tried = 0

    def add_query(self, uid, query_time, query_AS, query_cc, has_cloud, resolver_AS):
        if not uid in self.uid_list:
            self.uid_list[uid] = cloud_query(uid, query_time, query_AS, query_cc)
        if has_cloud:
            self.uid_list[uid].has_cloud = True
            self.uid_list[uid].cloud_AS = resolver_AS
        else:
            self.uid_list[uid].has_other = True
    
    # load the input files
    def load_csv_log(self, saved_file):
        nb_events = 0
        with open(saved_file, newline='') as csvfile:
            rsv_reader = csv.reader(csvfile, delimiter=',', quotechar='"')
            is_first = True
            is_second = True
            header_row = [ 'query_time', 'query_AS', 'query_user_id', 'resolver_tag', 'resolver_AS', 'query_cc' ]
            header_index = [ -1, -1, -1, -1, -1, -1 ]

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
                    query_cc = row[header_index[5]]
                    self.add_query(uid, query_time, query_AS, query_cc, resolver_tag == "Cloud", resolver_AS)
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

    cloud_list = [ cloud_slices(3600, "") , cloud_cc_as_list(), cloud_share() ]
    for query_AS in as_list:
        cloud_list.append(cloud_slices(300, query_AS))
        cloud_list.append(cloud_slices(3600, query_AS))

    first_time = 0
    for csv_file in csv_files:
        cq = cloud_queries()
        nb_events = cq.load_csv_log(csv_file)
        print(csv_file + ": " + str(nb_events) + " events, " + str(len(cq.uid_list)) + " unique ids.")
        if first_time == 0:
            first_time = cq.first_time
            for hts in cloud_list:
                hts.first_time = first_time
        for hts in cloud_list:
            cq.add_slices(hts)

    print ("Cloud share: " + str(len(cloud_list[2].AS_list)))

    metric_df = cloud_list[0].get_df()
    metric_file = os.path.join(output_dir, "cloud_metric.csv" )
    metric_df.to_csv(metric_file, sep=",")
    print("Saved: " + str(metric_df.shape[0]) + " time slices in " + metric_file)

    as_df = cloud_list[1].get_df()
    as_file = os.path.join(output_dir, "cloud_as_list.csv" )
    as_df.to_csv(as_file, sep=",")
    print("Saved: " + str(as_df.shape[0]) + " AS in " + as_file)

    share_df = cloud_list[2].get_df()
    share_file = os.path.join(output_dir, "clouds_list.csv" )
    share_df.to_csv(share_file, sep=",")
    print("Saved: " + str(share_df.shape[0]) + " cloud services in " + share_file)

    for asn_dup in cloud_list[3:]:
        asn = asn_dup.query_AS
        asn_file = os.path.join(output_dir, "cloud_" + asn + "_" + str(asn_dup.slice_duration) + ".csv" )
        asn_df = asn_dup.get_df()
        asn_df.to_csv(asn_file, sep=",")
        print("Saved: " + str(asn_df.shape[0]) + " time slices in " + asn_file)
