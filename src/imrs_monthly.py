#!/usr/bin/python
# coding=utf-8
#
# This script organizes the sum of IMRS resolver data per
# cluster for the month. The results are collected in
# the folder  ~/ipstats/cluster, with one subfolder
# per cluster, using names like ~/ipstats/cluster/us-lax/.
# For each cluster, the script compute a single file
# such as ~/ipstats/cluster/us-lax.202403.csv, containing
# the aggregated statistics for the whole month.
#

import sys
import traceback
import random
import time
import concurrent.futures
import os
from os import listdir
from os.path import isfile, isdir, join

def check_or_create_dir(dir_path):
    if not isdir(dir_path):
        try:
            os.mkdir(dir_path)
        except Exception as e:
            traceback.print_exc()
            print("Cannot create <" + dir_path  + ">\nException: " + str(e))
            return False
    return True

# main
if len(sys.argv) < 4 or len(sys.argv) > 5 or \
    (len(sys.argv) == 5 and sys.argv[4] != "debug"):
    print("Usage: imrs_monthly <ipstats_folder> <yyyymm> <ithitool> [\"debug\"]")
    print("There are just " + str(len(sys.argv)) + " arguments.")
    exit (1)
ipstats_folder = sys.argv[1]
month = sys.argv[2]
ithitool = sys.argv[3]
do_debug = len(sys.argv) == 5

print("Writing monthly per custom clusters aggregates for: " + ipstats_folder)
try:
    # Look at every cluster under the "clusters" folder
    clusters_folder = join(ipstats_folder, "clusters")
    monthly_folder = join(ipstats_folder, "monthly")
    tmp_folder = join(ipstats_folder, "tmp")
    cluster_list = listdir(clusters_folder)
    if check_or_create_dir(monthly_folder) and \
       check_or_create_dir(tmp_folder):
        for cluster_id in cluster_list:
            # check that this is a cluster, and not some other file
            this_cluster_dir = join(clusters_folder, cluster_id)
            cluster_parts = cluster_id.split("-")
            if len(cluster_parts) != 2 or \
                len(cluster_parts[0]) != 2 or \
                len(cluster_parts[1]) != 3:
                print("*** Unexpected cluster name: " + this_cluster_dir)
            elif not isdir(this_cluster_dir):
                print("*** Not a cluster folder: " + this_cluster_dir)
            else:
                tmp_file_name = join(tmp_folder, cluster_id + '.' + month + ".txt")
                nb_files = 0
                with open(tmp_file_name, "wt") as F:
                    result_list = listdir(this_cluster_dir)
                    for result_file in result_list:
                        result_path = join(this_cluster_dir, result_file)
                        if not isfile(result_path) or \
                            not result_file.startswith(month) or \
                            not result_file.endswith("ipstats.csv"):
                            print("*** Unexpected file: " + result_file)
                        else:
                            nb_files += 1
                            F.write(result_path+"\n")
                print(cluster_id + ", " + str(nb_files))
                if nb_files > 0:
                    ipstats_file = cluster_id + "." + month + "-" + "ipstats.csv"
                    ipstats_path = join(monthly_folder, ipstats_file)
                    merge_cmd = ithitool + ' -I ' + ipstats_path + " " + tmp_file_name
                    cmd_ret = os.system(merge_cmd)
                    if cmd_ret == 0:
                        if do_debug:
                            print(report_name + ": computed.")
                    else:
                        print(report_name + ": computation failed, error:" + str(cmd_ret))
except Exception as exc:
   traceback.print_exc()
   print('\nCode generated an exception: %s' % (exc))





