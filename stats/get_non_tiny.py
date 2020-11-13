#!/usr/bin/python
# coding=utf-8
#
# Quick extraction of the list of IP addresses in the "non tiny" category

import codecs
import sys
from address_file import address_line, address_file_line
import traceback
import ipaddress
import ip2as
import datetime
import math
import os
from os.path import isfile, join
import frequent_ip
from ip_summary import subnet_string


# Main loop
if len(sys.argv) != 3:
    print("Usage: " + sys.argv[0] + " <non_apnic_count.csv> <non_tiny_list.csv> \n")
    exit(1)
    
file_others = sys.argv[1]
file_non_tiny = sys.argv[2]

print("loading others file: " + file_others)
other_count = 0
non_tiny_count = 0
with open(file_non_tiny, "wt") as w:
    for line in open(file_others, "rt"):
        other_count += 1
        if (other_count%1000000) == 0:
            print(other_count)
        try:
            al = address_file_line("")
            al.from_csv(line)
            if len(al.ip) > 0 and al.total() > 1000:
                w.write(al.ip + "\n")
                non_tiny_count += 1
        except:
            traceback.print_exc()
            print("Cannot parse: " + line)
            exit(1)
print("loaded " + str(non_tiny_count) + " addresses out of " + str(other_count) + " from other resolvers list.")