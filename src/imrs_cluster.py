#!/usr/bin/python
# coding=utf-8
#
# This script organizes the grouping of IMRS resolver data per
# cluster. The results are collected in two folders:
# ~/ipstats/west, ~/ipstats/east
# These folders conatins one subfolder per instances, with
# names such as aa01-us-rtv, i.e. <instance>-<country>-<city>.
# The cluster name is the tuple <country-city>.
# The instance folder contain a file per day, e.g.
# ~/ipstats/west/aa01-us-rsv/20240319-ipstats.csv
# The script lists the instances and daily summaries available
# under "east" or "west", computes a list of instance per clusters,
# and compute a list per cluster of instance file names per day.
#

import sys
import traceback
import random
import time
import concurrent.futures
import os
from os import listdir
from os.path import isfile, isdir, join

def collect_cluster_dates(clusters, cluster_id, instance_folder, month, datemax):
    dates = dict()
    if cluster_id in clusters:
        dates = clusters[cluster_id]
    file_list = listdir(instance_folder)
    for file_name in file_list:
        parts = file_name.split("-")
        if len(parts) == 2 and \
            parts[1] == "ipstats.csv" and \
            len(parts[0]) == 8 and \
            parts[0].startswith(month) and \
            parts[0] <= datemax:
            file_date = parts[0]
            file_list = []
            if file_date in dates:
                file_list = dates[file_date]
            file_list.append(join(instance_folder, file_name))
            dates[file_date] = file_list
    clusters[cluster_id] = dates

def prepare_cluster_list(ipstats_folder, month, datemax):
    clusters = dict()
    folder_list = listdir(ipstats_folder)
    for folder in folder_list:
        if folder == "east" or folder == "west":
            folder_path = join(ipstats_folder, folder)
            if isdir(folder_path):
                instance_list = listdir(folder_path)
                for instance in instance_list:
                    parts = instance.split("-")
                    instance_folder = join(folder_path,instance)
                    if not isdir(instance_folder):
                        print("        Not a folder: " + instance_folder)
                    elif \
                        len(parts) == 3 and \
                        len(parts[0]) == 4 and \
                        len(parts[1]) == 2 and \
                        len(parts[2]) == 3:
                        cluster_id = parts[1] + "-" + parts[2]
                        collect_cluster_dates(clusters, cluster_id, instance_folder, month, datemax)
    return clusters

def check_or_create_dir(dir_path):
    if not isdir(dir_path):
        try:
            os.mkdir(dir_path)
        except Exception as e:
            traceback.print_exc()
            print("Cannot create <" + dir_path  + ">\nException: " + str(e))
            return False
    return True

def process_cluster(cluster_id, result_folder, tmp_folder, ithitool, dates, do_debug):
    cluster_folder = join(result_folder, cluster_id)
    dates = clusters[cluster_id]
    if check_or_create_dir(cluster_folder):
        for one_date in dates:
            file_list = dates[one_date]
            ipstats_file_name = one_date + "-ipstats.csv"
            report_name = cluster_id + ipstats_file_name
            ipstats_file = join(cluster_folder, ipstats_file_name)
            if isfile(ipstats_file):
                if do_debug:
                    print(report_name + ": already computed.")
            elif len(file_list) == 0:
                if do_debug:
                    print(report_name + ": no cbor file.")
            elif len(file_list) == 1:
                cp_cmd = "cp " + file_list[0] + " " + ipstats_file
                cp_ret = os.system(cp_cmd)
                if cp_ret == 0:
                    if do_debug:
                        print(report_name + " copied.")
                else:
                    print(report_name + " copy failed, error:" + str(cp_ret))
                    return False
            else:
                tmp_file_name = cluster_id + "-" + one_date + ".txt"
                tmp_file = join(tmp_folder, tmp_file_name)
                with open(tmp_file_name,"wt") as F:
                     for file_name in file_list:
                         F.write(file_name + "\n")
                merge_cmd = ithitool + ' -I ' + ipstats_file + " " + tmp_file_name
                cmd_ret = os.system(merge_cmd)
                if cmd_ret == 0:
                    if do_debug:
                        print(report_name + ": computed.")
                else:
                    print(report_name + ": computation failed, error:" + str(cmd_ret))
                    return False
    return True

# main
if len(sys.argv) < 5 or len(sys.argv) > 6 or \
    (len(sys.argv) == 6 and sys.argv[5] != "debug"):
    print("Usage: imrs_cluster <ipstats_folder> <yyyymm> <last_day> <ithitool> ")
    print("There are just " + str(len(sys.argv)) + " arguments.")
    exit (1)
ipstats_folder = sys.argv[1]
month = sys.argv[2]
datemax = sys.argv[3]
ithitool = sys.argv[4]
do_debug = len(sys.argv) == 6

print("Writing clusters for: " + ipstats_folder)
try:
    clusters = prepare_cluster_list(ipstats_folder, month, datemax)
    result_folder = join(ipstats_folder, "clusters")
    tmp_folder = join(ipstats_folder, "tmp")
    if check_or_create_dir(result_folder) and \
       check_or_create_dir(tmp_folder):
        for cluster_id in clusters:
            dates = clusters[cluster_id]
            if len(dates) > 0:
                if not process_cluster(cluster_id, result_folder, tmp_folder, ithitool, dates, do_debug):
                    exit(1)
except Exception as exc:
   traceback.print_exc()
   print('\nCode generated an exception: %s' % (exc))





