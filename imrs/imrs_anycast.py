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

class ip_clusters:
    def __init__(self, ip):
        self.ip = ip
        self.clusters=dict()

class cluster_clusters:
    def __init__(self, cluster):
        self.cluster = cluster
        self.clusters=dict()

class cluster_item:
    def __init__(self, cluster, volume):
        self.cluster = cluster
        self.volume = volume

def parse_cluster(cluster_folder, cluster_file, ips, min_volume):
    # extract cluster name from file name
    parts = cluster_file.split(".")
    cluster = parts[0]
    # compose path
    file_path = join(cluster_folder, cluster_file)
    # record the clusters in which the address is found
    for line in open(file_path,"r"):
        r = imrs_record()
        if r.parse_imrs(line) and r.query_volume > min_volume:
            if not r.ip in ips:
                ips[r.ip] = ip_clusters(r.ip)
            if not cluster in ips[r.ip].clusters:
                ips[r.ip].clusters[cluster] = 0
            ips[r.ip].clusters[cluster] += r.query_volume

def compute_cross_path(ips, cross_path):
    for ip in ips:
        main_cluster = ""
        v_max = 0
        v_sum = 0
        for cluster in ips[r.ip].clusters:
            v_sum += ips[r.ip].clusters[cluster]
            if ips[r.ip].clusters[cluster] > v_max:
                v_max = ips[r.ip].clusters[cluster]
                main_cluster = cluster
        if not main_cluster in cross_path:
            cross_path[main_cluster] = cluster_clusters[cluster]
        for cluster in ips[r.ip].clusters:
            if not cluster in cross_path[main_cluster].clusters:
                cross_path[main_cluster].clusters[cluster] = 0
            cross_path[main_cluster].clusters[cluster] += ips[r.ip].clusters[cluster]

def cross_path_output(cross_path, output_file):
    with open(output_file, "w") as F:
        cluster_list = list(cross_path.keys()).sorted()
        for cluster in cluster_list:
            c_c = cross_path[cluster]
            c_volume = c_c.clusters[cluster]
            F.write(cluster + ",")
            F.write(str(c_volume)+",")
            v = []
            for cluster2 in c_c.clusters:
                if cluster2 != cluster:
                    v.append(cluster_item(cluster2, c_c.clusters[cluster2]))
                    c_volume += c_c.clusters[cluster2]
            vs = sorted(v,key=lambda cl: cl.volume, reversed=True)
            for vs_c in vs:
                F.write(vs_c.cluster + ",")
                F.write(str(vs_c.volume) + ",")
                F.write(str(100*vs_c.volume/c_volume) + "%,")

# main
if len(sys.argv) != 4:
    print("Usage: py imrs_anycast.py <cluster_folder> <output_file> min_volume")
    exit(1)
cluster_folder = sys.argv[1]
output_file = sys.argv[2]
min_volume = 0
try:
    min_volume = int(sys.argv[3])
except:
    print("Cannot parse number of transactions from: " + sys.argv[3])
    exit(1)

ips = dict()
clusters = listdir(cluster_folder)
for cluster_file in clusters:
    stdout.write(cluster[:6] + ",")
    parse_cluster(cluster_folder, cluster_file, ips, min_volume)
stdout.write(cluster[:6] + "\n")
cross_path = dict()
compute_cross_path(ips, cross_path)
cross_path_output(cross_path, output_file)




