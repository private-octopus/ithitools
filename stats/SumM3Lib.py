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
from confluent_kafka import Consumer, Producer

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

def sumM3EnsureEndInSep(folder_name, sep):
    if len(folder_name) > 0 and folder_name[len(folder_name)-1] != sep[0]:
        folder_name += sep
    return folder_name

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
            print("Error parsing " + msg);
            print("Expected 8 parts, got " + str(len(parts)))
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
                print("Error parsing " + msg);
                print("Expected duration, got " + parts[6])
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

    def poll_kafka(self, c, how_long):
        time_passed = 0.0
        self.topic = ""
        while time_passed < how_long:
            time_passed += 10.0 
            msg = c.poll(10.0)
            if not msg is None:
                if msg.error():
                    print('error: {}'.format(msg.error()))
                else:
                    # The Kafka message should provide keys of this summary: location and date,
                    # and the name of the file.
                    record_value = msg.value().decode('utf-8')
                    # Parse the message value, retrieve the capture file name, 
                    if not self.parse(record_value):
                        print("Could not parse: " + record_value)
                    else:
                        print("Received: " + record_value)
                    break
        return

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

