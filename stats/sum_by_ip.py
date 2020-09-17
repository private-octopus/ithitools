
#!/usr/bin/python
# coding=utf-8
#
# This script takes as input a summary of IP address file produced
# by "cout_ip.py", and produces a summary of the results
# There are several potential criteria outlined by previous research:
# - Is this IP address mentioned or not in the frequently seen list or not.
# - Is this address generating enough traffic to be interesting, e.g. more than 1,000 queries in a month?
# - Fraction of NX domains + DGA over total
# - Fraction of DGA over total
# - Fraction of .ARPA requests over total "good" queries
# - Fraction of .COM requests over total "good" queries
# - Fraction of time slices for which the address is seen
#
# Each of the "fractions" can have three values: low, high, in between.
# The threshold suggested by initial analysis are:
# - Traffic: low if < 1000 requests, high if > 1000000
# - NX: low if <10%, high if >90%
# - DGA: low if <10%, high if >90%
# - ARPA: low if <2%, high if >10%
# - COM: low if <5%, high if >20%
# - Slices: low if < 25%, high if > 90%
#
# The initial analysis may well be wrong, we will have to adjust it based on broader measurements.
# To facilitate this analysis, we will keep counters for each metric. The slices will be:
# - Traffic: count addresses that generate <10, <100, ... <1000000, >1000000 queries.
# - NX, DGA, COM, slices: count for each 5% slice
# - ARPA: count for each 1% slice, up to 20%
#
# Once criterias have stabilized, we want to count the number of nodes and the share of
# traffic in each "classification bucket". We have 6 tentative criteria and 3 classification dict_values
# (low, middle, high) for each, thus 729 possible buckets. However, some criteria may well
# be very correlated: slices and traffic, NX and DGA, ARPA and COM. This will show in
# the number of entries and volume of traffic corresponding to each bucket. Sorting
# the list by decreasing volume will show the interesting buckets.

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
from ip_summary import summary_enum, subnet_string, address_summary_line, address_summary

# Main loop
if len(sys.argv) < 8:
    print("Usage: " + sys.argv[0] + "nb_days <frequent_ip.csv> <as_names.csv> <ip_stats.csv> <subnet_stats.csv> <as_stats.csv> <as_name_stats.csv> <count_file.csv>* \n")
    exit(1)

nb_days = int(sys.argv[1])
max_slice = nb_days*24*12
frequent_ip_file = sys.argv[2]
as_name_file = sys.argv[3]
file_ip = sys.argv[4]
file_subnet = sys.argv[5]
file_as = sys.argv[6]
file_as_name = sys.argv[7]
files_in = sys.argv[8:]

as_dict = ip2as.asname()
as_dict.load(as_name_file)
print("Loaded " + str(len(as_dict.table)) + " AS names.")
x = 0
for key in as_dict.table:
    print(str(key) + ":" + str(as_dict.table[key]))
    x += 1
    if x > 10:
        break

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

#compute the respective shares of three classes of IP addresses
f_class_name = [ "top", "frequent", "others"]
f_class_count = [0, 0, 0]
f_class_total = [0, 0, 0]
f_class_dga = [0, 0, 0]
f_class_good = [0, 0, 0]
for ip in ip_sum.table:
    f_class = 2
    if ip_sum.table[ip].frequent > fip.limit_10000:
        f_class = 0
    elif ip_sum.table[ip].frequent > 0:
        f_class = 1
    f_class_count[f_class] += 1
    f_class_total[f_class] += ip_sum.table[ip].total
    f_class_dga[f_class] += ip_sum.table[ip].dga
    f_class_good[f_class] += ip_sum.table[ip].good

for f_class in range(0,3):
    print(f_class_name[f_class] + "," + \
        str(f_class_count[f_class]) + "," + str(f_class_total[f_class]) + "," + \
        str(f_class_dga[f_class]) + "," + str(f_class_good[f_class]))



#todo: add shares
nb_lines = ip_sum.save_as_csv(file_ip, 0, 0)
print("Saved " + str(nb_lines) + " lines in " + file_ip)

net_sum = address_summary(summary_enum.by_subnet)
net_sum.add_summary(ip_sum)
nb_lines = net_sum.save_as_csv(file_subnet, 0, 0)
print("Saved " + str(nb_lines) + " lines in " + file_subnet)

as_sum = address_summary(summary_enum.by_asn)
as_sum.add_summary(net_sum)
nb_lines = as_sum.save_as_csv(file_as, 0, 0)
print("Saved " + str(nb_lines) + " lines in " + file_as)

asname_sum = address_summary(summary_enum.by_asname)
asname_sum.add_summary(as_sum)
nb_lines = asname_sum.save_as_csv(file_as_name, 0, 0)
print("Saved " + str(nb_lines) + " lines in " + file_as_name)




