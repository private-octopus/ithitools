#!/usr/bin/python
# coding=utf-8
#
# This scripts tries to estimate the variance of a few M3 test metrics.

import codecs
import sys
import m3name
import captures
import m3summary
import os
from os.path import isfile, join

def load_m3(file_name, sum_m3):
    m3sl = m3summary.m3summary_line()
    if m3sl.load_m3(file_name) != 0:
        return -1
    sum_m3.write(m3sl.to_string() +  "\n")
    return 0


def load_folder(mypath, name_sum_f3):
    sum_m3 = codecs.open(name_sum_f3, "w", "UTF-8")
    sum_m3.write(m3summary.summary_title_line() + "\n")

    file_list = []
    m3summary.ithiwalk(file_list,mypath)
    nb_loaded = 0
    for file in file_list:
        # If this is an M3 capture file, add it.
        if load_m3(file, sum_m3) == 0:
            nb_loaded += 1
            if (nb_loaded%1000) == 0:
                print(str(nb_loaded))
    sum_m3.close()
    print("In " + mypath + " found " + str(len(file_list)) + " files, loaded " + str(nb_loaded) + " summaries.")


# By default, we expect to test with the file "data/tiny_capture.csv, which has 589 lines.

my_top_folder = sys.argv[1]
my_sum_folder = sys.argv[2]
print("Listing: " + my_top_folder)
folder_list = os.listdir(my_top_folder)
print("Found: " + str(len(folder_list)) + " folders.")
for ff in folder_list:
    parts = ff.split(".")
    summary_name = my_sum_folder + parts[0] + ".csv"
    print("Composing: " + summary_name)
    load_folder(my_top_folder + ff, summary_name)
