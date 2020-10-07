
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
from ip_summary import subnet_string

# Main loop
if len(sys.argv) < 5:
    print("Usage: " + sys.argv[0] + " <frequent_ip.csv>  <frequent_ip_stats.csv> <file_others.csv> <file_nets.csv> <count_file.csv>* \n")
    exit(1)

frequent_ip_file = sys.argv[1]
file_frq = sys.argv[2]
file_others = sys.argv[3]
file_nets = sys.argv[4]
files_in = sys.argv[5:]

fip = frequent_ip.frequent_ip()
fip.load(frequent_ip_file)
print("loaded " + str(len(fip.table)) + " addresses from APNIC frequent list.")
print("largest: " + str(fip.largest) + ", limit_10000: " + str(fip.limit_10000) + ", smallest:" + str(fip.smallest))

frqs = dict()
others = dict()
nets = dict()

sum_t = 0
sum_n = 0

for f in files_in:
    for line in open(f, "rt"):
        try:
            c = []
            for x in range(0,6):
                c.append(0)
            al = address_file_line("")
            al.from_csv(line)
            if len(al.ip) > 0:
                t = al.total()
                # TODO: if total is very large, add to selection.
                # TODO: if total is too small, do not include in analysis
                sum_t += t
                sum_n += 1
                c = []
                if al.ip in fip.table:
                    al.frequent = fip.table[al.ip].count_users_weighted                   
                    al.users = fip.table[al.ip].count_users 
                    if al.ip in frqs:
                        frqs[al.ip].add(al)
                    else:
                        frqs[al.ip] = al;
                elif al.ip in others:
                    others[al.ip].add(al)
                else:
                    others[al.ip] = al
                net_prefix = subnet_string(al.ip)
                if not net_prefix in nets:
                    nl = address_file_line("")
                    nl.ip = net_prefix
                    nl.as_name = al.as_name
                    nl.asn = al.asn
                    nl.add(al)
                    nets[net_prefix] = nl
                else:
                    nets[net_prefix].add(al)
        except:
            traceback.print_exc()
            print("Cannot parse: " + line)
            exit(1)

with open(file_frq,"wt") as w:
    w.write(address_file_line.csv_head())
    for ip in frqs:
        w.write(frqs[ip].to_csv())

with open(file_others,"wt") as w:
    w.write(address_file_line.csv_head())
    for ip in others:
        if others[ip].total() >= 1000:
            w.write(others[ip].to_csv())

total_net_load = 0
total_net_load_saved = 0
nb_nets_saved = 0
with open(file_nets,"wt") as w:
    w.write(address_file_line.csv_head())
    for net_prefix in nets:
        total_net_load += nets[net_prefix].total()
        if nets[net_prefix].total() >= 1000:
            w.write(nets[net_prefix].to_csv())
            nb_nets_saved += 1
            total_net_load_saved += nets[net_prefix].total()

print("Found: " + str(len(nets)) + " networks, saved: " + str(nb_nets_saved))
print("Total load: " + str(total_net_load))
print("Total load of saved networks: " + str(total_net_load_saved))
print("Saved load fraction: " + str(100.0*total_net_load_saved/total_net_load) + "%")
print("Load not tracked: " + str(100.0*(1.0-total_net_load_saved/total_net_load)) + "%")
