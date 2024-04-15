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
    monthly_folder = join(ipstats_folder, "monthly")
    tmp_folder = join(ipstats_folder, "tmp")
    monthly_list = listdir(monthly_folder)
    if check_or_create_dir(monthly_folder) and \
       check_or_create_dir(tmp_folder):
        tmp_file_name = join(tmp_folder, month + ".txt")
        with open(tmp_file_name, "wt") as F:
            # check that this is a cluster, and not some other file
            # Watch for: cluster_id + "." + month + "-" + "ipstats.csv"
            monthly_file_end = month + "-" + "ipstats.csv"
            for monthly_file in monthly_list:
                monthly_path = join(monthly_folder, monthly_file)
                if len(monthly_file) > 7 and \
                    monthly_file[2] == "-" and \
                    monthly_file[6] == "." and \
                    monthly_file.endswith(monthly_file_end):
                    F.write(monthly_path +"\n")
                    if do_debug:
                        print("Adding: " + monthly_file)
                elif do_debug:
                    print("Not a monthly file: " + monthly_path)
        total_file = "total-" + month +  "-" + "ipstats.csv"
        total_path = join(ipstats_folder, total_file)
        merge_cmd = ithitool + ' -I ' + total_path + " " + tmp_file_name
        if do_debug:
            print("Running: " + merge_cmd)
            sys.stdout.flush()
        cmd_ret = os.system(merge_cmd)
        if cmd_ret == 0:
            if do_debug:
                print(total_file + ": computed.")
        else:
            print(report_name + ": computation failed, error:" + str(cmd_ret))
except Exception as exc:
   traceback.print_exc()
   print('\nCode generated an exception: %s' % (exc))





