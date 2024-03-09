#!/usr/bin/python
# coding=utf-8
#
# This script organizes the collection of per resolver statistics from
# IMRS traces. The IMRS traces are collected per instance, stored
# in compressed CBOR files such as:
# 
# /data/ITHI/cbor/IMRS/aa01-ar-eza/cbor/20240305-180208_300.cbor.xz
#
# There are several components to these file names:
#
# Storage folder: /data/ITHI/cbor/IMRS/
# Instance name: aa01-ar-eza
# CBOR folder per instance: /cbor/
# Date: 20240305
# Capture ID: -180208_300
# CBOR suffix: .cbor.xz
# 
# The goal is to compute one file per instance and per date, using the "-I" option
# of "ithitools". The files will be located in the result folder such as:
#
# ~/ipstats/results/aa01-ar-eza/20240305.ipstats.csv
#
# Intermediate results will be stored in temporary folders, such as:
#
# ~/ipstats/tmp/aa01-ar-eza/20240305-180208_300.ipstats.csv
#
# The collection jobs will start "near the data" on the octo-east and
# octo-west servers, updating the local result and tmp folders. The
# result files will then be copied on the compute server "compute0"
# using rsynch -- only the ~/ipstats/results/ folder is copied, not
# the tmp folder.
#
# Start from Python job, with parameters:
#  * storage folder
#  * collection folder
#  * selected month (e.g., 202403)
#  * duration of run (default, 12 hours)
# if not yet present, creates the collection folder, the result
#   folder and the temporary folder.
# Collect list of instances (from storage folder)
# In parallel, process instances -- one python thread by instance:
#   if ~/ipstats/results/<instance> not present, create.
#   if ~/ipstats/tmp/<instance> not present, create.
#   list all slices files in /data/ITHI/cbor/IMRS/<instance>/cbor.
#   from list of slices, extract list of dates.
#   trim list of dates:
#        - remove the last date, because it might be incomplete.
#        - remove all dates not in target month.
#   for each date in trimmed list:
#        - if ~/ipstats/<instance>/<date>--ipstats.csv is not present: 
#               extract sublist of slices.
#               for each slice of date:
#                   if ~/ipstats/tmp/<instance>/<slice>.ipstats.csv exists, continue
#                   else:
#                       spawn ithitools task, create csv from slice.
#           add all slice.csv from progress into <date>.csv for instance, using
#           ithitools task.
#       - remove all dated files from ~/ipstats/<instance>/progress/
#
# The script can stop if the specified duration time is elapsed. The time is checked
# in the for loops "for each date" and "for each slice of date".

import sys
import traceback
import random
import time
import concurrent.futures
import os
from os import listdir
from os.path import isfile, isdir, join

def subfolder_list(folder):
    subfolders = [f for f in listdir(folder) if isdir(join(folder, f))]
    return subfolders

def file_list(folder):
    filenames = [f for f in listdir(folder) if isfile(join(folder, f))]
    return filenames

def check_or_create_dir(dir_path):
    if not isdir(dir_path):
        try:
            os.mkdir(dir_path)
        except Exception as e:
            traceback.print_exc()
            print("Cannot create <" + dir_path  + ">\nException: " + str(e))
            return False
    return True

def prepare_instance_list(storage_folder):
    instance_list = []
    if isdir(storage_folder):
        folder_list = listdir(storage_folder)
        instance_list = []
        for folder in folder_list:
            folder_path = join(storage_folder, folder)
            if isdir(folder_path):
                cbor_path = join(folder_path, "cbor")
                if isdir(cbor_path):
                    instance_list.append(folder)
    return sorted(instance_list)

class instance_bucket:
    def __init__(self, instance, storage_folder, result_path, tmp_path, month, cmd, do_debug):
        self.instance = instance
        self.storage_folder = storage_folder
        self.result_path = result_path
        self.tmp_path = tmp_path
        self.month = month
        self.cmd = cmd
        self.result_instance = join(result_path, instance)
        self.tmp_instance = join(tmp_path, instance)
        self.storage_instance = join(storage_folder, instance)
        self.cbor_instance = join(self.storage_instance, "cbor")
        self.slices = []
        self.is_complete = False
        self.end_time = time.time() + 18*60*60
        self.do_debug = do_debug

    def begin_instance(self):
        # verify the result and tmp subfolders
        if not check_or_create_dir(self.result_instance) or not check_or_create_dir(self.tmp_instance):
            print("Could not find or create " + self.result_path + " and " + self.tmp_path)
            return False
        self.slices = file_list(self.cbor_instance)
        return True

    def get_list_of_dates(self):
        date_set = set()
        for slice in self.slices:
            slice_date = slice[0:8]
            date_set.add(slice_date)
        date_list = sorted(list(date_set))
        print(date_list)
        self.date_list = [ d for d in date_list[:-1] if d.startswith(self.month) ]
        if self.do_debug:
            print("Found " + str(len(self.date_list)) + "dates. Retain 1")
            self.date_list = self.date_list[0:1]
        return True

    def process_date(self, d):
        date_result = join(self.result_instance, d + "-ipstats.csv")
        date_tmp = join(self.tmp_instance, d + ".txt")
        print("Computing " + date_result)
        if not isfile(date_result):
            print("Need to compute: " + date_result)
            this_slice = [ s for s in self.slices if s.startswith(d) and s.endswith(".cbor.xz") ]
            try:
                with open(date_tmp, "wt") as F:
                    for s in this_slice:
                        F.write(join(self.cbor_instance, s) + "\n")
                merge_cmp = self.cmd + ' -I ' + date_result + " " + date_tmp
                cmd_ret = os.system(merge_cmp)
                if cmd_ret == 0:
                    print("Computation of " + date_result + " succeeds.")
                else:
                    print("Computation of " + date_result + " failed, error:" + str(cmd-ret))
                    return False
            
            except Exception as exc:
                traceback.print_exc()
                print('\nInstance %s generated an exception: %s' % (self.instance, exc))
                return False
        else:
            print ("Already computed: " + date_result)
        return True

    def load(self):
        try:
            print("Loading " + self.instance)
            if not self.begin_instance():
                print("Begin " + self.instance + "failed.")
                return False
            print("Dates for " + self.instance)
            if not self.get_list_of_dates():
                print("Dates for " + self.instance + "failed.")
                return False
            print ("Found " + str(len(self.slices)) + " slices, " + str(len(self.date_list)) + " dates.")
            for d in self.date_list:
                if time.time() > self.end_time:
                    print("Job has been running too long, stop now")
                    return False
                print("Looking date " + d)
                if not self.process_date(d):
                    return False
            print("Complete.")
            self.is_complete = True
            return True
        except Exception as exc:
                traceback.print_exc()
                print('\nScript generated an exception: %s' %(exc))
                return False

def instance_bucket_load(bucket):
    return bucket.load()
# Main
def main():
    start_time = time.time()

    if len(sys.argv) < 5 or len(sys.argv) > 6 or \
       (len(sys.argv) == 6 and sys.argv[5] != "debug"):
        print("Usage: imrsrsv <storage_folder> <collection_folder> <yyyymm> <ithitool> [debug]")
        print("There are just " + str(len(sys.argv)) + " arguments.")
        exit (1)
    storage_folder = sys.argv[1]
    collection_folder = sys.argv[2]
    month = sys.argv[3]
    ithitool = sys.argv[4]
    do_debug = len(sys.argv) == 6
    result_path = join(collection_folder, "results")
    tmp_path = join(collection_folder, "tmp")

    instance_list = prepare_instance_list(storage_folder)
    if len(instance_list) == 0:
        print("Could not find instances in " + storage_folder)
        exit(1)
        
    if not isdir(collection_folder) or not check_or_create_dir(result_path) or not check_or_create_dir(tmp_path):
        print("Could not find or create " + result_path + " and " + tmp_path)
        exit (1)

    print ("Found " + str(len(instance_list)) + " instances")
    
    nb_process = os.cpu_count()
    bucket_list = []
    s = ""
    for instance in instance_list:
        bucket = instance_bucket(instance, storage_folder, result_path, tmp_path, month, ithitool, do_debug)
        bucket_list.append(bucket)
        s += instance + ", "
        if do_debug:
            break;
    print("Starting to process " + str(len(instance_list)) + " instances:\n" + s)
    
    # process multiple instances in parallel
    with concurrent.futures.ProcessPoolExecutor(max_workers = nb_process) as executor:
        future_to_bucket = {executor.submit(instance_bucket_load, bucket):bucket for bucket in bucket_list }
        for future in concurrent.futures.as_completed(future_to_bucket):
            bucket = future_to_bucket[future]
            try:
                # data = future.result()
                bucket.is_complete = True
                sys.stdout.write(".")
                sys.stdout.flush()
            except Exception as exc:
                traceback.print_exc()
                print('\nBucket %s generated an exception: %s' % (bucket.instance, exc))
        print("\nAll buckets processed.")

# actual main program, can be called by threads, etc.
if __name__ == '__main__':
    main()
