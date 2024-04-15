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
import os
from os import listdir
from os.path import isfile, isdir, join

def read_next_line(F):
    line = ""
    is_done = False
    try:
        #read next line
        line = F.readline()
        if len(line) == 0:
            is_done = True
        elif len(line.strip()) == 0:
            print("Empty line!!")
    except EOFError as e:
        is_done = True
    return is_done, line

def progress_imrs(F_IMRS):
    ip = ""
    is_done = True
    is_empty = False
    is_done, line = read_next_line(F_IMRS)
    if not is_done:
        parts = line.split(",")
        if len(parts) > 1:
            ip = parts[0].strip()
        else:
            print("Empty imrs line: " + line)
            exit(1)
    return is_done, ip

def progress_apnic(F_APNIC):
    ip = ""
    is_done = True
    is_empty = False
    is_done, line = read_next_line(F_APNIC)
    if not is_done:
        parts = line.split(",")
        if len(parts) > 1:
            ip = parts[0].strip()
        else:
            print("Empty apnic line: " + line)
            is_empty = True
            exit(1)
    return is_done, ip

# main

if len(sys.argv) != 4:
    print("Usage: imrs_sample.py <imrs_file> <apnic_file> <output_file>")
    exit(1)
imrs_file = sys.argv[1]
apnic_file = sys.argv[2]
file_name_out = sys.argv[3]

nb_only_imrs = 0
nb_only_apnic = 0
nb_both = 0

imrs_ip = ""
apnic_ip = ""
imrs_done = False
apnic_done = False

try:
    with open(imrs_file,"r") as F_IMRS:
        with open(apnic_file,"r") as F_APNIC:
            # initialize the first line of IMRS
            imrs_done, imrs_ip = progress_imrs(F_IMRS)
            # initialize the first line of APNIC
            apnic_done, apnic_ip = progress_apnic(F_APNIC)
            while not imrs_done and not apnic_done:
                if ((nb_only_imrs + nb_both) % 1000) == 0:
                    print(str(nb_only_imrs) + "," + str(nb_both))
                if apnic_ip < imrs_ip:
                    nb_only_apnic += 1
                    apnic_done, apnic_ip = progress_apnic(F_APNIC)
                    pass
                elif imrs_ip < apnic_ip:
                    nb_only_imrs += 1
                    imrs_done, imrs_ip = progress_imrs(F_IMRS)
                    pass
                else:
                    print("Both: " + imrs_ip)
                    nb_both += 1
                    apnic_done, apnic_ip = progress_apnic(F_APNIC)
                    imrs_done, imrs_ip = progress_imrs(F_IMRS)
                    pass
            while not imrs_done:
                nb_only_imrs += 1
                imrs_done, imrs_ip = progress_imrs(F_IMRS)
            while not apnic_done:
                nb_only_apnic += 1
                apnic_done, apnic_ip = progress_apnic(F_APNIC)
except Exception as e:
        traceback.print_exc()
        print("Cannot open APNIC or IMRS file Exception: " + str(e))
        exit(1)

# report the values.
print("Only APNIC: " + str(nb_only_apnic))
print("Only IMRS: " + str(nb_only_imrs))
print("Both: " + str(nb_both))