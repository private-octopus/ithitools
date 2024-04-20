#!/usr/bin/python
# coding=utf-8
#
# This computes the montly totals for all the instances found
# in the east and west folders. The raw data is organized as:
# - ipstats / west / one folder per instance / files per date
#           / east / one folder per instance / files per date
# The first processing step is to collect the list of file names
# for each instance: one file per date, possibly more if the
# same instance is present in east and west.
#
#

import sys
import traceback
import random
import time
import concurrent.futures
import os
from os import listdir
from os.path import isfile, isdir, join

def prepare_instances_list(ipstats_folder, month):
    instances = dict()
    for pole in [ "east", "west" ]:
        pole_dir = join(ipstats_folder, pole)
        folder_pole = listdir(pole_dir)
        for instance_id in folder_pole:
            instance_path = join(pole_dir, instance_id)
            if is_dir(instance_path):
                if not instance_id in instances:
                    instances[instance_id] = []
                file_list = listdir(instance_folder)
                for file_name in file_list:
                    instances[instance_id].append(join(instance_folder, file_name))
    return instances

def check_or_create_dir(dir_path):
    if not isdir(dir_path):
        try:
            os.mkdir(dir_path)
        except Exception as e:
            traceback.print_exc()
            print("Cannot create <" + dir_path  + ">\nException: " + str(e))
            return False
    return True

def process_instance(instance_id, month, result_folder, tmp_folder, ithitool, instances, do_debug):
    result_file = instance-id + "_" + month + "-ipstats.csv"
    result_path = join(result_folder, result_file)
    tmp_file = instance-id + "_" + month + "-file-list.txt"
    tmp_path = join(tmp_folder, tmp_file)
    with open(tmp_path,"wt") as F:
            for file_name in instances[instance_id]:
                F.write(file_name + "\n")
    merge_cmd = ithitool + ' -I ' + result_path + " " + result_path
    cmd_ret = os.system(merge_cmd)
    if cmd_ret == 0:
        if do_debug:
            print(result_file + ": computed.")
    else:
        print(result_file + ": computation failed, error:" + str(cmd_ret))
        return False
    return True

# main
if len(sys.argv) < 4 or len(sys.argv) > 5 or \
    (len(sys.argv) == 5 and sys.argv[4] != "debug"):
    print("Usage: imrs_instances <ipstats_folder> <yyyymm> <ithitool> [debug]")
    print("There are just " + str(len(sys.argv)) + " arguments.")
    exit (1)
ipstats_folder = sys.argv[1]
month = sys.argv[2]
ithitool = sys.argv[3]
do_debug = len(sys.argv) == 5

print("Writing instance monthly files for: " + ipstats_folder)
try:
    instances = prepare_instances_list(ipstats_folder, month)
    result_folder = join(ipstats_folder, "instances")
    tmp_folder = join(ipstats_folder, "tmp")
    if check_or_create_dir(result_folder) and \
       check_or_create_dir(tmp_folder):
        for instance_id in instances:
            if len(instances[instance_id]) > 0:
                if not process_instance(instance_id, month, result_folder, tmp_folder, ithitool, instances, do_debug):
                    exit(1)
except Exception as exc:
   traceback.print_exc()
   print('\nCode generated an exception: %s' % (exc))





