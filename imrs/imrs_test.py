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

for line in open(imrs_file, "r"):
    rec = imrs_record()
    if not rec.parse_imrs(line):
        break




