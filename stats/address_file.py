#!/usr/bin/python
# coding=utf-8
#
# Definition of classes used to compute the address summaries.

import traceback
import datetime

# slice to time and time to slice utilities
def slice_to_time(slice):
    year = int(slice[0:4])
    month = int(slice[4:6])
    day = int(slice[6:8])
    hour = int(slice[9:11])
    minute = int(slice[11:13])
    second = int(slice[13:15])
    t = datetime.datetime(year, month, day, hour, minute, second)
    return t

def time_to_slice(t):
    s = t.strftime("%Y%m%d-%H%M%S")
    return s

def time_duration_to_slice_time(t,duration):
    # set time to the middle of the duration interval
    dt = datetime.timedelta(seconds=(duration/2))
    t += dt
    # set minute to the start of the 5 minute interval before the middle
    t = t.replace(minute = 5*int(t.minute / 5), second=0)
    return t

# source file: find the slice definition from a file name.
class source_file:
    def __init__(self, folder, file_name, f_num):
        self.folder = folder
        self.file_name = file_name
        self.f_num = f_num
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
            # set time slice to start of 5 minute period including the middle of the duration interval
            t = time_duration_to_slice_time(t, duration)
            # get the text value of the time
            self.slice = time_to_slice(t)
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


# Address line: parsing of a line of the "address" files produced by ithitools
class address_line:
    def __init__(self):
        self.ip = ""
        self.tld = ""
        self.nx_domain = False
        self.name_type = ""
        self.tld_min_delay = -1
        self.count = 0

    def file_line(self, line):
        csv = line.split(",")
        try:
            # start with getting all the required numbers, throw an exception is incorrect.
            nx = int(csv[2])
            min_d = int(csv[4])
            count = int(csv[5])
            # the assignment will only happen if the line was correct
            self.ip = csv[0].strip()
            qtld = csv[1].strip()
            self.tld = qtld.strip('\"')
            if nx != 0:
                self.nx_domain = True
            self.name_type = csv[3].strip()
            self.tld_min_delay = min_d
            self.count = count
        except:
            if csv[0] != "Address":
                traceback.print_exc()
                print("Cannot parse: " + line.strip() + "\n")
                self.ip = ""

# File properties: the files names are of the form:
# 20190630-201942_300.cbor.xz-results-addr.csv
# Parsing the file name extracts the date of the file capture

class address_file_line:
    def __init__(self, ip):
        self.ip = ip
        self.asn = 0
        self.as_name = ""
        self.frequent = 0.0
        self.dga = 0
        self.nx_domain = 0
        self.arpa = 0
        self.com = 0
        self.tld = 0
        self.tld_min_delay = -1
        self.nb_slices = 0
        self.last_slice = ""
        self.org = 0
        self.local = 0
        self.home = 0
        self.lan = 0
        self.users = 0
        self.min_slices = 0
        self.nb_addresses = 0

    def update(self, aline):
        if aline.nx_domain:
            if aline.name_type == "dga":
                self.dga += aline.count
            elif aline.tld.upper() == "LOCAL":
                self.local += aline.count
            elif aline.tld.upper() == "HOME":
                self.home += aline.count
            elif aline.tld.upper() == "LAN":
                self.lan += aline.count
            else:
                self.nx_domain += aline.count
        else:
            if aline.tld.upper() == "ARPA":
                self.arpa += aline.count
            elif aline.tld.upper() == "COM":
                self.com += aline.count
            elif aline.tld.upper() == "ORG":
                self.org += aline.count
            else:
                self.tld += aline.count
            if aline.tld_min_delay > 0 and (self.tld_min_delay < 0 or self.tld_min_delay > aline.tld_min_delay):
                self.tld_min_delay = aline.tld_min_delay

    def add_slice(self, slice):
        if slice != self.last_slice:
            self.nb_slices += 1
            self.last_slice = slice

    def csv_head():
        s = "ip,asn,frq,total,dga,nx_domain,arpa,com,tld,tld_min_delay,nb_slices"
        s += ",org,local,home,lan,users,min_slices,nb_addresses\n"
        return s

    def nx_non_dga(self):
        t = self.local + self.home + self.lan + self.nx_domain
        return t
    def nx(self):
        t = self.dga + self.nx_non_dga()
        return t
    def good(self):
        t = self.arpa + self.com + self.org + self.tld
        return t
    def total(self):
        t = self.nx() + self.good()
        return t

    def to_csv(self):
        s = str(self.ip) + "," + str(self.asn) + "," + str(self.frequent) + "," + str(self.total()) + "," + \
            str(self.dga) + "," + str(self.nx_domain) + "," + \
            str(self.arpa) + "," + str(self.com) + "," + str(self.tld) + "," + \
            str(self.tld_min_delay) + "," + str(self.nb_slices) + "," + \
            str(self.org) + "," + str(self.local) + "," + str(self.home) + "," + \
            str(self.lan) + "," + str(self.users) + "," + str(self.min_slices) +  "," + \
            str(self.nb_addresses) + "\n"
        return s

    def from_csv(self, line):
        csv = line.split(",")
        try:
            self.ip = csv[0].strip()
            self.asn = int(csv[1].strip())
            self.frequent = float(csv[2].strip())
            self.dga = int(csv[4].strip())
            self.nx_domain = int(csv[5].strip())
            self.arpa = int(csv[6].strip())
            self.com = int(csv[7].strip())
            self.tld = int(csv[8].strip())
            self.tld_min_delay = int(csv[9].strip())
            self.nb_slices = int(csv[10].strip())
            if len(csv) > 17:
                self.org = int(csv[11].strip())
                self.local = int(csv[12].strip())
                self.home = int(csv[13].strip())
                self.lan = int(csv[14].strip())
                self.users = int(csv[15].strip())
                self.min_slices =  int(csv[16].strip())
                self.nb_addresses = int(csv[17].strip())
            else:
                self.min_slices =  self.nb_slices 
                self.nb_addresses = 1
        except:
            if csv[0] != "ip":
                traceback.print_exc()
                print("Cannot parse: " + line.strip() + "\n")
            self.ip = ""
    
    def add(self, other):
        self.dga += other.dga
        self.local += other.local
        self.home += other.home
        self.lan += other.lan
        self.nx_domain += other.nx_domain
        self.arpa += other.arpa
        self.com += other.com
        self.org += other.org
        self.tld += other.tld
        self.frequent += other.frequent
        self.users += other.users
        if other.tld_min_delay > 0 and (self.tld_min_delay < 0 or self.tld_min_delay > other.tld_min_delay):
            self.tld_min_delay = other.tld_min_delay
        self.nb_slices += other.nb_slices
        self.min_slices = max(self.min_slices, other.min_slices)
        self.nb_addresses += other.nb_addresses
    