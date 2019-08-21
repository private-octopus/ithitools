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

def ithiwalk(file_list, path):
    print(path)
    for x in os.listdir(path):
        y = join(path, x)
        if isfile(y):
            file_list.append(y)
        else:
            ithiwalk(file_list, y)

def load_m3(file_name, sum_m3):
    m3sl = m3summary.m3summary_line()
    if m3sl.load_m3(file_name) != 0:
        return -1
    sum_m3.write(m3sl.to_string() +  "\n")
    return 0

# By default, we expect to test with the file "data/tiny_capture.csv, which has 589 lines.

mypath = sys.argv[1]
name_sum_f3 = "sum_f3.csv"
if len(sys.argv) >= 3:
    name_sum_f3 = sys.argv[2]
sum_m3 = codecs.open(name_sum_f3, "w", "UTF-8")
sum_m3.write(m3summary.m3summary_line.title_line() + "\n")

file_list = []
ithiwalk(file_list,mypath)
nb_loaded = 0
for file in file_list:
    # If this is an M3 capture file, add it.
    if load_m3(file, sum_m3) == 0:
        nb_loaded += 1
sum_m3.close()

print("Found " + str(len(file_list)) + " files, loaded " + str(nb_loaded) + " summaries.")
