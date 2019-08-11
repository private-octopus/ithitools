#!/usr/bin/python
# coding=utf-8
#
# This scripts tries to estimate the variance of a few M3 test metrics.

import codecs
import sys
import m3name
import captures
import os
from os.path import isfile, join
import glob

def ithiwalk(file_list, path):
    for x in os.listdir(path):
        y = join(path, x)
        if isfile(y):
            file_list.append(y)
        else:
            ithiwalk(file_list, y)

def load_m3(file_name, sum_m3):
    m3n = m3name.m3name()
    if m3n.parse_file_id(file_name) != 0:
        return -1
    capture = captures.capture_file()
    if capture.load(file_name) != 0:
        return -1
    c0 = capture.find("root-QR", 0, 0, "")
    c1 = capture.find("root-QR", 0, 3, "")
    nb_queries = c0 + c1
    c_tld_home = capture.find("LeakedTLD", 1, 0, "HOME")
    c_tld_corp = capture.find("LeakedTLD", 1, 0, "CORP") 
    c_tld_mail = capture.find("LeakedTLD", 1, 0, "MAIL")
    sum_m3.write(
        m3n.address_id + "," +
        m3n.country_code + "," +
        m3n.city_code + "," +
        m3n.m3_date + "," + 
        m3n.m3_hour + "," + 
        str(duration) + "," 
        + str(nb_queries) + "," + 
        str(c1) + "," +
        str(c_tld_home) + "," + 
        str(c_tld_corp) + "," +
        str(c_tld_mail) +  "\n")
    return 0

# By default, we expect to test with the file "data/tiny_capture.csv, which has 589 lines.

mypath = sys.argv[1]
name_sum_f3 = "sum_f3.csv"
if len(sys.argv) >= 3:
    name_sum_f3 = sys.argv[2]
sum_m3 = codecs.open(name_sum_f3, "w", "UTF-8")
sum_m3.write("address_id" + "," +
                 "CC" + "," + "City" + "," + 
                 "Date" + "," + "Hour" + "," +
                 "Duration" + "," + "nb_queries" + "," + 
                 "Nb NX Domain" + "," + 
                 "Nb .Home" + "," + 
                 "Nb .Corp" + "," + 
                 "Nb .Mail" + "," + "\n")

file_list = []
ithiwalk(file_list,mypath)
nb_loaded = 0
for file in file_list:
    # If this is an M3 capture file, add it.
    if load_m3(file, sum_m3) == 0:
        nb_loaded += 1
sum_m3.close()

print("Found " + str(len(file_list)) + " files, loaded " + str(nb_loaded) + " summaries.")