
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


def add_line(line, ip_dict, total_count, previous_ip):
    aline = address_line()
    aline.file_line(line)  
    if len(aline.ip) > 0:
        if aline.ip != previous_ip:
            previous_ip = aline.ip
            if not aline.ip in ip_dict:
                ip_dict[aline.ip] = address_file_line(aline.ip)
            ip_dict[aline.ip].add_file(input_file)
        ip_dict[aline.ip].update(aline)
        total_count += aline.count
    return total_count, previous_ip

# Main loop


if len(sys.argv) != 3:
    print("Usage: " + sys.argv[0] + " <input-folder> <count_file.csv> \n")
    exit(1)

input_path = sys.argv[1]
count_file = sys.argv[2]

ip_dict = dict()

input_files = [f for f in listdir(input_path ) if isfile(join(input_path, f))]

f_out = open(count_file, "wt")
f_out.write(address_file_line.csv_head())
total_count = 0
for input_file in input_files:
    previous_ip = ""
    if input_file.endswith(".gz"):      
        for line in gzip.open(join(input_path,input_file), 'rt'):
            total_count, previous_ip = add_line(line, ip_dict, total_count, previous_ip)
    else:    
        for line in open(join(input_path,input_file)):
            total_count, previous_ip = add_line(line, ip_dict, total_count, previous_ip)

for ip_address in ip_dict:
    f_out.write(ip_dict[ip_address].to_csv())

f_out.close();
print("Processed " + str(len(ip_dict)) + " IP addresses, " + str(total_count) + " transactions.")
