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
class sumM3Thresholder:
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

# The pattern class is used to select all file updates that meet a pattern
# such as a country name or a city name. If the city is not specified, just
# check the country. If a country is not specified, just check the city.
# if neither is specified, match everything.
# Keep a list of the updated files for a given day. On trigger, provides
# a list of files for the application to compute averages and files.
# The pattern class will hold the files for the current day and the previous
# day. If a new file arrives after the current day, it becomes the
# previous day, and the previous day is swapped out.

class sumM3DayPattern:
    def __init__(self, nb_hours):
        self.bin_date = datetime.date(1,1,1)
        self.bin_time = datetime.time.min
        self.nb_hours = nb_hours
        self.publish_time = datetime.time(nb_hours,0,0,0)
        self.final_publish = False
        self.msg_list = dict()

    def is_too_old(self, current_date, nb_days):
        return (self.bin_date + datetime.timedelta(days=nb_days) < current_date)

    def add_element(self, sm3_msg):
        need_update = False
        if sm3_msg.bin_date == self.bin_date:
            need_update = sm3_msg.bin_time != self.bin_time
            self.msg_list[sm3_msg.node_dns] = sm3_msg
        elif sm3_msg.bin_date > self.bin_date and \
                sm3_msg.node_dns in self.msg_list and \
                self.msg_list[sm3_msg.node_dns].bin_time < datetime.time.max:
            need_update = self.bin_time == self.msg_list[sm3_msg.node_dns].bin_time
            self.msg_list[sm3_msg.node_dns].bin_time = datetime.time.max
        if need_update:
            self.bin_time = datetime.time.max
            for node in self.msg_list:
                if self.msg_list[node].bin_time < self.bin_time:
                    self.bin_time = self.msg_list[node].bin_time

    def publish(self):
        publish_list = []
        if not self.final_publish and self.bin_time >= self.publish_time:
            for node in self.msg_list:
                publish_list.append(self.msg_list[node].fpath)
            if self.bin_time >= datetime.time(23,11,59):
                self.final_publish = True
            else:
                next_hour = self.publish_time.hour + self.nb_hours
                if next_hour > 23:
                    self.publish_time = datetime.time(23,11,59)
                else:
                    self.publish_time = datetime.time(next_hour,0,0)
        return publish_list

    def node_list(self):
        node_list = []
        for node in self.msg_list:
            node_list.append(node)
        return node_list

class sumM3Pattern:
    def __init__(self, name, country_code, city_code, nb_days, nb_hours):
        self.name = name
        self.country_code = country_code
        self.city_code = city_code
        self.nb_days = nb_days
        self.nb_hours = nb_hours
        self.days = []

    # Use pattern match filter to only consider records that match the pattern
    def pattern_match(self, sm3_msg):
        val = (self.country_code == "" or self.country_code == sm3_msg.country_code) and \
              (self.city_code == "" or self.city_code == sm3_msg.city_code)
        return val

    # Remove the old days, add a new one if necessary,
    # and return the index of the new one
    def flush_old(self, sm3_msg):
        found = False
        i = 0
        while i < len(self.days) and not found:
            if self.days[i].bin_date == sm3_msg.bin_date:
                found = True
            elif self.days[i].is_too_old(sm3_msg.bin_date, self.nb_days):
                del self.days[i]
            else:
                i += 1
        if not found:
            new_day = sumM3DayPattern(self.nb_hours)
            new_day.bin_date = sm3_msg.bin_date
            i = len(self.days)
            self.days.append(new_day)
        return i

    def add_element(self, sm3_msg):
        for day_pat in self.days:
            day_pat.add_element(sm3_msg)





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
    thr = sumM3Thresholder(np_hours_thresh)

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
        print("Test of sumM3Thresholder succeeds")



# Test of the pattern matching function.
# Need to test pattern match, removing old days,
# updating the list, and detecting changes.


node_dns_sample2 = "aa02-us-lax.l.dns.icann.org"
file_expected2 = temp_dir + "2017-01-31" + sep_local + "result-" + node_dns_sample2 + ".sum3"

msg_in_pattern = [
     "m3Thresholder,fr,par,aa01-fr-par.l.dns.icann.org,2017-02-01,23:11:59,300," + temp_dir + \
         "2017-02-01" + sep_local + "result-aa01-fr-par.l.dns.icann.org.sum3",
     "m3Thresholder,us,chi,aa01-us-chi.l.dns.icann.org,2017-02-01,12:00:00,300," + temp_dir + \
         "2017-02-01" + sep_local + "result-aa01-us-chi.l.dns.icann.org.sum3",
     "m3Thresholder,us,lax," + node_dns_sample + ",2017-01-31,06:00:00,300," + file_expected,
     "m3Thresholder,us,lax," + node_dns_sample2 + ",2017-01-31,06:00:00,300," + file_expected2,
     "m3Thresholder,us,lax," + node_dns_sample + ",2017-01-31,12:00:00,300," + file_expected,
     "m3Thresholder,us,lax," + node_dns_sample2 + ",2017-01-31,12:00:00,300," + file_expected2,
     "m3Thresholder,us,lax," + node_dns_sample + ",2017-01-31,18:00:00,300," + file_expected,
     "m3Thresholder,us,lax," + node_dns_sample2 + ",2017-01-31,18:00:00,300," + file_expected2,
     "m3Thresholder,us,lax," + node_dns_sample + ",2017-01-31,23:11:59,300," + file_expected,
     "m3Thresholder,us,lax," + node_dns_sample2 + ",2017-01-31,23:11:59,300," + file_expected2,
     "m3Thresholder,us,lax," + node_dns_sample + ",2017-02-01,06:00:00,300," + file_expected,
     "m3Thresholder,us,lax," + node_dns_sample2 + ",2017-02-01,06:00:00,300," + file_expected2,
     "m3Thresholder,us,lax," + node_dns_sample + ",2017-02-01,12:00:00,300," + file_expected,
     "m3Thresholder,us,lax," + node_dns_sample2 + ",2017-02-01,12:00:00,300," + file_expected2,
     "m3Thresholder,us,lax," + node_dns_sample + ",2017-02-01,18:00:00,300," + file_expected,
     "m3Thresholder,us,lax," + node_dns_sample2 + ",2017-02-01,18:00:00,300," + file_expected2,
     "m3Thresholder,us,lax," + node_dns_sample + ",2017-02-01,23:11:59,300," + file_expected,
     "m3Thresholder,us,lax," + node_dns_sample2 + ",2017-02-01,23:11:59,300," + file_expected2,
     "m3Thresholder,us,lax," + node_dns_sample + ",2017-02-02,06:00:00,300," + file_expected]

msg_pattern_is_in = [False, False, True, True, True, True, True, True, True, True, True, True, True, True, True, True, True, True, True]
msg_pattern_is_old = [False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, True]
msg_pattern_pub_0 = [0, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0]
msg_pattern_pub_1 = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0]

if success:
    s3p = sumM3Pattern("us-lax", "us", "lax", 1, 12)
    i = 0
    while i < len(msg_in_pattern) and success:
        pattern_in = sumM3Message()
        if not pattern_in.parse(msg_in_pattern[i]):
            print("Cannot parse message <" + msg_in_pattern[i] + ">")
            success = False
        elif not s3p.pattern_match(pattern_in):
            if msg_pattern_is_in[i]:
                print("Message not in pattern <" + msg_in_pattern[i] + ">")
                success = False
        elif not msg_pattern_is_in[i]:
            print("Message in pattern <" + msg_in_pattern[i] + "> !")
            success = False
        else:
            if len(s3p.days) > 0 and s3p.days[0].is_too_old(pattern_in.bin_date, 1):
                if not msg_pattern_is_old[i]:
                    print("First day old for msg[" + str(i) + "].date = " + str(pattern_in.bin_date))
                    success = False
            elif msg_pattern_is_old[i]: 
                print("First day not old for msg[" + str(i) + "].date = " + str(pattern_in.bin_date))
                success = False
            # if not testing, should check here for processing too old days
            s3p.flush_old(pattern_in)
            # Add the newly received message
            s3p.add_element(pattern_in)
            # Simulate processing
            p_list_0 = []
            p_list_1 = []
            if len(s3p.days) > 0:
                p_list_0 = s3p.days[0].publish()
                if len(p_list_0) != msg_pattern_pub_0[i]:
                    print("Pattern input 0 [" + str(i) + "], got " + str(len(p_list_0)))
                    print("P0. Min time= " + s3p.days[0].bin_time.isoformat())
                    print("P0. Pub time= " + s3p.days[0].publish_time.isoformat())
                    for node in s3p.days[0].msg_list:
                        print(node + ", " + s3p.days[0].msg_list[node].fpath + ", " + s3p.days[0].msg_list[node].bin_time.isoformat())
                    success = False
                else:
                    node_list_0 = s3p.days[0].node_list()
                    if len(node_list_0) != len(p_list_0):
                        print("found " + str(len(node_list_0)) + " nodes[0] instead of " + str(p_list_0))
                        success = False
            elif msg_pattern_pub_0[i] > 0:
                print("Pattern input 0 [" + str(i) + "], list is empty")
                success = False
            if len(s3p.days) > 1:
                p_list_1 = s3p.days[1].publish()
                if len(p_list_1) != msg_pattern_pub_1[i]:
                    print("Pattern input 1 [" + str(i) + "], got " + str(len(p_list_1)))           
                    print("P1. Min time= " + s3p.days[1].bin_time.isoformat())
                    print("P1. Pub time= " + s3p.days[1].publish_time.isoformat())
                    for node in s3p.days[1].msg_list:
                        print(node + ", " + s3p.days[1].msg_list[node].fpath + ", " + s3p.days[1].msg_list[node].bin_time.isoformat())
                    success = False
                else:
                    node_list_1 = s3p.days[1].node_list()
                    if len(node_list_1) != len(p_list_1):
                        print("found " + str(len(node_list_1)) + " nodes[1] instead of " + str(p_list_1))
                        success = False
            elif msg_pattern_pub_1[i] > 0:
                print("Pattern input 1 [" + str(i) + "], list is empty")
                success = False
        i += 1
    if success:
        print("Test of sumM3Pattern succeeds")


if not success:
    exit(1)
else:
    print("Success")
    exit(0)