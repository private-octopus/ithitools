#!/usr/bin/python
# coding=utf-8
#
# This processes a list of monthly files (instances or clusters)
# and extracts a table, with entries:
# instance, nb_ip, nb_queries
# or
# cluster, instances, nb_ip, nb_queries
# where "nb_ip" is the number of IP addresses seens by this cluster,
# and "nb_queries" is the total number of queries
#
# usage: imrs_montly_ip input_folder output_file

import sys
import traceback
import random
import time
import concurrent.futures
import os
from os import listdir
from os.path import isfile, isdir, join

def parse_imrs(line):
    ok = False
    ip = ""
    count = 0
    try:
        parts = line.split(",")
        ip = parts[0].strip()
        count = int(parts[1].strip())
        ok = True
    except Exception as e:
        traceback.print_exc()
        print("Cannot parse IMRS Record:\n" + line.strip()  + "\nException: " + str(e))
    return ok, ip, count

# main
if len(sys.argv) != 3:
    print("usage: imrs_montly_ip input_folder output_file")
    exit(1)
input_folder = sys.argv[1]
output_file = sys.argv[2]
is_instances = not input_folder[:-1].endswith("monthly")
if is_instances:
    print("From cluster monthly, " + input_folder + " compute " + output_file)
else:
    print("From instances monthly, " + input_folder + " compute " + output_file)
clusters = dict()
if is_instances:
    file_list = listdir(input_folder)
    for file_name in file_list:
        parts = file_name.split("_")
        first_parts = parts[0].split('-')
        cluster_id = parts[1] + "-" + parts[2]
        if not cluster_id in clusters:
            clusters[cluster_id] = []
        clusters[cluster_id].append(file_name)
else:
    file_list = listdir(input_folder)
    for file_name in file_list:
        parts = file_name.split(".")
        cluster_id = parts[0]
        if not cluster_id in clusters:
            clusters[cluster_id] = []
        clusters[cluster_id].append(file_name)

id_list = sorted(list(clusters.keys()))

with open(output_file, "w") as F:
    F.write("Cluster, Instance, nb_IP, nb_queries,\n")
    for cluster_id in id_list:
        total_ip = 0
        total_queries = 0
        for file_name in clusters[cluster_id]:
            file_path = join(input_folder, file_name)
            nb_ip = 0
            nb_queries = 0
            for line in file_path:
                ok,ip,count = parse_imrs(line)
                if ok:
                    nb_ip += 1
                    nb_queries += count
            if is_instances:
                file_parts = file_name.split("_")
                instance_id = parts[0]
                F.write(cluster_id + "," + instance_id + "," + str(nb_ip) + "," + str(nb_queries))
            total_ip += nb_ip
            total_queries += nb_queries
        F.write(cluster_id + "," + total + "," + str(total_ip) + "," + str(total_queries))
