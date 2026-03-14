# We find cases where the number of "A queries within 0 second of first"
# is larger than the number of "unique A queries". This feels wrong.
# we take a sample to find out how this could happen.
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

class dupabug_uid:
    def __init__(self, uid, query_cc, query_AS, query_time):
        self.uid = uid
        self.query_cc = query_cc
        self.query_AS = query_AS
        self.query_time = query_time
        self.query_ad_time = 0
        self.hit_count = 0
        self.hit_file = [ "", "", "" ]
        self.hit_event = [ 0, 0, 0 ]

    def hit(self, file_name, nb_event):
        self.hit_count += 1
        if self.hit_count < 3:
            self.hit_file[self.hit_count] = file_name
            self.hit_event[self.hit_count] = nb_event

    def columns():
        return("uid,query_cc,query_AS,hits,f1,nb1,f2,nb2,f3,nb3,")

    def get_s(self):
        s = ""
        s += self.uid + ","
        s += self.query_cc + ","
        s += self.query_AS + ","
        s += str(self.hit_count) + ","
        for i in range(0, 3):
            s += self.hit_file[i] + ","
            s += str(self.hit_event[i]) + ","
        return s


class dupabug_log:
    def __init__(self):
        self.uids=dict()

    def add_entry(self, uid, query_cc, query_AS, query_time, query_ad_time, file_name, nb_event):
        if not uid in self.uids:
            self.uids[uid] = dupabug_uid(uid, query_cc, query_AS, query_time)
        if self.uids[uid].query_time <= query_time:
            self.uids[uid].hit(file_name, nb_event)

    def load_dupabug_log(self, log_file, log_threshold=15625, time_start=0):
        nb_events = 0
        nb_dups = 0

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
                if x.filter(rr_types=['A'], experiment=['0du'], query_delay=30, check_dotnxdomain=True):
                    self.add_entry(x.query_user_id, x.query_cc, x.query_AS, x.query_time, x.query_ad_time, log_file, nb_events)
                    nb_events += 1
                    if (nb_events%lth) == 0:
                        new_time = time.time() - time_start
                        print(log_file + ": loaded " + str(nb_events) + " events at " + str(new_time) + ", " + str(nb_dups) + " dups.")
                        sys.stdout.flush()
                        if lth < 1000000:
                            lth *= 2
        new_time = time.time() - time_start
        print(log_file + ": loaded " + str(nb_events) + " events at " + str(new_time) + ", " + str(nb_dups) + " dups.")

    def save_and_close(self, file_name):
        with open(file_name, "w") as f:
            f.write(dupabug_uid.columns() + "\n")
            for uid in self.uids:
                if self.uids[uid].nb_hits > 1:
                    f.write(self.uids[uid].get_s() + "\n")

# main

if len(sys.argv) < 3:
    print("usage: dupabug.py <output.csv> *<input_log>")
    print("Only " + str(len(sys.argv)) + " arguments provided.")
    exit(-1)

dbl = dupabug_log()
time_start = time.time()

for log_file in sys.argv[2:]:
    nb_events = dbl.load_dupabug_log(log_file, log_threshold=15625, time_start=time_start)

dbl.save_and_close(sys.argv[1])
