#!/usr/bin/python
# coding=utf-8
#
# This script analyzes how traffic is spread by routing protocols.
# For each IP address, we compute the fraction of traffic going to a specific
# location (cluster), as already compute in the clusters "monthly" files.
# Once we have computed all the data for an IP address, we make the hypothesis
# that there is a dominant cluster, and then some "diverted" traffic
# routed to other locations. We can then sum that data per dominant cluster:
#   - total amount of traffic
#   - amount routed to other places.
# We can visualize that as a graph, showing the effects of anycast routing,
# but we can also express that as a csv file, with N lines and N columns,
# allowing for further visualization.
#
# Usage: py imrs_anycast.py <cluster_folder> <output_file>
import sys
import traceback
import os
from os import listdir
from os.path import isfile, isdir, join
import imrs
from imrs import imrs_record

# main
if len(sys.argv) != 2:
    print("Usage: py imrs_test.py ipstats_file")
    exit(1)
imrs_file = sys.argv[1]
n_apnic = 0
t_queries = 0
t_com = 0
t_apnic = 0
max_queries = 0
max_com = 0
max_apnic = 0
min_queries = 1000000000
min_com = 1000000000
min_apnic = 1000000000
n_lines = 0
q_l = 0
c_l = 0
a_l = 0
n_l = 0

for line in open(imrs_file, "r"):
    rec = imrs_record()
    if not rec.parse_imrs(line):
        break
    if rec.apnic_count > 0:
        if n_apnic == 0:
            print("line: " + str(n_lines))
            print(line)
            n_l = n_lines
            q_l = rec.query_volume
            c_l = rec.tld_counts[0]
            a_l = rec.apnic_count
        n_apnic += 1
        t_queries += rec.query_volume
        t_com += rec.tld_counts[0]
        t_apnic += rec.apnic_count
        if rec.query_volume > max_queries:
            max_queries = rec.query_volume
        if rec.tld_counts[0] > max_com:
            max_com = rec.tld_counts[0]
        if rec.apnic_count > max_apnic:
            max_apnic = rec.apnic_count
        if rec.query_volume < min_queries:
            min_queries = rec.query_volume
        if rec.tld_counts[0] < min_com:
            min_com = rec.tld_counts[0]
        if rec.apnic_count < min_apnic:
            min_apnic = rec.apnic_count
    n_lines += 1
print("Samples: " + str(n_apnic) + " / " + str(n_lines))
print("queries: " + str(min_queries) + " < " + str(t_queries/n_apnic) + " < " + str(max_queries))
print("com:     " + str(min_com) + " < " + str(t_com/n_apnic) + " < " + str(max_com))
print("apnic:   " + str(min_apnic) + " < " + str(t_apnic/n_apnic) + " < " + str(max_apnic))
print("Line[" + str(n_l) + "]: queries: " + str(q_l) + ", com:" + str(c_l) + ", apnic:" + str(a_l))







