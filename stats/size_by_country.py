
#!/usr/bin/python
# coding=utf-8
#
# This script takes as input a summary of IP address file produced
# by "cout_ip.py", and produces a summary by country. For each
# country, the summary has 4 categories:
# xx-0-top : traffic from top IP addresses in APNIC file
# xx-1-apnic : traffic from non-top IP addresses in APNIC file
# xx-2-tiny : traffic from addresses with little traffic
# xx-3-non-apnic : traffic from other addresses, not in APNIC file
# This requires as input the APNIC list of resolvers, and also
# the list of "others" addresses produced by "size_by_ip.py"

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

class cc_stats:
    def __init__(self, cc):
        self.cc = cc
        self.nb_top = 0
        self.nb_apnic = 0
        self.nb_tiny = 0
        self.nb_non_apnic = 0
        self.total_top = 0
        self.total_apnic = 0
        self.total_tiny = 0
        self.total_non_apnic = 0
        self.chp = 0
        self.nx_non_chp = 0
        self.good = 0

    def add(self, other):
        self.nb_top += other.nb_top
        self.nb_apnic += other.nb_apnic
        self.nb_tiny += other.nb_tiny
        self.nb_non_apnic += other.nb_non_apnic
        self.total_top += other.total_top
        self.total_apnic += other.total_apnic
        self.total_tiny += other.total_tiny
        self.total_non_apnic += other.total_non_apnic
        self.chp += other.chp
        self.nx_non_chp += other.nx_non_chp
        self.good += other.good

    def csv_head():
        s = "cc,nb_top,nb_apnic,nb_tiny,nb_non_apnic,"
        s += "total_top,total_apnic,total_tiny,total_non_apnic,"
        s += "chp,nx_non_chp,good"
        return s + "\n"

    def to_csv(self):
        s = str(self.cc)+","+str(self.nb_top)+","+str(self.nb_apnic)+","+str(self.nb_tiny)+","
        s += str(self.nb_non_apnic)+","+str(self.total_top)+","+str(self.total_apnic)+","
        s += str(self.total_tiny)+","+str(self.total_non_apnic,)+","+str(self.chp)+","
        s += str(self.nx_non_chp)+","+str(self.good)
        return s + "\n"


# Main loop
if len(sys.argv) != 5:
    print("Usage: " + sys.argv[0] + " <apnic_list.csv> <non_apnic_count.csv> <country_file.csv> <count_folder> \n")
    exit(1)

frequent_ip_file = sys.argv[1]
file_non_tiny = sys.argv[2]
file_countries = sys.argv[3]
folder_in = sys.argv[4]

fip = frequent_ip.frequent_ip()
fip.load(frequent_ip_file)
print("loaded " + str(len(fip.table)) + " addresses from APNIC frequent list.")
print("largest: " + str(fip.largest) + ", limit_10000: " + str(fip.limit_10000) + ", smallest:" + str(fip.smallest))

non_tiny = dict()
print("loading non tiny file: " + file_non_tiny)
for line in open(file_non_tiny, "rt"):
    ip = line.strip()
    non_tiny[ip] = ip
print("loaded " + str(len(non_tiny)) + " addresses from " + file_non_tiny)

cc_list = dict()

sum_t = 0
sum_n = 0

for f in os.listdir(folder_in):
    line = ""
    try:
        y = join(folder_in, f)
        if isfile(y):
            # Get the country code from the name of the file.
            f_parts = f.split("-")
            cc = f_parts[1]
            print("Loading count file: " + y + ", country: " + cc);
            # Extract the summary data for the country
            for line in open(y, "rt"):
                al = address_file_line("")
                al.from_csv(line)
                ccl = cc_stats(cc)
                if len(al.ip) > 0:
                    if al.ip in fip.table:
                        frequent = fip.table[al.ip].count_users_weighted  
                        if frequent > fip.limit_10000:
                            ccl.nb_top = 1
                            ccl.total_top = al.total()
                        else:     
                            ccl.nb_apnic = 1
                            ccl.total_apnic = al.total()
                    else:
                        if al.ip in non_tiny:
                            ccl.nb_non_apnic = 1
                            ccl.total_non_apnic = al.total()
                        else:
                            ccl.nb_tiny = 1
                            ccl.total_tiny = al.total()
                    ccl.chp = al.dga
                    ccl.nx_non_chp = al.nx_non_dga()
                    ccl.good = al.good()
                    if cc in cc_list:
                        cc_list[cc].add(ccl)
                    else:
                        cc_list[cc] = ccl;
    except:
        traceback.print_exc()
        print("Cannot parse: " + f + "," + line)
        exit(1)

with open(file_countries,"wt") as wc:
    wc.write(cc_stats.csv_head())
    for cc in sorted(cc_list):
        wc.write(cc_list[cc].to_csv())

