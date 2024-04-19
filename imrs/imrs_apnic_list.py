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

class imrs_apnic_item:
    def __init__(self, ip, apnic_use, imrs_use):
        self.ip = ip
        self.apnic_use = apnic_use
        self.imrs_use = imrs_use

    def head():
        s = "IP, apnic_use, imrs_use,"
        return s


    def text(self):
        s = ip + "," + str(apnic_use) + "," + str(imrs_use) + ","
        return s

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
    print("Usage: imrs_apnic_list.py <imrs_file> <apnic_file> <output_file>")
    exit(1)
imrs_file = sys.argv[1]
apnic_file = sys.argv[2]
output_file = sys.argv[3]

apnic_dict = dict()

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

with open(output_file, "w") as F:
    F.write("IP, apnic_use, imrs_use,\n")
    for ip in apnic_dict:
        apnic_entry = apnic_dict[ip]
        if apnic_entry.seen_in_imrs:
            F.write(apnic_entry.ip + "," + str(apnic_entry.use_count) + "," + str(apnic_entry.imrs_count) + "\n")
