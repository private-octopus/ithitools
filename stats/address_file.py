#!/usr/bin/python
# coding=utf-8
#
# Definition of classes used to compute the address summaries.

import traceback

# Address line: parsing of a line of the "address" files produced by ithitools
class address_line:
    def __init__(self):
        self.ip = ""
        self.tld = ""
        self.nx_domain = False
        self.name_type = ""
        self.tld_min_delay = -1
        self.tld_count = 0

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
        self.frequent = 0
        self.nx_domain = 0
        self.arpa = 0
        self.tld = 0
        self.tld_min_delay = -1
        self.nb_files = 0
        self.last_file = ""

    def update(self, aline):
        if aline.nx_domain:
            self.nx_domain += aline.count
        elif aline.tld.upper() == "ARPA":
            self.arpa += aline.count
        else:
            self.tld += aline.count
            if aline.tld_min_delay > 0 and (self.tld_min_delay < 0 or self.tld_min_delay > aline.tld_min_delay):
                self.tld_min_delay = aline.tld_min_delay

    def add_file(self, last_file):
        if last_file != self.last_file:
            self.nb_files += 1
            self.last_file = last_file

    def csv_head():
        return "ip,asn,frq,total,nx_domain,arpa,tld,tld_min_delay,nb_files\n"

    def to_csv(self):
        total = self.nx_domain + self.arpa + self.tld
        s = self.ip + "," + str(self.asn) + "," + str(self.frequent) + "," + str(total) + "," + \
            str(self.nx_domain) + "," + str(self.arpa) + "," + str(self.tld) + "," + \
            str(self.tld_min_delay) + "," + str(self.nb_files) + "\n"
        return s

    def from_csv(self, line):
        csv = line.split(",")
        try:
            self.ip = csv[0].strip()
            self.asn = int(csv[1].strip())
            self.frequent = int(csv[2].strip())
            self.nx_domain = int(csv[4].strip())
            self.arpa = int(csv[5].strip())
            self.tld = int(csv[6].strip())
            self.tld_min_delay = int(csv[7].strip())
            self.nb_files = int(csv[8].strip())
        except:
            if csv[0] != "ip":
                traceback.print_exc()
                print("Cannot parse: " + line.strip() + "\n")
            self.ip = ""
    
    def add(self, other):
        self.nx_domain += other.nx_domain
        self.arpa += other.arpa
        self.tld += other.tld
        if other.tld_min_delay > 0 and (self.tld_min_delay < 0 or self.tld_min_delay > other.tld_min_delay):
            self.tld_min_delay = other.tld_min_delay
        self.nb_files += other.nb_files
    