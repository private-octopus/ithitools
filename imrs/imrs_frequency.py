#
# This script will try to build a sample of the input file.
# The purpose of the sample is, get a realistic test file
# that is small enough for iterative development, measures,
# etc., yet big enough to obtain statistically significant
# results.
#
# Usage: imrs_sample.py <input_file> <sampling rate in %> <output_file>
#

import sys
import traceback
import random
import time
import concurrent.futures
import math
import os
from os import listdir
from os.path import isfile, isdir, join

def parse_imrs(line):
    ok = False
    ip = ""
    count = 0
    try:
        parts = line.split(",")
        ip = parts[0].strip()
        count = int(parts[1].strip())
        ok = True
    except Exception as e:
        traceback.print_exc()
        print("Cannot parse IMRS Record:\n" + line.strip()  + "\nException: " + str(e))
    return ok, ip, count


# main

if len(sys.argv) < 3 or len(sys.argv) > 4:
    print("Usage: imrs_frequency.py <imrs_file> <output_file> [load_step%]")
    exit(1)
imrs_file = sys.argv[1]
output_file = sys.argv[2]
load_step = 0
if len(sys.argv) == 4:
    s_load_step = sys.argv[3]
    if not s_load_step.endswith("%"):
        print("Load step should be %, e.g. 1%, 0.1%, not " + s_load_step)
        exit(1)
    else:
        load_step = float(s_load_step[:-1])/100.0

load_vec = []

total_load = 0
for line in open(imrs_file,"r"):
    ok, ip, use_count = parse_imrs(line)
    if ok:
        load_vec.append(use_count)
        total_load += use_count

load_vec.sort(reverse=True)

with open(output_file, "w") as F:
    cumulative_use = 0
    cumulative_count = 0
    delta_threshold = int(total_load*load_step)
    threshold = 0
    last_written = 0
    F.write("Count, Queries, frequency,\n")
    for use_count in load_vec:
        cumulative_count += 1
        cumulative_use += use_count
        if cumulative_use >= threshold:
            F.write(str(cumulative_count) + "," + str(cumulative_use) + "," + str(cumulative_use/total_load) + ",\n")
            threshold += delta_threshold
            last_written = cumulative_count
    if last_written < cumulative_count:
        F.write(str(cumulative_count) + "," + str(cumulative_use) + "," + str(cumulative_use/total_load))