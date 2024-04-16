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

# APNIC record:
#
# each record is a resolver
# the columns are:
# 1 - IP address
# 2 - AS Number
# 3 - Country Code
# 4 - Use count (i.e. the number of queries from this source address)
# 5,6,... a list of paired values which are the AS of the end user and the number of queries from this AS
#
class apnic_record:
    def __init__(self):
        self.ip = ""
        self.use_count = 0
        self.seen_in_imrs = False
        self.imrs_count = 0

    def parse(self, line):
        parts = line.split(",")
        nb_parts = len(parts)
        if nb_parts >= 4:
            try:
                self.ip = parts[0].strip()
                self.use_count = int(parts[3].strip())
            except Exception as e:
                traceback.print_exc()
                print("Cannot parse APNIC Record:\n" + line.strip()  + "\nException: " + str(e))
                return False
        return True

class log_count:
    def __init__(self):
        self.log_count = []
        self.log_total = []
        self.count = 0
        self.queries = 0
    
    def add(self, queries):
        if queries > 0:
            lc = int(math.log10(queries))
            while len(self.log_count) <= lc:
                self.log_count.append(0)
                self.log_total.append(0)
            self.log_count[lc] += 1
            self.log_total[lc] += queries
            self.count += 1
            self.queries += queries

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

if len(sys.argv) != 4:
    print("Usage: imrs_sample.py <imrs_file> <apnic_file> <output_file>")
    exit(1)
imrs_file = sys.argv[1]
apnic_file = sys.argv[2]
output_file = sys.argv[3]

apnic_dict = dict()
imrs_only = log_count()
imrs_both = log_count()

for line in open(apnic_file,"r"):
    apnic = apnic_record()
    if apnic.parse(line):
        apnic_dict[apnic.ip] = apnic

for line in open(imrs_file,"r"):
    ok, ip, count = parse_imrs(line)
    if ok:
        if ip in apnic_dict:
            apnic_dict[ip].seen_in_imrs = True
            apnic_dict[ip].imrs_count = count
            imrs_both.add(count)
        else:
            imrs_only.add(count)

apnic_only = log_count()
apnic_both = log_count()
for ip in apnic_dict:
    if apnic_dict[ip].seen_in_imrs:
        apnic_both.add(apnic_dict[ip].use_count)
    else:
        apnic_only.add(apnic_dict[ip].use_count)

# report the values.
print("Only IMRS: " + str(imrs_only.count))
print("Both (IMRS): " + str(imrs_both.count))
print("Both(APNIC): " + str(apnic_both.count))
print("Only APNIC: " + str(apnic_only.count))

log_counts = [ imrs_only, imrs_both, apnic_only, apnic_both]
head_counts = [ "imrs_only", "imrs_both", "apnic_only", "apnic_both" ]

with open(output_file, "w") as F:
    lg_max = 0
    for lc in log_counts:
        if len(lc.log_count) > lg_max:
            lg_max = len(lc.log_count)

    F.write("rank, range, ")
    for hc in head_counts:
        F.write(hc + "_count," + hc + "_use,")
    F.write("\n")

    range_max = 1
    for lg in range(0, lg_max):
        s_range = str(range_max) + " to "
        range_max *= 10
        s_range += str(range_max - 1)
        F.write(str(lg) + "," + s_range + ",")
        for lc in log_counts:
            if lg >= len(lc.log_count):
                F.write("0,0,")
            else:
                F.write(str(lc.log_count[lg]) + "," + str(lc.log_total[lg]) + ",")
        F.write("\n")
