#!/usr/bin/env python
# coding=utf-8
#
# ITHI Kafka functions, used in multiple modules.

import sys
import os
import codecs
from enum import Enum
import copy
import traceback
import datetime
import m3name
import m3summary
from pathlib import Path

# Convert the string yyyymmdd into date object
def sumM3Date(yyyymmdd):
    this_date = datetime.date(1,1,1)
    if len(yyyymmdd) == 8:
        try:
            yyyy = int(yyyymmdd[0:3], 10)
            mm = int(yyyymmdd[4:5], 10)
            dd = int(yyyymmdd[6:7], 10)
            this_date = datetime.date(yyyy, mm, dd)
        except:
            pass
    return(this_date)

# Find the part separator: "/" as Unix or "\" as Windows?
def sumM3FileSeparator(basefile):
    part_slash = basefile.split("/")
    part_backslash = basefile.split("\\")
    if len(part_backslash) > len(part_slash):
        sep = "\\"
    else:
        sep = "/"
    return sep

# Check that a directory exists and if not create it
def sumM3EnsureDir(dir_path):
    Path(dir_path).mkdir(parents=True, exist_ok=True)

# Create directory by date
def sumM3CreateDirPathDate(m3n, basedir, sep):
    dir_path = basedir + m3n.m3_date + sep
    return dir_path

# Create a file name from a base file name and an M3 message, with
# the template:
# <basefile>/yyyy-mm-dd/results-aa01-br-sao.l.dns.icann.org.sum3
def sumM3FileName(m3n, dir_path, prefix, suffix):
    file_name = dir_path + prefix + m3n.node_dns + suffix
    return(file_name)

# Analysis
# The "message_in" specifies a file name and properties
# in "m3name" format. For a given message, add line 
# to the file <base>/<date>/results-<node_id>.sum3
def sumM3AppendLine(message_in, base_dir, sep):
    # Parse the message value as M3Name data and file name
    m3n = m3name.m3name()
    if not m3n.sumM3Parse(message_in):
        print("Failed to parse message: %s" % (message_in))
    else:
        print("File:" + m3n.file_id)
        print("Node:" + m3n.node_dns)
        # Read the M3 summary file into a summary line
        msl = m3summary.m3summary_line()
        ret = msl.load_m3(m3n.file_id)
        if ret != 0:
            print("load_m3(" + m3n.file_id + ") returns: " + str(ret))
        else:
            # Add line to summary file 
            dir_path = sumM3CreateDirPathDate(m3n, base_dir, sep)
            sumM3EnsureDir(dir_path)
            fpath = sumM3FileName(m3n, dir_path, "result-", ".sum3")
            non_zero = os.path.isfile(fpath) and os.path.getsize(fpath) > 0
            sum_file = open(fpath, "a")
            if not non_zero:
                sum_file.write(m3summary.summary_title_line() + "\n")
            sum_file.write(msl.to_string() + "\n")
            sum_file.close()


class sumM3Message:
    def __init__(self):
        self.topic = ""
        self.country_code = ""
        self.city_code = ""
        self.node_dns = ""
        self.m3_date = ""
        self.m3_hour = ""
        self.duration = 0
        self.fpath = ""
        
    def __init__(self, topic, m3n, fpath):
        self.topic = topic
        self.fpath = fpath
        self.copy_m3name_values(m3n)
    
    def to_string(self):
        msg = self.topic + ',' + \
            self.country_code + ',' + self.city_code + ',' + self.node_dns + "," + \
            self.m3_date + ',' + self.m3_hour + ',' +  str(self.duration) + ',' + \
            self.fpath
        return msg

    def parse(self, msg):
        parts = msg.split(",")
        if len(parts) != 8:
            return False
        else:
            self.topic = parts[0]
            self.country_code = parts[1]
            self.city_code = parts[2]
            self.node_dns = parts[3]
            self.m3_date = parts[4]
            self.m3_hour = parts[5]
            self.file_id = parts[7]
            try:
                self.duration = int(parts[6])
            except:
                self.duration = 0
                return False
        return True

    def copy_m3name_values(self, m3n):
        self.country_code = m3n.country_code
        self.city_code = m3n.city_code
        self.node_dns = m3n.node_dns
        self.m3_date = m3n.m3_date
        self.m3_hour = m3n.m3_hour
        self.duration = m3n.duration
        





#
# Test program for the common sumM3 API
#

f_list = [ "aaa/bbb/ccc", "aaa\\bbb\\ccc", "./", ".\\", ""]
sep_list = [ "/", "\\", "/", "\\", "/" ]
success = True
i = 0
while i < len(f_list) and success:
    sep = sumM3FileSeparator(f_list[i])
    if sep != sep_list[i]:
        print(" For string <" + f_list[i] + "> got separator <" + sep + ">")
        success = False
    i += 1

sep = "/"
m3_base = "ithi_base/"
m3_list = [   
        "/home/rarends/data/20190609/aa01-in-bom.l.dns.icann.org/20190609-132848_300-aa01-in-bom.l.dns.icann.org.csv",
        "20190609-144834_25-aa01-fr-par.l.dns.icann.org.csv",
        "20180512-105748_300-bah01.l.root-servers.org.csv" ]
m3D_list = [
    "ithi_base/2019-06-09/",
    "ithi_base/2019-06-09/",
    "ithi_base/2018-05-12/" ]
m3F_list = [
    "ithi_base/2019-06-09/result-aa01-in-bom.l.dns.icann.org.sum3",
    "ithi_base/2019-06-09/result-aa01-fr-par.l.dns.icann.org.sum3",
    "ithi_base/2018-05-12/result-bah01.l.root-servers.org.sum3" ]
m3msg_list = [
    "capture,in,bom,aa01-in-bom.l.dns.icann.org,2019-06-09,13:28:48,300,ithi_base/2019-06-09/result-aa01-in-bom.l.dns.icann.org.sum3",
    "capture,fr,par,aa01-fr-par.l.dns.icann.org,2019-06-09,14:48:34,25,ithi_base/2019-06-09/result-aa01-fr-par.l.dns.icann.org.sum3",
    "capture,bh,bah,bah01.l.root-servers.org,2018-05-12,10:57:48,300,ithi_base/2018-05-12/result-bah01.l.root-servers.org.sum3" ]

i = 0
while i < len(m3_list) and success:
    m3n = m3name.m3name()
    m3n.parse_file_id(m3_list[i])
    dir_path = sumM3CreateDirPathDate(m3n, m3_base, sep)
    if dir_path != m3D_list[i]:
        print(" For string <" + m3_list[i] + "> got dir name <" + dir_path + ">")
        success = False
    else:
        fn = sumM3FileName(m3n, dir_path, "result-", ".sum3")
        if fn != m3F_list[i]:
            print(" For string <" + m3_list[i] + "> got file name <" + fn + ">")
            success = False
        else:
            s3msg = sumM3Message("capture", m3n, fn)
            msg = s3msg.to_string()
            if msg != m3msg_list[i]:
                print(" For string <" + m3_list[i] + "> got message <" + msg + ">")
                success = False

    i += 1

yyyymmdd_list = [ "", "0000", "00000000", "20190512" ]
date_list = [ datetime.date(1,1,1), datetime.date(1,1,1), datetime.date(1,1,1), datetime.date(2019,5,12)]
i = 0
while i < len(m3_list) and success:
    test_date = sumM3Date(yyyymmdd_list[i])
    if test_date != date_list[i]:
        print(" For string <" + yyyymmdd_list[i] + "> got date <" + test_date.isoformat() + ">")
        success = False
    i += 1

# if success :
#    sumM3AppendLine("US,LAX,AA01,2017-01-31,09:31:17,300,aa01-us-lax.l.dns.icann.org,.\\20170131-093117_300-aa01-us-lax.l.dns.icann.org.csv", "..\\x64\\Debug\\", "\\", sep)


if not success:
    exit(1)
else:
    print("Success")
    exit(0)