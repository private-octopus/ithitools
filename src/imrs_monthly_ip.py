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
        if len(parts) >= 2:
            ip = parts[0].strip()
            count = int(parts[1].strip())
            ok = True
        else:
            print("Line <" + line.strip() + " has only " + str(len(parts)) + " parts.")
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
    print("From instances monthly, " + input_folder + " compute " + output_file)
else:
    print("From cluster monthly, " + input_folder + " compute " + output_file)
clusters = dict()
nb_files = 0
if is_instances:
    file_list = listdir(input_folder)
    for file_name in file_list:
        parts = file_name.split("_")
        print(file_name + " -> " + parts[0])
        first_parts = parts[0].split('-')
        if len(first_parts) != 3 or \
           len(first_parts[0]) != 4 or \
           len(first_parts[1]) != 2 or \
           len(first_parts[2]) != 3:
            print("Cannot get cluster ID from: " + file_name)
        else:
            cluster_id = first_parts[1] + "-" + first_parts[2]
            if not cluster_id in clusters:
                clusters[cluster_id] = []
            clusters[cluster_id].append(file_name)
            nb_files += 1
else:
    file_list = listdir(input_folder)
    for file_name in file_list:
        parts = file_name.split(".")
        cluster_id = parts[0]
        if not cluster_id in clusters:
            clusters[cluster_id] = []
        clusters[cluster_id].append(file_name)
        nb_files +=1

print("Found " + str(len(clusters)) + " clusters, " + str(nb_files) + " files.")

id_list = sorted(list(clusters.keys()))

with open(output_file, "w") as F:
    F.write("Cluster, Instance, nb_IP, nb_queries,\n")
    for cluster_id in id_list:
        sys.stdout.write(cluster_id)
        total_ip = 0
        total_queries = 0
        for file_name in clusters[cluster_id]:
            sys.stdout.write(".")
            sys.stdout.flush()
            file_path = join(input_folder, file_name)
            ip_list = set()
            nb_queries = 0
            for line in open(file_path, "r"):
                ok,ip,count = parse_imrs(line)
                if ok:
                    nb_ip += 1
                    if not ip in ip_list:
                        ip_list.add(ip)
                    nb_queries += count
            if is_instances:
                file_parts = file_name.split("_")
                instance_id = parts[0]
                F.write(cluster_id + "," + instance_id + "," + str(nb_ip) + "," + str(nb_queries) + ",\n")
            total_queries += nb_queries
        total_ip = len(ip_list)
        F.write(cluster_id + ",total," + str(total_ip) + "," + str(total_queries) + ",\n")
    print("\nAll done.")
