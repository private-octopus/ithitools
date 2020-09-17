
#!/usr/bin/python
# coding=utf-8
#
# Explore categorization of IP addresses based on the last digit (IPv4) or last 64 bits
# of IPv6 addresses. Also produces a sample file to test various categories.
# 

import codecs
import sys
from address_file import address_line, address_file_line
import traceback
import ipaddress
import ip2as
import datetime
import math
from os.path import isfile, join
import frequent_ip
from enum import Enum
import ipaddress
from ip_summary import summary_enum, subnet_string, address_summary_line, address_summary

class ip_suffix_list:
    def __init__(self, tag):
        self.tag = tag
        self.count = 0
        self.total = 0
        self.dga = 0
        self.good = 0
        self.invalid = 0
        self.nb_v6 = 0
        self.nb_v4 = 0
        self.v6_count = []
        self.v4_count = []
        self.dup_sample = []
        self.sample = []
        for x in range(0,16):
            self.v6_count.append(0)
            self.v4_count.append(0)
    def add(self,ip,total,dga,good,nb_files):
        self.count += 1
        self.total += total
        self.dga += dga
        self.good += good
        if len(self.sample) < 10:
            self.sample.append(ip)
        elif nb_files > 1 and len(self.dup_sample) < 10:
            self.dup_sample.append(ip) 
        try:
            ipa = ipaddress.ip_address(ip)
            if ipa.version == 4:
                parts = ip.split(".")
                last = int(parts[3])
                x = int(last/16)
                self.v4_count[x] += 1
                self.nb_v4 += 1
            else:
                long_ip6 = ipa.exploded
                parts = long_ip6.split(":")
                x = 0
                for i in range(4,8):
                    if parts[i] == '0000' and i != 7:
                        x += 4
                    else:
                        p = parts[i]
                        for j in range(0,3):
                            if p[j] == '0':
                                x += 1
                            else:
                                break
                self.v6_count[x] += 1
                self.nb_v6 += 1
        except:
            traceback.print_exc()
            print("Cannot parse: " + ip)
            self.invalid += 1

    def csv_head():
        s = "tag,count,total,dga,good,invalid,nb_v4,nb_v6,"
        for x in range(0,16):
            s += "v4." + str(x*16) + "-" + str((x+1)*16-1)+","
        for x in range(0,16):
            s += "v6." + str(x) + "*0,"
        return s

    def to_csv(self):
        s = self.tag + ","
        s += str(self.count) + ","
        s += str(self.total) + ","
        s += str(self.dga) + ","
        s += str(self.good) + ","
        s += str(self.invalid) + ","
        s += str(self.nb_v4) + ","
        s += str(self.nb_v6) + ","
        for x in range(0,16):
            s += str(self.v4_count[x]) + ","
        for x in range(0,16):
            s += str(self.v6_count[x]) + ","
        return s

# Main loop
if len(sys.argv) < 7:
    print("Usage: " + sys.argv[0] + " nb_days <frequent_ip.csv> <as_names.csv> <ip_classes.csv> <ip_samples.csv> <count_file.csv>*\n")
    exit(1)

nb_days = int(sys.argv[1])
max_slice = nb_days*24*12
frequent_ip_file = sys.argv[2]
as_name_file = sys.argv[3]
file_suffix = sys.argv[4]
file_sample = sys.argv[5]
files_in =sys.argv[6:]

as_dict = ip2as.asname()
as_dict.load(as_name_file)
print("Loaded " + str(len(as_dict.table)) + " AS names.")

fip = frequent_ip.frequent_ip()
fip.load(frequent_ip_file)
print("loaded " + str(len(fip.table)) + " addresses from APNIC frequent list.")
print("largest: " + str(fip.largest) + ", limit_10000: " + str(fip.limit_10000) + ", smallest:" + str(fip.smallest))

ip_sum = address_summary(summary_enum.by_ip)

sum_t = 0
sum_n = 0

for f in files_in:
    try:
        ip_sum.add_address_file(f, as_dict.table, fip.table)
    except:
        traceback.print_exc()
        print("Cannot parse: " + f)
        exit(1)
    print("After parsing " + f + ", " + str(len(ip_sum.table)) + " addresses.")

#compute the respective shares of five classes of IP addresses
f_class_name = [ "top", "frequent", "frequent-dust", "others", "others-dust"]
f_class_suffix = []
for x in f_class_name:
    y = ip_suffix_list(x)
    f_class_suffix.append(y)

for ip in ip_sum.table:
    f_class = 2
    if ip_sum.table[ip].frequent > fip.limit_10000:
        f_class = 0
    elif ip_sum.table[ip].frequent > 0:
        if ip_sum.table[ip].total > 1000:
            f_class = 1
        else:
            f_class = 2
    elif ip_sum.table[ip].total > 1000:
        f_class = 3
    else:
        f_class = 4
    f_class_suffix[f_class].add(ip, ip_sum.table[ip].total, ip_sum.table[ip].dga, ip_sum.table[ip].good, ip_sum.table[ip].nb_files)

with open(file_suffix,"wt") as w_suf:
    w_suf.write(ip_suffix_list.csv_head() + "\n")
    for x in range(0,len(f_class_suffix)):
        w_suf.write(f_class_suffix[x].to_csv() + "\n")

with open(file_sample,"wt") as w_samp:
    w_samp.write("tag, dup, ip,")
    for x in range(0,len(f_class_suffix)):
        for ip in f_class_suffix[x].sample:
             w_samp.write(f_class_suffix[x].tag + ",0," + ip + "\n")
        for ip in f_class_suffix[x].dup_sample:
             w_samp.write(f_class_suffix[x].tag + ",1," + ip + "\n")