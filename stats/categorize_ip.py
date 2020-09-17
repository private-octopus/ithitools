
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
import ip2as

class criteria:
    def __init__(self, v_name, v_low, v_high, v_max, v_step):
        self.v_name = v_name
        self.v_low = v_low
        self.v_high = v_high
        self.v_max = v_max
        self.v_step = v_step
        self.v_count=[]
        self.total=[]
        for x in range(0,int(v_max/v_step)+1):
            self.v_count.append(0)
            self.total.append(0)

    def record(self,v,t):
        if v > self.v_max:
            x = len(self.v_count) - 1
        else:
            x = int(v/self.v_step)
        self.v_count[x] += 1
        self.total[x] += t

class category:
    def __init__(self, key):
        self.key = key
        self.count = 0
        self.sums = address_file_line("")
        self.samples = []
        self.share = 0.0

    def csv_key_head():
        s = "Key,frequent,log_t,slice,nx,dga,arpa,com,count,share,"
        return s

    def csv_key(self):
        s = self.key + ","
        for x in range(0,7):
            if self.key[x] == "H":
                s += "high,"
            elif self.key[x] == "l":
                s += "low,"
            else:
                s += ","
        s += str(self.count) + ","
        s += str(self.share) + ","
        return s


# Main loop
if len(sys.argv) < 8:
    print("Usage: " + sys.argv[0] + " nb_days <frequent_ip.csv> <as_names.csv> <criterias.csv> <categories.csv> <samples.csv> <frequent_ip_stats.csv> <count_file.csv>* \n")
    exit(1)

nb_days = int(sys.argv[1])
max_slice = nb_days*24*12
frequent_ip_file = sys.argv[2]
as_name_file = sys.argv[3]
file_crit = sys.argv[4]
file_cats = sys.argv[5]
file_samp = sys.argv[6]
file_frq_stats = sys.argv[7]
files_in = sys.argv[8:]

as_dict = ip2as.asname()
as_dict.load(as_name_file)
print("Loaded " + str(len(as_dict.table)) + " AS names.")

fip = frequent_ip.frequent_ip()
fip.load(frequent_ip_file)
print("loaded " + str(len(fip.table)) + " addresses from APNIC frequent list.")
print("largest: " + str(fip.largest) + ", limit_10000: " + str(fip.limit_10000) + ", smallest:" + str(fip.smallest))

crit = []
crit.append(criteria("frq", fip.smallest/2, fip.limit_10000, fip.largest, fip.largest/20))
crit.append(criteria("log_t", 5.0, 7.0, 10.0, 0.5))
crit.append(criteria("slice", 0.25, 0.75, 1.0, 0.05))
crit.append(criteria("nx", 0.2, 0.8, 1.0, 0.05))
crit.append(criteria("dga", 0.2, 0.8, 1.0, 0.05))
crit.append(criteria("arpa", 0.002, 0.01, 0.02, 0.001))
crit.append(criteria("com", 0.01, 0.03, 0.04, 0.002))

cats = dict()

sum_t = 0
sum_n = 0

w_frq = open(file_frq_stats, "wt")
w_frq.write("ip, as, total, dga, nx, count_users, count_users_weighted, validating,\n")
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
                c.append(al.frequent)
                c.append(math.log10(t))
                c.append(al.nb_slices/max_slice)
                if t > 1000000:
                    c.append(al.nx()/t)
                    c.append(al.dga/t)
                    c.append(al.arpa/t)
                    c.append(al.com/t)
                else:
                    for x in range(3,7):
                        c.append(0)

                s = ""

                for x in range(0, len(c)):
                    crit[x].record(c[x],t)
                    if c[x] <= crit[x].v_low:
                        s += 'l'
                    elif c[x] >= crit[x].v_high:
                        s += 'H'
                    else:
                        s += 'm'
                if not s in cats:
                    cat = category(s)
                    cats[s] = cat
                cats[s].count += 1
                if len(cats[s].samples) < 10:
                    cats[s].samples.append(al)
                cats[s].sums.add(al)

                if al.frequent == 1:
                    s = al.ip + "," + str(al.asn) + "," + str(al.total()) + "," + str(al.dga) + "," + str(al.nx_domain) + ","
                    x_fip = fip.table[al.ip]
                    s += str(x_fip.count_users) + ","
                    s += str(x_fip.count_users_weighted) + ","
                    s += str(x_fip.validating) + ",\n"
                    w_frq.write(s)
        except:
            traceback.print_exc()
            print("Cannot parse: " + line)
            exit(1)
w_frq.close()

if sum_t > 0:
    for key in cats:
        cats[key].share = cats[key].sums.total()/sum_t

with open(file_crit,"wt") as w:
    w.write("n,")
    n = 0
    for c in crit:
        w.write(c.v_name + ", nb_" + c.v_name + "/N, q_" + c.v_name + "/T,")
        w.write("nb_" + c.v_name + ", q_" + c.v_name + ",")
        n =max(n,len(c.v_count))

    w.write("\n")
    for x in range(0,n):
        w.write(str(x)+",")
        for c in crit:
            if x < len(c.v_count):
                w.write(str(c.v_step*x) + "," + str(c.v_count[x]/sum_n) + "," + str(c.total[x]/sum_t) + "," + \
                    str(c.v_count[x]) + "," + str(c.total[x]) + ",")
            else:
                w.write(",,,,,")
        w.write("\n")

with open(file_cats,"wt") as w:
    w.write(category.csv_key_head())
    w.write(address_file_line.csv_head())

    for key in cats:
        w.write(cats[key].csv_key())
        w.write(cats[key].sums.to_csv())

with open(file_samp,"wt") as w:
    w.write("key,")
    w.write(address_file_line.csv_head())

    for key in cats:
        for al in cats[key].samples:
            w.write(key + ",")
            w.write(al.to_csv())

