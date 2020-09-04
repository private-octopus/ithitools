
#!/usr/bin/python
# coding=utf-8
#
# This script takes as input a list of "address" files and provides a summary
# with one line per address in the file. The it computes a simple summary 
# with one line per address.
#
# TODO: deal with clusters. The recording of the cluster nodes should be logically
# merged, so that a given address is not counted twice if present on two cluster
# nodes. This means sorting by <location><time><cluster-id>, and spreading all
# files for the same time and location to the same parallel processor. It also
# means counting "minute slices" instead of files.

import codecs
import sys
import os
from os import listdir
from os.path import isfile, join
from address_file import address_line, address_file_line
import gzip
import concurrent.futures
import traceback
import time
import ipaddress
import ip2as
import datetime

class source_file:
    def __init__(self, folder, file_name):
        self.folder = folder
        self.file_name = file_name
        self.slice = "00000000-000000"
        self.set_time_slice()

    def set_time_slice(self):
        try:
            # expect names in the form 20190601-000853_300.cbor.xz-results-addr.csv
            dot_slice = self.file_name.split(".")
            under_slice = dot_slice[0].split("_")
            time_slice = under_slice[0].split("-")
            duration = int(under_slice[1])
            year = int(time_slice[0][0:4])
            month = int(time_slice[0][4:6])
            day = int(time_slice[0][6:])
            hour = int(time_slice[1][0:2])
            minute = int(time_slice[1][2:4])
            second = int(time_slice[1][4:])
            t = datetime.datetime(year, month, day, hour, minute, second)
            # set time to the middle of the duration interval
            dt = datetime.timedelta(seconds=(duration/2))
            t += dt
            # set minute to the start of the 5 minute interval before the middle
            t = t.replace(minute = 5*int(t.minute / 5), second=0)
            # get the text value of the time
            self.slice = t.strftime("%Y%m%d-%H%M%S")
        except:
            traceback.print_exc()
            print("Error, file: " + self.file_name)

    def compare(self, other):
        if (self.slice < other.slice):
            return -1
        elif (self.slice > other.slice):
            return 1
        elif (self.folder < other.folder):
            return -1
        elif (self.folder > other.folder):
            return 1
        elif (self.file_name < other.file_name):
            return -1
        elif (self.file_name > other.file_name):
            return 1
        else:
            return 0
    
    def __lt__(self, other):
        return self.compare(other) < 0
    def __gt__(self, other):
        return self.compare(other) > 0
    def __eq__(self, other):
        return self.compare(other) == 0
    def __le__(self, other):
        return self.compare(other) <= 0
    def __ge__(self, other):
        return self.compare(other) >= 0
    def __ne__(self, other):
        return self.compare(other) != 0


class file_bucket:
    def __init__(self):
        self.ip_dict = dict()
        self.input_files = []
        self.total_count = 0
        self.previous_ip = ""
        self.bucket_id = 0
        self.file_path = ""
        self.ip_short = "0.0.0.0"

    def add_line(self, line, slice):
        aline = address_line()
        aline.file_line(line)  
        if len(aline.ip) > 0:
            if aline.ip != self.previous_ip:
                self.previous_ip = aline.ip
                ip_binary = ipaddress.ip_address(aline.ip)
                self.ip_short = ip_binary.compressed
                aline.ip = self.ip_short
                if not self.ip_short in self.ip_dict:
                    self.ip_dict[self.ip_short] = address_file_line(aline.ip)
                self.ip_dict[self.ip_short].add_slice(slice)
            self.ip_dict[self.ip_short].update(aline)
            self.total_count += aline.count

    def load(self):
        nb_done = 0
        nb_step = max(int(len(self.input_files)/16), 2000)
        nb_msg = nb_step
        try:
            for input_file in self.input_files:
                self.previous_ip = ""
                file_path = join(input_file.folder,input_file.file_name)
                if input_file.file_name.endswith(".gz"):      
                    for line in gzip.open(file_path, 'rt'):
                        self.add_line(line, input_file.slice)
                else:    
                    for line in open(file_path):
                        self.add_line(line, input_file.slice)
                nb_done += 1
                if self.bucket_id == 0 and nb_done >= nb_msg:
                    print("Bucket 0, processed " + str(nb_done) + " files.")
                    nb_msg += nb_step
        except:
            traceback.print_exc()
            print("Abandon bucket " + str(self.bucket_id))
        return True

    def save(self):
        f_out = open(self.file_path, "wt")
        f_out.write(address_file_line.csv_head())
        for ip_address in self.ip_dict:
            f_out.write(self.ip_dict[ip_address].to_csv())
        f_out.close();
        print("Process " + str(self.bucket_id) + ": " + str(len(self.ip_dict)) + " IP addresses, " + str(self.total_count) + " transactions.")



def load_bucket(bucket):
    bucket.load()
    bucket.save()

# Main loop

def main():
    if len(sys.argv) < 7:
        print("Usage: " + sys.argv[0] + "<count_file.csv> <temp_prefix> <frequent-ip.csv> <ip2as.csv> <ip2asv6.csv> <input-folder>* \n")
        exit(1)

    count_file = sys.argv[1]
    temp_prefix = sys.argv[2]
    frequent_ip = sys.argv[3]
    ip2as_in = sys.argv[4]
    ip2asv6_in = sys.argv[5]
    input_paths = sys.argv[6:]

    input_files = []
    for p in input_paths:
        for f in listdir(p):
            if isfile(join(p, f)):
                sf = source_file(p, f)
                input_files.append(sf)
    input_files = sorted(input_files)

    nb_process = os.cpu_count()
    print("Aiming for " + str(nb_process) + " processes")
    process_left = nb_process

    bucket_list = []
    bucket_first = 0
    bucket_id = 0
    while bucket_first < len(input_files):
        bucket = file_bucket()
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
    with concurrent.futures.ProcessPoolExecutor(max_workers = nb_process) as executor:
        future_to_bucket = {executor.submit(load_bucket, bucket):bucket for bucket in bucket_list }
        for future in concurrent.futures.as_completed(future_to_bucket):
            bucket = future_to_bucket[future]
            try:
                data = future.result()
            except Exception as exc:
                print('Bucket %d generated an exception: %s' % (bucket.bucket_id, exc))

    bucket_time = time.time()

    ip_dict = dict()
    total_count = 0
    for bucket in bucket_list:
        # load the data for the bucket
        try:
            for line in open(bucket.file_path):
                file_line = address_file_line("")
                file_line.from_csv(line)
                if file_line.ip != "":
                    if file_line.ip in ip_dict:
                        # merge the two lines
                        ip_dict[file_line.ip].add(file_line)
                    else:
                        ip_dict[file_line.ip] = file_line
                    total_count += file_line.nx_domain + file_line.arpa + file_line.tld
        except:
            traceback.print_exc()
            print("Abandon bucket " + str(bucket.bucket_id))
        print("After bucket %d, %d IP, %d transactions"%(bucket.bucket_id, len(ip_dict), total_count))
    summary_time = time.time()
    print("Threads took " + str(bucket_time - start_time))
    print("Summary took " + str(summary_time - bucket_time))

    # document which files are "frequent"
    for line in open(frequent_ip):
        parts = line.split(",")
        try:
            ip_address = ipaddress.ip_address(parts[0].strip())
            ip_text = str(ip_address)
            if ip_text in ip_dict:
                ip_dict[ip_text].frequent = 1
        except:
            pass
    
    frequent_time = time.time()
    print("Frequent IP took " + str(frequent_time - summary_time))

    # document the AS number
    ipv4table = ip2as.ip2as_table()
    ipv4table.load(ip2as_in)
    ipv6table = ip2as.ip2as_table()
    ipv6table.load(ip2asv6_in)

    for ip in ip_dict:
        if ":" in ip:
            ip_dict[ip].asn = ipv6table.get_asn(ip)
        else:
            ip_dict[ip].asn = ipv4table.get_asn(ip)

    as_time = time.time()
    print("AS lookup took " + str(as_time - frequent_time))


    f_out = open(count_file, "wt")
    f_out.write(address_file_line.csv_head())
    for ip_address in ip_dict:
        f_out.write(ip_dict[ip_address].to_csv())
    f_out.close();
    
    end_time = time.time()
    print("Processed " + str(len(ip_dict)) + " IP addresses, " + str(total_count) + " transactions.")
    print("Complete in " + str(end_time - start_time))

if __name__ == '__main__':
    #freeze_support()
    main()