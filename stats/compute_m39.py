#!/usr/bin/env python
# coding=utf-8
#
# This script is designed to update the M3.9 metrics, based on a set of M3 files.
# The M3 files are read from a source directory, organized by dates.
# The file for 2021-01-31 is directly copied to output.
# The files with dates larger than 2021-01-31 are parsed to obtain the
# average "non chromoid" for length 7 to 15. They are then copied to output,
# unchanged.
# The files with dates lower than 2021 are parsed to obtain the sum of
# values for length 7 to 15. This provides an over estimate of the fraction of
# chromioids, from which we subtract the "non chromioid" value estimated
# previously. The files are copied line by line. If M3.9 is found, it is
# copied. If not found, it is copied before M3.10, or at the end of file if M3.10 is
# not present.

import os
from os.path import isfile, join
import sys

# Main
in_dir = sys.argv[1]
out_dir = sys.argv[2]
jan31csv =  "M3-2021-01-31.csv"
m333 = "M3.3.3"

# first pass
nb_up_to_date = 0
v_up_to_date = 0
for x in os.listdir(in_dir):
    y = join(in_dir, x)
    if isfile(y) and len(x) == len(jan31csv) and x.startswith("M3-") and x > jan31csv:
        v_plus = 0.0
        for line in open(y, "rt"):
            # Example: M3.3.3,2020-12-31,v2.00, length_10, 0.038009,
            try:
                parts = line.split(",")
                p0 = parts[0].strip();
                p3 = parts[3].strip();
                p4 = parts[4].strip();
                if p0 == m333 and p3.startswith("length_"):
                    l = int(p3[7:])
                    if l >= 7 and l <= 15:
                        v_plus += float(p4)
            except:
                print("Fail to parse: " + line.strip())
        if v_plus > 0:
            nb_up_to_date += 1
            v_up_to_date += v_plus
            print("For " + x + " found excess = " + "{:8.6f}".format(v_plus))
v_average = v_up_to_date / nb_up_to_date
print("Average excess over " + str(nb_up_to_date) + " files: " + str(v_average))

# second pass
for x in os.listdir(in_dir):
    y = join(in_dir, x)
    if isfile(y) and len(x) == len(jan31csv) and x.startswith("M3-"):
        z = join(out_dir,x)
        with open(z,"wt") as f:
            v_m39 = -v_average
            m39_date = ""
            computed =  x >= jan31csv
            for line in open(y, "rt"):
                # Example: M3.3.3,2020-12-31,v2.00, length_10, 0.038009,
                if not computed:
                    parts = line.split(",")
                    p0 = parts[0].strip();
                    mp = p0.split(".")
                    if mp[0].strip() == "M3" and len(mp) >= 2:
                        sub_met = int(mp[1])
                        if m39_date == "":
                            m39_date = parts[1].strip()
                        if p0 == m333:
                            p3 = parts[3].strip();
                            p4 = parts[4].strip();
                            if p3.startswith("length_"): 
                                l = int(p3[7:])
                                if l >= 7 and l <= 15:
                                    v_m39 += float(p4)
                        elif sub_met > 9:
                            f.write("M3.9,2020-10-31,v2.02,," + "{:8.6f}".format(v_m39) + ",\n")
                            print("For " + x + ", inserted M3.9 = " + "{:8.6f}".format(v_m39))
                            computed = True
                        elif sub_met == 9:
                            print("For " + x + ", found M3.9 (" + m39_date + ") = " + line.strip())
                            line = parts[0] + "," + m39_date + "," + parts[2] + ",," + parts[4] + ",\n"
                            print("Replace by: " + line.strip())
                            computed = True
                f.write(line)
            if not computed:
                f.write("M3.9,2021-10-31,v2.02,," + "{:8.6f}".format(v_m39) + ",\n")
                print("For " + x + ", computed M3.9 = " + "{:8.6f}".format(v_m39))








        


