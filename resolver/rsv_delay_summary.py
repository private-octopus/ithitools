# rsv sum delays: take several "first delay" results, and summarize them
#
# The first delay is exported as:
# class delay_query_as:
#    def __init__(self, query_cc, query_AS):
#        self.query_cc = query_cc
#        self.query_AS=query_AS
#        self.late_repeats = 0
#        self.nb_uids = 0
#        self.nb_uids_repeated = 0
#        self.nb_repeats = 0
#        self.max_repeats = 0
#        self.max_delay = 0
#        self.first_repeat_slice = []
#
# We are going to read a CSV file, and look at the columns
# as a rank followed by the values.
# If the cc/as is not registered yet, the value read (minus the rank)
# becomes the summary. If it is, the composition rules are:
#
#        self.query_cc = same
#        self.query_AS = same
#        self.late_repeats = add two values 
#        self.nb_uids = add two values
#        self.nb_uids_repeated = add two values
#        self.nb_repeats = add two values
#        self.max_repeats = max of the two values
#        self.max_delay = max of the two values
#        self.first_repeat_slice = [] add two values for the whole vector.
#
# In addition to per cc/AS result, we will also compose global summary,
# the sum of all values.


import sys
import os
import rsv_log_parse
from rsv_log_parse import get_time_hour, get_slice_time
import pandas as pd
import traceback
import time
import calendar
import csv
import rsv_arguments
from rsv_delay_class import delay_query_as

class delay_lines:
    def __init__(self):
        self.cc_as_list = dict()

    def add_row(self, row):
        dqa = delay_query_as.from_row(row)
        key = dqa.query_cc + "-" + dqa.query_AS
        if key in self.cc_as_list:
            self.cc_as_list[key].add(dqa)
        else:
            self.cc_as_list[key] = dqa
        
    def load_delay_log(self, file_name):
        df = pd.read_csv(file_name, sep=",")
        print(file_name + ": " + str(df.shape[0]) + " lines.")
        df.apply(lambda row: self.add_row(row),axis=1)
        print("After loading " + file_name + ", " + str(len(self.cc_as_list)) + " CC/AS.")

    def get_df(self):
        t = []
        for key in self.cc_as_list:
            x = self.cc_as_list[key].get_row()
            t.append(x)

        t.sort(key=lambda x:x[3], reverse=True)

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
    if len(sys.argv) < 2:
        usage()
        exit(-1)

    output_dir = sys.argv[1]
    if not os.path.isdir(output_dir):
        print("Invalid output dir: " + output_dir)
        usage()
        exit(-1)

    csv_files, has_error = rsv_arguments.parse_file_list(sys.argv[2:], [ ".csv"])
    if has_error:
        print("Invalid list of input files.")
        usage()
        exit(-1)
    
    dl = delay_lines()
    for csv_file in csv_files:
        dl.load_delay_log(csv_file)

    as_df = dl.get_df()
    as_file = os.path.join(output_dir, "delay_as_list.csv" )
    as_df.to_csv(as_file, sep=",")
    print("Saved: " + str(as_df.shape[0]) + " AS in " + as_file)