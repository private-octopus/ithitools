
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
#
# TODO: Add AS number.
#
# TODO: Integrate frequent IP documentation. There are by definition 10,000 IP
# addresses in the frequent IP list, which means that it makes more sense to just
# loop on the list and check the list addresses one by one, rather than
# starting with the collected IP dict and checking every address.

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


class file_bucket:
    def __init__(self):
        self.ip_dict = dict()
        self.input_files = []
        self.input_path = ""
        self.total_count = 0
        self.previous_ip = ""
        self.bucket_id = 0
        self.file_path = ""
        self.ip_short = "0.0.0.0"

    def add_line(self, line, input_file):
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
                self.ip_dict[self.ip_short].add_file(input_file)
            self.ip_dict[self.ip_short].update(aline)
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
    if len(sys.argv) != 7:
        print("Usage: " + sys.argv[0] + "<count_file.csv> <temp_prefix> <frequent-ip.csv> <ip2as.csv> <ip2asv6.csv> <input-folder> \n")
        exit(1)

    count_file = sys.argv[1]
    temp_prefix = sys.argv[2]
    frequent_ip = sys.argv[3]
    ip2as_in = sys.argv[4]
    ip2asv6_in = sys.argv[5]
    input_path = sys.argv[6]

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
        bucket.file_path = temp_prefix + "_" + str(bucket.bucket_id) + ".csv"
        bucket_list.append(bucket)
        bucket_id += 1
        bucket_first = bucket_next

    nb_threads = min(nb_threads, len(bucket_list))
    print("Will use " + str(nb_threads) + " threads, " + str(len(bucket_list)) + " buckets")
    total_files = 0
    for bucket in bucket_list:
        total_files += len(bucket.input_files)
    print("%d files in %d buckets (%d .. %d), vs %d" %(total_files, len(bucket_list), len(bucket_list[0].input_files), len(bucket_list[len(bucket_list)-1].input_files), len(input_files)))


    start_time = time.time()
    with concurrent.futures.ProcessPoolExecutor(max_workers = nb_threads) as executor:
        future_to_bucket = {executor.submit(load_bucket, bucket):bucket for bucket in bucket_list }
        for future in concurrent.futures.as_completed(future_to_bucket):
            bucket = future_to_bucket[future]
            try:
                data = future.result()
            except Exception as exc:
                print('Bucket %d generated an exception: %s' % (bucket.bucket_id, exc))
            #else:
            #    print('Bucket %d has %d transactions' % (bucket.bucket_id, bucket.total_count))

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