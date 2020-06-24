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
from SumM3Lib import sumM3Date, sumM3FileSeparator, sumM3EnsureEndInSep, sumM3EnsureDir, \
                     sumM3CreateDirPathDate, sumM3FileName, sumM3Message, \
                     sumM3Thresholder, sumM3DayPattern, sumM3Pattern

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
#if len(data_dir) > 0 and data_dir[len(data_dir)-1] != sep_local[0]:
#    data_dir += sep_local
#if len(temp_dir) > 0 and temp_dir[len(temp_dir)-1] != sep_local[0]:
#    temp_dir += sep_local

data_dir = sumM3EnsureEndInSep(data_dir, sep_local)
temp_dir = sumM3EnsureEndInSep(temp_dir, sep_local)

print("Using data dir: " + data_dir)
print("Using temp dir: " + temp_dir)

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
                elif len(p_list_0) > 0:
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
                elif len(p_list_1) > 0:
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