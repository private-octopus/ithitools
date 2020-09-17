
#!/usr/bin/python
# coding=utf-8
#
# This script takes into input a list of addresses that needs to be traced,
# and a list of folders containing traces per five minute slice. It produces
# for each address a csv file with one row per 5-minute slice and
# one column per server-specific folder, showing how much traffic the node
# sent to that server in the specified slice.

import codecs
import sys
import os
from os import listdir
from os.path import isfile, join
from address_file import address_line, address_file_line, source_file, slice_to_time, time_to_slice
import gzip
import concurrent.futures
import traceback
import time
import ipaddress
import ip2as
import datetime

# Trace item: defines the format of the files saved by concurrent processes
# and read when those processes complete.
class trace_item:
    def __init__(self, ip, slice, f_num, total):
        self.ip = ip
        self.slice = slice
        self.f_num = f_num
        self.total = total
    def to_csv(self):
        s = self.ip + "," + self.slice + "," + str(self.f_num) + "," + str(self.total) + ","
        return s
    def from_csv(s):
        x = trace_item("", "", 0, 0)
        p = s.split(",")
        x.ip = p[0].strip()
        x.slice = p[1].strip()
        x.f_num = int(p[2].strip())
        x.total = int(p[3].strip())
        return x


# We need to implement parallel processing because uncompressing 8000 files
# per folder could take way too much time. Since we need to run multiple
# parallel processes, we cannot directly update the "address trace" entries.
# Instead, we build summaries files that get lists of trace items
# (ip, slice, folder_number, total). The processes will write these lists
# in process files, one per bucket
class trace_bucket:
    def __init__(self, trace_pilot):
        self.trace_pilot = trace_pilot
        self.input_files = []
        self.items = []
        self.current_total = 0
        self.current_selected = False
        self.current_ip = ""
        self.bucket_id = 0
        self.file_path = ""
        self.ip_short = "0.0.0.0"

    def add_line(self, line, slice, f_num):
        aline = address_line()
        aline.file_line(line)
        if len(aline.ip) > 0:
            if aline.ip != self.current_ip:
                if self.current_selected:
                    # before replacing the current selection, save the result
                    tr = trace_item(self.ip_short, slice, f_num, self.current_total)
                    self.items.append(tr)
                self.current_ip = aline.ip
                ip_binary = ipaddress.ip_address(aline.ip)
                self.ip_short = ip_binary.compressed
                self.current_total = 0
                self.current_selected = False
                if self.ip_short in self.trace_pilot.ip_list:
                    self.current_selected = True
            if self.current_selected:
                self.current_total += aline.count

    def load(self):
        nb_done = 0
        nb_step = max(int(len(self.input_files)/16), 2000)
        nb_msg = nb_step
        self.current_ip = ""
        try:
            for input_file in self.input_files:
                file_path = join(input_file.folder,input_file.file_name)
                if input_file.file_name.endswith(".gz"):      
                    for line in gzip.open(file_path, 'rt'):
                        self.add_line(line, input_file.slice, input_file.f_num)
                else:    
                    for line in open(file_path):
                        self.add_line(line, input_file.slice, input_file.f_num)
                if self.current_selected and len(self.current_ip) > 0:
                    # Save the last item in the file.
                    tr = trace_item(self.ip_short, input_file.slice, input_file.f_num, self.current_total)
                    self.items.append(tr)
                    self.current_selected = False
                nb_done += 1
                if self.bucket_id == 0 and nb_done >= nb_msg:
                    print("Process 0, processed " + str(nb_done) + " files.")
                    nb_msg += nb_step
        except:
            traceback.print_exc()
            print("Abandon bucket " + str(self.bucket_id))
        return True

    def save(self):
        f_out = open(self.file_path, "wt")
        for ti in self.items:
            f_out.write(ti.to_csv() + "\n")
        f_out.close();

# Address trace: defines the set of traces collected for one specific IP address.
# This produces a file with one lie per time slice, and in that line
# the total count of queries found in each of the tracked files.
class address_trace:
    def __init__(self, tag, ip, nb_folders):
        self.tag = tag
        self.ip = ip
        self.slice_dict = dict()
        self.nb_folders = nb_folders
        self.min_slice = "99999999-9999"
        self.max_slice = "00000000-0000"

    def add_slice(self, slice, f_num, total):
        if not slice in self.slice_dict:
            p = []
            for x in range(0, self.nb_folders):
                p.append(0)
            if f_num >= self.nb_folders:
                print("F_num: " + str(f_num) + ", nb_folders: " + str(self.nb_folders))
                exit(-1)
            self.slice_dict[slice] = p
            self.min_slice = min(self.min_slice, slice)
            self.max_slice = max(self.max_slice, slice)
        self.slice_dict[slice][f_num] = total

    def write_file(self, out_file_name, min_slice, max_slice, f_names):
        with open(out_file_name,"wt") as w_out:
            w_out.write("tag, ip, slice,")
            for f in f_names:
                w_out.write(f + ",")
            w_out.write("\n")
            t_next = slice_to_time(min_slice)
            t_max = slice_to_time(max_slice)
            t_delta =  datetime.timedelta(seconds=300)
            while t_next < t_max:
                slice = time_to_slice(t_next)
                w_out.write(self.tag + "," + self.ip + "," + slice + ",")
                if slice in self.slice_dict:
                    for c in self.slice_dict[slice]:
                        w_out.write(str(c) + ",")
                else:
                    for x in range(0, self.nb_folders):
                        w_out.write(",")
                w_out.write("\n")
                t_next += t_delta

# Trace list:
# Manages an ip_list that has one entry per tracked IP.
# 

class trace_list:
    def __init__(self, nb_folders):
        self.nb_names = 0
        self.nb_folders = nb_folders
        self.ip_list = dict()
        self.f_names = []
        self.min_slice = "99999999-9999"
        self.max_slice = "00000000-0000"

    def init_ip_list(self, ip_file):
        for ip_line in open(ip_file,"rt"):
            parts = ip_line.split(",")
            tag = parts[0].strip()
            dup = parts[1].strip()
            ip = parts[2].strip()
            if tag != "tag":
                at = address_trace(tag + "-" + dup, ip, self.nb_folders)
                self.ip_list[ip] = at

    def add_item_file(self, item_file):
        for item_line in open(item_file,"rt"):
            ti = trace_item.from_csv(item_line)
            if ti.ip in self.ip_list:
                self.ip_list[ti.ip].add_slice(ti.slice, ti.f_num, ti.total)

    def save_lists(self, folder_out):
        for ip in self.ip_list:
            self.min_slice = self.ip_list[ip].min_slice
            self.max_slice = self.ip_list[ip].max_slice
            break
        for ip in self.ip_list:       
            self.min_slice = min(self.min_slice, self.ip_list[ip].min_slice)
            self.max_slice = max(self.max_slice, self.ip_list[ip].max_slice)
        for ip in self.ip_list:
            if len(self.ip_list[ip].slice_dict) > 0:
                ip_text = ip.replace(":","-")
                fname = "slices_" + ip_text + "_" + self.min_slice[0:4] + "-" + self.min_slice[4:6] + ".csv"
                fpath = join(folder_out,fname);
                self.ip_list[ip].write_file(fpath, self.min_slice, self.max_slice, self.f_names)
                print("Saved:" + fpath)
            else:
                print("Nothing to store for " + ip)


def load_trace_bucket(bucket):
    bucket.load()
    bucket.save()

# Main loop

def main():
    if len(sys.argv) < 5:
        print("Usage: " + sys.argv[0] + " <result-folder> <temp_prefix> <selected-ip.csv> <input-folder>* \n")
        exit(1)

    result_folder = sys.argv[1]
    temp_prefix = sys.argv[2]
    selected_ip = sys.argv[3]
    input_paths = sys.argv[4:]

    tl = trace_list(len(input_paths))
    tl.f_names = input_paths # TODO: extract location-ID, e.g. US-LAX

    tl.init_ip_list(selected_ip)
    print("Will track " + str(len(tl.ip_list)) + " IP, " + str(len(input_paths)) + " paths.") 

    input_files = []
    f_num = 0 
    for p in input_paths:
        for f in listdir(p):
            if isfile(join(p, f)):
                sf = source_file(p, f, f_num)
                input_files.append(sf)
        print("Path " + p + " rank " + str(f_num))
        f_num += 1
    input_files = sorted(input_files)

    nb_process = os.cpu_count()
    print("Aiming for " + str(nb_process) + " processes")
    process_left = nb_process
    # When debugging, setting the nb of processes to 1 ensures single threaded execution.
    # process_left = 1
    # nb_process = 1

    bucket_list = []
    bucket_first = 0
    bucket_id = 0
    while bucket_first < len(input_files):
        bucket = trace_bucket(tl)
        step = int((len(input_files) - bucket_first + process_left - 1)/process_left)
        process_left -= 1
        bucket_next = min(bucket_first+step, len(input_files))
        bucket.input_files = input_files[bucket_first:bucket_next]
        last_slice = bucket.input_files[len(bucket.input_files)-1].slice
        while bucket_next < len(input_files) and input_files[bucket_next].slice == last_slice:
            bucket.input_files.append(input_files[bucket_next])
            bucket_next += 1
        bucket.bucket_id = bucket_id
        bucket.file_path = temp_prefix + "_" + str(bucket.bucket_id) + ".csv"
        bucket_list.append(bucket)
        bucket_id += 1
        bucket_first = bucket_next

    nb_process = min(nb_process, len(bucket_list))
    print("Will use " + str(nb_process) + " processes, " + str(len(bucket_list)) + " buckets")
    total_files = 0
    for bucket in bucket_list:
        total_files += len(bucket.input_files)
    print("%d files in %d buckets (%d .. %d), vs %d" %(total_files, len(bucket_list), len(bucket_list[0].input_files), len(bucket_list[len(bucket_list)-1].input_files), len(input_files)))


    start_time = time.time()
    if len(bucket_list) > 1:
        with concurrent.futures.ProcessPoolExecutor(max_workers = nb_process) as executor:
            future_to_bucket = {executor.submit(load_trace_bucket, bucket):bucket for bucket in bucket_list }
            for future in concurrent.futures.as_completed(future_to_bucket):
                bucket = future_to_bucket[future]
                try:
                    data = future.result()
                except Exception as exc:
                    print('Bucket %d generated an exception: %s' % (bucket.bucket_id, exc))
    else:
        load_trace_bucket(bucket_list[0])
        print("Loaded a single bucket")

    bucket_time = time.time()
    print("Threads took " + str(bucket_time - start_time))

    for bucket in bucket_list:
        # load the data for the bucket
        try:
            tl.add_item_file(bucket.file_path)
        except:
            traceback.print_exc()
            print("Abandon bucket " + str(bucket.bucket_id))
    summary_time = time.time()
    print("Summary took " + str(summary_time - bucket_time))

    tl.save_lists(result_folder)
    
    end_time = time.time()
    print("Complete in " + str(end_time - start_time))

if __name__ == '__main__':
    #freeze_support()
    main()