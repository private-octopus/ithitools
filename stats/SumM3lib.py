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

class sumM3Message:
    def __init__(self):
        self.topic = ""
        self.country_code = ""
        self.city_code = ""
        self.node_dns = ""
        self.m3_date = "1901-01-01"
        self.m3_time = "00:00:00"
        self.duration = 0
        self.fpath = ""
        self.bin_date = datetime.date(1,1,1)
        self.bin_time = datetime.time(0,0,0,0)
        
    def fill_values(self, topic, m3n, fpath):
        self.topic = topic
        self.fpath = fpath
        self.copy_m3name_values(m3n)
    
    def to_string(self):
        msg = self.topic + ',' + \
            self.country_code + ',' + self.city_code + ',' + self.node_dns + "," + \
            self.m3_date + ',' + self.m3_time + ',' +  str(self.duration) + ',' + \
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
            self.m3_time = parts[5]
            self.fpath = parts[7]
            self.set_datetime()
            try:
                self.duration = int(parts[6])
            except:
                self.duration = 0
                return False
        return True

    def copy_id(self, m3n):
        self.country_code = m3n.country_code
        self.city_code = m3n.city_code
        self.node_dns = m3n.node_dns

    def copy_m3name_values(self, m3n):
        self.copy_id(m3n)
        self.m3_date = m3n.m3_date
        self.m3_time = m3n.m3_time
        self.set_datetime()
        self.duration = m3n.duration

    def set_datetime(self):
        try:
            self.bin_date = datetime.date.fromisoformat(self.m3_date)
            self.bin_time = datetime.time.fromisoformat(self.m3_time)
        except:
            print("Cannot parse time: " + self.m3_date + " " + self.m3_time)
            pass

    # Analysis: for a given message, add line 
    # to the file <base>/<date>/results-<node_id>.sum3
    def sumM3AppendLine(self, base_dir, sep):
        s3msg_out = sumM3Message()
        s3msg_out.topic = "m3Analysis"
        s3msg_out.copy_m3name_values(self)
        # Read the M3 summary file into a summary line
        msl = m3summary.m3summary_line()
        ret = msl.load_m3(self.fpath)
        if ret != 0:
            print("load_m3(" + self.fpath + ") returns: " + str(ret))
            s3msg_out.topic = "error"
        else:
            # Add line to summary file 
            dir_path = sumM3CreateDirPathDate(self, base_dir, sep)
            sumM3EnsureDir(dir_path)
            s3msg_out.fpath = sumM3FileName(self, dir_path, "result-", ".sum3")
            non_zero = os.path.isfile(s3msg_out.fpath) and os.path.getsize(s3msg_out.fpath) > 0
            sum_file = open(s3msg_out.fpath, "a")
            if not non_zero:
                sum_file.write(m3summary.summary_title_line() + "\n")
            sum_file.write(msl.to_string() + "\n")
            sum_file.close()
        return s3msg_out


# Thresholder class. 
class sum3Thresholder:
    def __init__(self, nb_hours):
        self.node_list = dict()
        self.nb_hours = nb_hours

    def checkList(self, sm3_msg):
        # TODO: instead of working on ascii strings,
        # should work with datetime.
        val = False
        if sm3_msg.node_dns not in self.node_list:
            sm3_msg_item = sumM3Message()
            sm3_msg_item.topic = "m3Thresholder"
            sm3_msg_item.copy_id(sm3_msg)
            sm3_msg_item.bin_date = sm3_msg.bin_date
            sm3_msg_item.m3_date = sm3_msg.m3_date
            sm3_msg_item.fpath = sm3_msg.fpath
            if self.nb_hours < 24:
                sm3_msg_item.bin_time = datetime.time(self.nb_hours, 0, 0, 0)
                val = sm3_msg.bin_time > sm3_msg_item.bin_time
            else:
                sm3_msg_item.bin_time = datetime.time.max
            sm3_msg_item.m3_time = sm3_msg_item.bin_time.isoformat(timespec='seconds')
            self.node_list[sm3_msg.node_dns] = sm3_msg_item
        elif sm3_msg.bin_date > self.node_list[sm3_msg.node_dns].bin_date or \
            (sm3_msg.bin_date == self.node_list[sm3_msg.node_dns].bin_date and \
             sm3_msg.bin_time > self.node_list[sm3_msg.node_dns].bin_time):
            val = True
        return val

    def update(self, sm3_msg):
        if sm3_msg.node_dns not in self.node_list:
            print("unexpected: node <" + sm3_msg.node_dns + "> was not yet created")
            self.node_list[sm3_msg.node_dns] = sm3_msg.deepcopy()
            self.node_list[sm3_msg.node_dns].m3_time = "00:00:00"
        else:
            self.node_list[sm3_msg.node_dns].fpath = sm3_msg.fpath
            self.node_list[sm3_msg.node_dns].m3_date = sm3_msg.m3_date
            self.node_list[sm3_msg.node_dns].m3_time = "00:00:00"
        self.node_list[sm3_msg.node_dns].set_datetime()
        this_time = sm3_msg.bin_time
        next_hour = self.nb_hours
        while True:
            next_time = datetime.time(next_hour, 0, 0, 0)
            if next_time >  this_time:
                self.node_list[sm3_msg.node_dns].bin_time = next_time
                self.node_list[sm3_msg.node_dns].m3_time = next_time.isoformat(timespec='seconds')
                break
            next_hour += self.nb_hours
            if next_hour >= 24:
                self.node_list[sm3_msg.node_dns].bin_time = datetime.time.max
                self.node_list[sm3_msg.node_dns].m3_time = self.node_list[sm3_msg.node_dns].bin_time.isoformat(timespec='seconds')
                break



#
# Test program for the common sumM3 API
#

# check the calling argument
if len(sys.argv) != 3:
    print("Usage: " + sys.argv[0] + "<data_dir> <temp_dir>\n")
    exit(1)

data_dir = sys.argv[1]
temp_dir = sys.argv[2]
sep_local = sumM3FileSeparator(temp_dir)
if len(data_dir) > 0 and data_dir[len(data_dir)-1] != sep_local[0]:
    basefile += sep_local
if len(temp_dir) > 0 and temp_dir[len(temp_dir)-1] != sep_local[0]:
    basefile += sep_local

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

sep_test = "/"
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
    "m3Capture,in,bom,aa01-in-bom.l.dns.icann.org,2019-06-09,13:28:48,300,ithi_base/2019-06-09/result-aa01-in-bom.l.dns.icann.org.sum3",
    "m3Capture,fr,par,aa01-fr-par.l.dns.icann.org,2019-06-09,14:48:34,25,ithi_base/2019-06-09/result-aa01-fr-par.l.dns.icann.org.sum3",
    "m3Capture,bh,bah,bah01.l.root-servers.org,2018-05-12,10:57:48,300,ithi_base/2018-05-12/result-bah01.l.root-servers.org.sum3" ]

i = 0
while i < len(m3_list) and success:
    m3n = m3name.m3name()
    m3n.parse_file_id(m3_list[i])
    dir_path = sumM3CreateDirPathDate(m3n, m3_base, sep_test)
    if dir_path != m3D_list[i]:
        print(" For string <" + m3_list[i] + "> got dir name <" + dir_path + ">")
        success = False
    else:
        fn = sumM3FileName(m3n, dir_path, "result-", ".sum3")
        if fn != m3F_list[i]:
            print(" For string <" + m3_list[i] + "> got file name <" + fn + ">")
            success = False
        else:
            s3msg = sumM3Message()
            s3msg.fill_values("m3Capture", m3n, fn)
            msg = s3msg.to_string()
            if msg != m3msg_list[i]:
                print(" For string <" + m3_list[i] + "> got message <" + msg + ">")
                success = False

    i += 1
if success:
    print("tests of createDirPath, sumM3FileName and sumM3Message succeed.")

if success:
    yyyymmdd_list = [ "", "0000", "00000000", "20190512" ]
    date_list = [ datetime.date(1,1,1), datetime.date(1,1,1), datetime.date(1,1,1), datetime.date(2019,5,12)]
    i = 0
    while i < len(m3_list) and success:
        test_date = sumM3Date(yyyymmdd_list[i])
        if test_date != date_list[i]:
            print(" For string <" + yyyymmdd_list[i] + "> got date <" + test_date.isoformat() + ">")
            success = False
        i += 1
    if success:
        print("test of YYYYMMDD conversion succeeds.")


# Check that the analysis data is correctly written to disk
node_dns_sample = "aa01-us-lax.l.dns.icann.org"
file_expected = temp_dir + "2017-01-31" + sep_local + "result-" + node_dns_sample + ".sum3"
capture_sample = data_dir + "20170131-093117_300-" + node_dns_sample + ".csv"
msg_in_sample="m3Capture,us,lax," + node_dns_sample + ",2017-01-31,09:31:17,300," + capture_sample
sum3_line_sample = "US,LAX,AA01,2017-01-31,09:31:17,300," + node_dns_sample + "," + capture_sample
msg_out_sample= "m3Analysis,us,lax," + node_dns_sample + ",2017-01-31,09:31:17,300," + file_expected

if success :

    try:
        os.unlink(file_expected)
        print("File <" + file_expected + "> deleted.")
    except:
        print("File <" + file_expected + "> was not present before test.")
    s3msg_in = sumM3Message()
    s3msg_in.parse(msg_in_sample)
    s3msg_out = s3msg_in.sumM3AppendLine(temp_dir, sep_local)
    if s3msg_out.to_string() != msg_out_sample:
        print("sumM3AppendLine returns <" + s3msg_out.to_string() + ">")
        success = False
    elif not os.path.isfile(file_expected):
        print("File <" + file_expected + "> was not created by test.")
        success = False
    else:
        print("Test of sumM3AppendLine succeeds")

# test the thresholding functions

np_hours_thresh = 12
msg_in_thresh = [
     "m3Analysis,us,lax," + node_dns_sample + ",2017-01-31,09:31:17,300," + file_expected,
     "m3Analysis,us,lax," + node_dns_sample + ",2017-01-31,13:17:22,300," + file_expected,
     "m3Analysis,us,lax," + node_dns_sample + ",2017-01-31,15:22:01,300," + file_expected,
     "m3Analysis,us,lax," + node_dns_sample + ",2017-01-31,23:17:22,300," + file_expected,
     "m3Analysis,fr,par,aa01-fr-par.l.dns.icann.org,2017-02-01,01:11:49,300," + temp_dir + \
         "2017-02-01" + sep_local + "result-aa01-fr-par.l.dns.icann.org.sum3",
     "m3Analysis,us,lax," + node_dns_sample + ",2017-02-01,01:11:49,300," + file_expected
    ]
msg_out_thresh = [
    "",
    "m3Thresholder,us,lax," + node_dns_sample + ",2017-01-31,12:00:00,0," + file_expected,
    "",
    "",
    "",
    "m3Thresholder,us,lax," + node_dns_sample + ",2017-01-31,23:59:59,0," + file_expected
    ]

if success:
    # test the thresholding functions
    thr = sum3Thresholder(np_hours_thresh)

    i = 0
    while i < len(msg_in_thresh) and success:
        thresh_in = sumM3Message()
        thresh_in.parse(msg_in_thresh[i])
        if thr.checkList(thresh_in):
            thresh_msg = thr.node_list[thresh_in.node_dns]
            if thresh_msg.to_string() != msg_out_thresh[i]:
                print("For thresholder <" + msg_in_thresh[i] + "> got <" +
                      thresh_msg.to_string() + ">")
                success = False
            thr.update(thresh_in)
        elif msg_out_thresh[i] != "":
            print("For thresholder <" + msg_in_thresh[i] + "> got False")
            success = False
        i += 1
    if success:
        print("Test of sum3Thresholder succeeds")


if not success:
    exit(1)
else:
    print("Success")
    exit(0)