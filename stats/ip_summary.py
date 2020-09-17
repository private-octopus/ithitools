
#!/usr/bin/python
# coding=utf-8
#
# This script takes as input a summary of IP address file produced
# by "cout_ip.py", and produces a summary of the results
# There are several potential criteria outlined by previous research:
# - Is this IP address mentioned or not in the frequently seen list or not.
# - Is this address generating enough traffic to be interesting, e.g. more than 1,000 queries in a month?
# - Fraction of NX domains + DGA over total
# - Fraction of DGA over total
# - Fraction of .ARPA requests over total "good" queries
# - Fraction of .COM requests over total "good" queries
# - Fraction of time slices for which the address is seen
#
# Each of the "fractions" can have three values: low, high, in between.
# The threshold suggested by initial analysis are:
# - Traffic: low if < 1000 requests, high if > 1000000
# - NX: low if <10%, high if >90%
# - DGA: low if <10%, high if >90%
# - ARPA: low if <2%, high if >10%
# - COM: low if <5%, high if >20%
# - Slices: low if < 25%, high if > 90%
#
# The initial analysis may well be wrong, we will have to adjust it based on broader measurements.
# To facilitate this analysis, we will keep counters for each metric. The slices will be:
# - Traffic: count addresses that generate <10, <100, ... <1000000, >1000000 queries.
# - NX, DGA, COM, slices: count for each 5% slice
# - ARPA: count for each 1% slice, up to 20%
#
# Once criterias have stabilized, we want to count the number of nodes and the share of
# traffic in each "classification bucket". We have 6 tentative criteria and 3 classification dict_values
# (low, middle, high) for each, thus 729 possible buckets. However, some criteria may well
# be very correlated: slices and traffic, NX and DGA, ARPA and COM. This will show in
# the number of entries and volume of traffic corresponding to each bucket. Sorting
# the list by decreasing volume will show the interesting buckets.

import codecs
import sys
from address_file import address_line, address_file_line
import traceback
import ipaddress
import ip2as
import datetime
import math
from os.path import isfile, join
import frequent_ip
from enum import Enum

class summary_enum(Enum):
    by_ip = 1
    by_subnet = 2
    by_asn = 3
    by_asname = 4

def subnet_string(ip):
    ipa = ipaddress.ip_address(ip)
    sn = ""
    if ipa.version == 4:
        sn = str(ipaddress.IPv4Network(ip + "/24", strict=False))
    elif ipa.version == 6:
        sn = str(ipaddress.IPv6Network(ip + "/48", strict=False))
    return sn

class address_summary_line:
    def __init__(self):
        self.ip = ""
        self.subnet = ""
        self.as_id = ""
        self.as_name = ""
        self.frequent = 0.0
        self.total = 0
        self.good = 0
        self.dga = 0
        self.nb_slices = 0
        self.nb_ip = 0
        self.nb_asn = 0
        self.nb_subnet = 0
        self.nb_files = 0

    def get_key(self, summary_type):
        key = ""
        if summary_type == summary_enum.by_asname:
            key = self.as_name
        elif summary_type == summary_enum.by_asn:
            key = self.as_id
        elif summary_type == summary_enum.by_subnet:
            key = self.subnet
        else:
            key = self.ip
        return key

    def add(self, other, summary_type):
        self.frequent += other.frequent
        self.total += other.total
        self.good += other.good
        self.dga += other.dga
        self.nb_slices = max(self.nb_slices, other.nb_slices)
        if summary_type == summary_enum.by_asname:
            self.nb_ip += other.nb_ip
            self.nb_subnet += other.nb_subnet
            self.nb_asn += other.nb_asn
        elif summary_type == summary_enum.by_asn:
            self.nb_ip += other.nb_ip
            self.nb_subnet += other.nb_subnet
            self.nb_asn = 1
        elif summary_type == summary_enum.by_subnet:
            self.nb_ip += other.nb_ip
            self.nb_subnet = 1
            self.nb_asn = 1
        else:
            self.nb_ip = 1
            self.nb_subnet = 1
            self.nb_asn = 1

    def csv_head():
        s = "ip,"
        s += "subnet,"
        s += "as_id,"
        s += "as_name,"
        s += "frequent,"
        s += "total,"
        s += "good,"
        s += "dga,"
        s += "nb_slices,"
        s += "nb_ip,"
        s += "nb_asn,"
        s += "nb_subnet,"
        return s

    def to_csv(self):
        s = self.ip + ","
        s += self.subnet + ","
        s += self.as_id + ","
        s += self.as_name + ","
        s += str(self.frequent) + ","
        s += str(self.total) + ","
        s += str(self.good) + ","
        s += str(self.dga) + ","
        s += str(self.nb_slices) + ","
        s += str(self.nb_ip) + ","
        s += str(self.nb_asn) + ","
        s += str(self.nb_subnet) + ","
        return s

class address_summary:
    def __init__(self, summary_type):
        self.table = dict()
        self.summary_type = summary_type
        self.last_key = ""
        self.nb_fails = 0

    def add_address_line(self, al_line, as_table, fip_table):
        al = address_file_line("")
        al.from_csv(al_line)
        if len(al.ip) > 0:
            if al.ip != self.last_key:
                if self.summary_type != summary_enum.by_ip:
                    raise Exception("Sorry, the add_address_line method only works for summary by ip")
                if not al.ip in self.table:
                    # create a summary line
                    sl = address_summary_line()
                    sl.ip = al.ip
                    sl.subnet = subnet_string(al.ip)
                    if sl.ip in fip_table:
                        sl.frequent = fip_table[sl.ip].count_users_weighted
                    else:
                        sl.frequent = 0.0
                    sl.as_id = "AS" + str(al.asn)
                    if sl.as_id in as_table:
                        sl.as_name = as_table[sl.as_id]
                    sl.nb_ip = 1
                    sl.nb_asn = 1
                    sl.nb_as_name = 1
                    sl.nb_subnet = 1
                    sl.nb_files = 1
                    self.table[al.ip] = sl
                else:
                    self.table[al.ip].nb_files += 1
                self.last_key = al.ip
            self.table[al.ip].total += al.total()
            self.table[al.ip].good += al.good()
            self.table[al.ip].dga += al.dga
            self.table[al.ip].nb_slices = max(self.table[al.ip].nb_slices, al.nb_slices)
        else:
            if self.nb_fails < 10:
                print("Fail: " + al_line.strip())
            self.nb_fails+= 1

    def add_address_file(self, file_name, as_table, fip_table):
        if self.summary_type != summary_enum.by_ip:
            raise Exception("Sorry, the add_address_file method only works for summary by ip")
        for line in open(file_name,"rt"):
            self.add_address_line(line, as_table, fip_table)

    def add_summary_line(self,sl):
        key = sl.get_key(self.summary_type)
        if self.last_key != key:
            if not key in self.table:
                sx = address_summary_line()
                sx.ip = sl.ip
                sx.subnet = sl.subnet
                sx.as_id = sl.as_id
                sx.as_name = sl.as_name
                self.table[key] = sx
            self.last_key = key
        self.table[key].add(sl, self.summary_type)

    def add_summary(self,su):
        if (self.summary_type == summary_enum.by_asname and su.summary_type != summary_enum.by_asn) or \
            (self.summary_type == summary_enum.by_asn and su.summary_type != summary_enum.by_subnet) or \
            (self.summary_type == summary_enum.by_subnet and su.summary_type != summary_enum.by_ip) or \
            (self.summary_type == summary_enum.by_ip):
            raise Exception("Incompatible summary types " + str(self.summary_type) + " and " + str(su.summary_type))
        for su_key in su.table:
            self.add_summary_line(su.table[su_key])

    def save_as_csv(self, file_name, min_u, min_t):
        nb_lines = 0
        with open(file_name,"wt") as w_out:
            w_out.write(address_summary_line.csv_head() + "\n")
            for sl in self.table:
                if self.table[sl].frequent >= min_u and self.table[sl].frequent >= min_t:
                    w_out.write(self.table[sl].to_csv() + "\n")
                    nb_lines += 1
        return nb_lines

