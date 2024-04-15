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

# main

if len(sys.argv) != 4:
    print("Usage: imrs_sample.py <imrs_file> <apnic_file> <output_file>")
    exit(1)
imrs_file = sys.argv[1]
apnic_file = sys.argv[2]
file_name_out = sys.argv[3]

apnic_set = set()

nb_only_imrs = 0
nb_only_apnic = 0
nb_both = 0

for line in open(apnic_file,"r"):
    parts = line.split(",")
    if len(parts) > 1:
        ip = parts[0].strip()
        if not ip in apnic_set:
            apnic_set.add(ip)

for line in open(imrs_file,"r"):
    parts = line.split(",")
    if len(parts) > 1:
        ip = parts[0].strip()
        if ip in apnic_set:
            nb_both += 1
        else:
            nb_only_imrs += 1

# report the values.
print("Only IMRS: " + str(nb_only_imrs))
print("Both: " + str(nb_both))
nb_apnic = len(apnic_set)
nb_only_apnic = nb_apnic - nb_both
print("Only APNIC: " + str(nb_only_apnic))