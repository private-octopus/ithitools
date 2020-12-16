
#!/usr/bin/python
# coding=utf-8
#
# This script takes as input a summary of IP address file produced
# by "cout_ip.py", and produces a summary of the results.
#
# There are four metrics defined in the M9.3 set:
#
# M9.1 - IMRS Query Load (Number of queries per month at all IMRS servers)
# M9.2 - IMRS traffic by Resolver category (using APNIC data) 
# M9.3 - TOP APNIC, number of COM queries per month per address (or subnet?)
# M9.4 - TOP APNIC, NX volume per month per address (or subnet?)
#
# The Query load is self explanatory. Just obtain the sum of all queries listed in all the files.
# The traffic by resolver category suppose first summing the resolver data per IP address across
# all the files, and then doing separation in the following set:
#
# - APNIC-Top
# - APNIC-Others
# - NON-APNIC Positive (fewer than 10% NX Domain)
# - NON-APNIC Tiny (fewer than 1000 queries per month)
# - NON-APNIC Others (not positive, not tiny)
#
# For each category, we compute the total number of resolvers, and the total load.
#
# The metric M9.3 and M9.4 are only computed on the APNIC-Top subset, because they assume resolvers
# with a volume of user generated queries.
#
# M9.3 tabulates resolvers by amount of queries per month to .COM. Resolvers are categorized in buckets,
# based on the number of queries sent to .COM in a month. A resolver with perfect caching would send 
# just 30 such queries, or even fewer if the load was spread across several root servers. The buckets
# are defined as powers of 10: <10, <100,..<100,000,000, >100,000,000. For each bucket we compute the
# number of resolvers, total load, and total APNIC user count. This provides an estimate of caching efficiencies.
#
# M9.4 tabulates resolvers by fraction of NX queries per month. Resolvers are categorized in
# 20 buckets: 0 to 5%, .. 90 to 95%, over 95%. This provides an estimated of NX domain
# caching efficiency. For each bucket we compute the
# number of resolvers, total load, and total APNIC user count. This provides an estimate of caching efficiencies.
#
# Program produces the M9.csv file for the month, using same conventions as Ithitools.

import codecs
import sys
from address_file import address_line, address_file_line
import traceback
import ipaddress
import ip2as
import datetime
import math
from os import listdir
from os.path import isfile, join
import frequent_ip
from ip_summary import subnet_string

class collect_m92:
    def __init__(self, apnic_top_limit, tiny_limit, positive_min, nx_ratio):
        self.apnic_top_limit = apnic_top_limit
        self.positive_min = positive_min
        self.tiny_limit = tiny_limit
        self.nx_ratio = nx_ratio
        self.class_names = [ "apnic-top", "apnic-others", "non-apnic-tiny", "non-apnic-positive", "non-apnic-others"]
        self.resolver_count = []
        self.load = []
        for i in range(len(self.class_names)):
            self.resolver_count.append(0)
            self.load.append(0)

    def add_resolver(self, queries, users, nx_queries):
        i_cat = 0
        if users >= self.apnic_top_limit:
            i_cat = 0
        elif users > 0:
            i_cat = 1
        elif queries < self.tiny_limit:
            i_cat = 2
        elif queries >= self.positive_min and queries*self.nx_ratio > nx_queries:
            i_cat = 3
        else:
            i_cat = 4
        self.resolver_count[i_cat] += 1
        self.load[i_cat] += queries
        return i_cat


class collect_m93:
    def __init__(self):
        self.limits = []
        self.user_count = []
        self.resolver_count = []
        self.load = []
        limit = 10
        while (limit < 1000000000):
            self.limits.append(limit)
            self.user_count.append(0)
            self.resolver_count.append(0)
            self.load.append(0)
            limit *= 10

    def add_resolver(self, queries, users):
        i = 0
        while i < len(self.limits) - 1:
            if queries < self.limits[i]:
                break;
            i += 1
        self.user_count[i] += users
        self.resolver_count[i] += 1
        self.load[i] += queries

class collect_m94:
    def __init__(self):
        self.step = 0.05
        self.user_count = []
        self.resolver_count = []
        self.load = []
        self.step_max = []
        step_max = 0
        while (step_max < 1.0):
            step_max += self.step
            self.step_max.append(step_max)
            self.user_count.append(0)
            self.resolver_count.append(0)
            self.load.append(0)

    def add_resolver(self, queries, nx_queries, users):
        if queries > 0:
            nx_ratio = float(nx_queries)/float(queries)
            i = min(int(nx_ratio/self.step), len(self.load)-1)
            self.user_count[i] += users
            self.resolver_count[i] += 1
            self.load[i] += queries

# Main loop
if len(sys.argv) != 5:
    print("Usage: " + sys.argv[0] + "<date> <m9_file.csv> <frequent_ip.csv> <count_folder.csv> \n")
    exit(1)

m9_date = sys.argv[1]
m9_file = sys.argv[2]
frequent_ip_file = sys.argv[3]
folder_in = sys.argv[4]

fip = frequent_ip.frequent_ip()
fip.load(frequent_ip_file)
print("loaded " + str(len(fip.table)) + " addresses from APNIC frequent list.")
print("largest: " + str(fip.largest) + ", limit_10000: " + str(fip.limit_10000) + ", smallest:" + str(fip.smallest))

all_addresses = dict()

sum_t = 0
sum_n = 0

for f in listdir(folder_in):
    pf = join(folder_in, f)
    if isfile(pf):
        for line in open(pf, "rt"):
            try:
                al = address_file_line("")
                al.from_csv(line)
                if len(al.ip) > 0:
                    t = al.total()
                    sum_t += t
                    sum_n += 1
                    al.frequent = 0.0                  
                    al.users = 0
                    if al.ip in all_addresses:
                        all_addresses[al.ip].add(al)
                    else:
                        all_addresses[al.ip] = al
                    if al.ip in fip.table:
                        # restore the user counts.
                        all_addresses[al.ip].frequent = fip.table[al.ip].count_users_weighted                   
                        all_addresses[al.ip].users = fip.table[al.ip].count_users 
                    else:
                        all_addresses[al.ip].frequent = 0                   
                        all_addresses[al.ip].users = 0                 
                    all_addresses[al.ip].nb_addresses = 1 
            except:
                traceback.print_exc()
                print("Cannot parse: " + line)
                exit(1)

# At this point, we have a list of all addresses in the set of files.
# We can compute the metrics M9 using a set of helpers
print("Found " + str(len(all_addresses)) + " in " + str(sum_n) + " lines for " + str(sum_t) + " queries.");
m91 = 0
m92 = collect_m92(fip.limit_10000, 1000, 100000, 0.1)
m93 = collect_m93()
m94 = collect_m94()

for ip in all_addresses:
    nb_queries = all_addresses[ip].total()
    nb_nx = all_addresses[ip].nx()
    m91 += nb_queries
    i_cat = m92.add_resolver(nb_queries, all_addresses[ip].frequent, nb_nx)
    if i_cat == 0:
        m93.add_resolver(nb_queries, all_addresses[ip].frequent)
        m94.add_resolver(nb_queries, nb_nx, all_addresses[ip].frequent)
      
# Now, we compose the m9 output file 
with open(m9_file,"wt") as w:
    w.write("M9.1," + m9_date + ",v2.00, ," + str(m91) + "\n")
    for i in range(len(m92.class_names)):
        w.write("M9.2.1," + m9_date + ",v2.00,\"" + m92.class_names[i] + "\"," + str(m92.load[i]) + "\n")
    for i in range(len(m92.class_names)):
        w.write("M9.2.2," + m9_date + ",v2.00,\"" + m92.class_names[i] + "\"," + str(m92.resolver_count[i]) + "\n")
    for i in range(len(m93.limits)):
        w.write("M9.3.1," + m9_date + ",v2.00," + m93.limits[i] + "," + str(m93.load[i]) + "\n")
    for i in range(len(m93.limits)):
        w.write("M9.3.2," + m9_date + ",v2.00," + m93.limits[i] + "," + str(m93.resolver_count[i]) + "\n")
    for i in range(len(m93.limits)):
        w.write("M9.3.3," + m9_date + ",v2.00," + m93.limits[i] + "," + str(m93.user_count[i]) + "\n")
    for i in range(len(m94.step_max)):
        w.write("M9.4.1," + m9_date + ",v2.00," + str(m94.step_max[i]) + "," + str(m94.load[i]) + "\n")
    for i in range(len(m94.step_max)):
        w.write("M9.4.2," + m9_date + ",v2.00," + str(m94.step_max[i]) + "," + str(m94.resolver_count[i]) + "\n")
    for i in range(len(m94.step_max)):
        w.write("M9.4.3," + m9_date + ",v2.00," + str(m94.step_max[i]) + "," + str(m94.user_count[i]) + "\n")

print("Found " + str(len(all_addresses)) + " in " + str(sum_n) + " lines for " + str(sum_t) + " queries.");

