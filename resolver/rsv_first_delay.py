# APNIC test.
#
# Load an APNIC trace and store the filtered and parsed version in a csv file
# 
# Usage: python rsv_as_study.py <csv_file> <log_file> <ASxxxx> <source_directory>

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

def usage():
    print("Usage: python rsv_first_pass.py <csv_file> <log_file> <ASxxxx>\n")
    print("This script will parse the log file, extract data for the specified ASes,")
    print("and save the parsed data in the csv file.")
    print("If no AS is specified, retains all ASes with more than 1000 UIDs.")

# We are doing a detailed study of repetitions.

class delay_uid:
    def __init__(self, query_cc, query_AS, first_time):
        self.query_cc = query_cc
        self.query_AS=query_AS
        self.first_time = first_time
        self.nb_repeats = 0
        self.first_delay = 0
        self.max_delay = 0

class delay_uids_log:
    def __init__(self):
        self.uids=dict()
        self.cc_as_dict=dict()

    def add_cc_as(self,  query_cc, query_AS):
        key = query_cc + "-" + query_AS
        if not key in self.cc_as_dict:
            # print("Adding: " + key)
            self.cc_as_dict[key] = delay_query_as(query_cc, query_AS)
        return key

    def add_entry(self, uid, query_time, query_cc, query_AS, query_ad_time):
        if not uid in self.uids:
            if query_time < query_ad_time + 10.0:
                # this is a new query that we can track
                # print("Adding " + uid)
                self.uids[uid] = delay_uid(query_cc, query_AS, query_time)
            else:
                #todo: tabulate "late repeats" by query AS
                key = self.add_cc_as(query_cc, query_AS)
                self.cc_as_dict[key].late_repeats += 1

        elif self.uids[uid].nb_repeats == 0:
            self.uids[uid].first_delay = query_time - self.uids[uid].first_time
            self.uids[uid].max_delay = self.uids[uid].first_delay
            self.uids[uid].nb_repeats = 1
        else:
            self.uids[uid].max_delay = query_time - self.uids[uid].first_time
            self.uids[uid].nb_repeats += 1


    def get_delay_log(self, log_file, log_threshold=15625, time_start=0):
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
                if x.filter(rr_types=['A'], experiment=['0du'], query_delay=1000000000):
                    self.add_entry(x.query_user_id, x.query_time, x.query_cc, x.query_AS, x.query_ad_time)
                    nb_events += 1
                    if (nb_events%lth) == 0:
                        new_time = time.time() - time_start
                        if time_start > 0:
                            print("loaded " + str(nb_events) + " events at " + str(new_time))
                        else:
                            print("loaded " + str(nb_events) + " events.")
                        if lth < 1000000:
                            lth *= 2

    def tabulate(self):
        for uid in self.uids:
            y = self.uids[uid]
            key = self.add_cc_as(y.query_cc, y.query_AS)
            self.cc_as_dict[key].tabulate(y)

    def get_df(self):
        t = []
        for key in self.cc_as_dict:
            x = self.cc_as_dict[key].get_row()
            t.append(x)
        try:
            df = pd.DataFrame(t, columns= delay_query_as.get_row_header())
        except:
            print(str(len(t[0])) + " != " + str(len(delay_query_as.get_row_header())))
            print(str(delay_query_as.get_row_header()))
            print(str(t[0]))
            exit(-1)
        return df

# Main program
if __name__ == "__main__":
    time_start = time.time()
    if len(sys.argv) < 3:
        usage()
        exit(-1)

    csv_delay_file = sys.argv[1]
    log_file = sys.argv[2]

    dul = delay_uids_log()
    print("Reading: " + log_file)

    dul.get_delay_log(log_file, time_start=time_start)
    time_file_read = time.time()
    print("File " + log_file + " read in " + str(time.time() - time_start) + " seconds.")
    print("Got " + str(len(dul.uids)) + " uids, " + str(len(dul.cc_as_dict)) + " CC/AS.")
    dul.tabulate()
    time_tab = time.time()
    print("File tabulated in " + str(time.time() - time_start) + " seconds.")
    print("After tabulation, " + str(len(dul.cc_as_dict)) + " CC/AS.")
    df = dul.get_df()
    if df.shape[0] == 0:
        print("No event found. Are you sure this is a correct file?")
    else:
        df.to_csv(csv_delay_file)
        print("Saved " + str(df.shape[0]) + " events to " + csv_delay_file + " at " + str(time.time() - time_start) + " seconds.")

    exit(0)


