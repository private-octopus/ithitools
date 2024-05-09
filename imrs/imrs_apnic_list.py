#
# This script will build a list of IP addresses present in both
# the APNIC and IMRS files, providing IMRS and APNIC query volume
# from that address.
#
# Usage: imrs_apnic_list.py <imrs_file> <apnic_file> <output_file>
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
import imrs
from imrs import parse_imrs_volume_only, apnic_record, apnic_load

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


# main

if len(sys.argv) != 4:
    print("Usage: imrs_apnic_list.py <imrs_file> <apnic_file> <output_file>")
    exit(1)
imrs_file = sys.argv[1]
apnic_file = sys.argv[2]
output_file = sys.argv[3]

apnic_dict = apnic_load(apnic_file)

for line in open(imrs_file,"r"):
    ok, ip, count = parse_imrs_volume_only(line)
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
