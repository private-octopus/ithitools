
#!/usr/bin/python
# coding=utf-8
#
# This script takes as input a list of "address" files and provides a summary
# with one line per address in the file. The it computes a simple summary 
# with one line per address.

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


class file_bucket:
    def __init__(self):
        self.ip_dict = dict()
        self.input_files = []
        self.input_path = ""
        self.total_count = 0
        self.previous_ip = ""
        self.bucket_id = 0

    def add_line(self, line, input_file):
        aline = address_line()
        aline.file_line(line)  
        if len(aline.ip) > 0:
            if aline.ip != self.previous_ip:
                self.previous_ip = aline.ip
                if not aline.ip in self.ip_dict:
                    self.ip_dict[aline.ip] = address_file_line(aline.ip)
                self.ip_dict[aline.ip].add_file(input_file)
            self.ip_dict[aline.ip].update(aline)
            self.total_count += aline.count

    def load(self):
        try:
            for input_file in self.input_files:
                self.previous_ip = ""
                if input_file.endswith(".gz"):      
                    for line in gzip.open(join(self.input_path,input_file), 'rt'):
                        self.add_line(line, input_file)
                else:    
                    for line in open(join(self.input_path,input_file)):
                        self.add_line(line, input_file)
        except:
            traceback.print_exc()
            print("Abandon bucket " + str(self.bucket_id))
        return True

def load_bucket(bucket):
    bucket.load()

# Main loop


if len(sys.argv) != 3:
    print("Usage: " + sys.argv[0] + " <input-folder> <count_file.csv> \n")
    exit(1)

input_path = sys.argv[1]
count_file = sys.argv[2]

input_files = [f for f in listdir(input_path ) if isfile(join(input_path, f))]

nb_threads = os.cpu_count()
print("Aiming for " + str(nb_threads) + " threads")

step = int((len(input_files)+nb_threads-1)/nb_threads)
bucket_list = []
bucket_first = 0
bucket_id = 0
while bucket_first < len(input_files):
    bucket = file_bucket()
    bucket_next = min(bucket_first+step, len(input_files))
    bucket.input_files = input_files[bucket_first:bucket_next]
    bucket.input_path = input_path
    bucket.bucket_id = bucket_id
    bucket_list.append(bucket)
    bucket_id += 1
    bucket_first = bucket_next

nb_threads = min(nb_threads, len(bucket_list))
print("Aiming for " + str(nb_threads) + " threads, " + str(len(bucket_list)) + " buckets")

start_time = time.time()
total_count = 0
with concurrent.futures.ThreadPoolExecutor(max_workers = nb_threads) as executor:
    future_to_bucket = {executor.submit(load_bucket, bucket):bucket for bucket in bucket_list }
    for future in concurrent.futures.as_completed(future_to_bucket):
        bucket = future_to_bucket[future]
        try:
            data = future.result()
            total_count += bucket.total_count
        except Exception as exc:
            print('Bucket %d generated an exception: %s' % (bucket.bucket_id, exc))
        #else:
        #    print('Bucket %d has %d transactions' % (bucket.bucket_id, bucket.total_count))

bucket_time = time.time()
ip_dict = bucket_list[0].ip_dict
for x in range(1, len(bucket_list)):
    for ip in bucket_list[x].ip_dict:
        if ip in ip_dict:
            # merge the two lines
            ip_dict[ip].add(bucket_list[x].ip_dict[ip])
        else:
            ip_dict[ip] = bucket_list[x].ip_dict[ip]

end_time = time.time()
print("Complete in " + str(end_time - start_time))
print("Threads took " + str(bucket_time - start_time))
print("Summary took " + str(end_time - bucket_time))



f_out = open(count_file, "wt")
f_out.write(address_file_line.csv_head())
for ip_address in ip_dict:
    f_out.write(ip_dict[ip_address].to_csv())
f_out.close();
print("Processed " + str(len(ip_dict)) + " IP addresses, " + str(total_count) + " transactions.")
