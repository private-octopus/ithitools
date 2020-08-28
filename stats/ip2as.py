#!/usr/bin/python
# coding=utf-8
#
# IP Address to AS number conversion tool.
# The input is the file produced by "ip2asbuilder.py". This is a csv file,
# with three columns:
#    ip_first: first IP address in range
#    ip_last: last address IP address in range
#    as_number: autonomous system number for that range
# As of August 2020, that file has over 300000 entries, so memory size is
# a concern. We write an adhoc parser for that file, with three elements
# per line:
#    ip_first
#    ip_last
#    AS number.
# The table is ordered, and the ranges do not overlap. We use that to
# implement a simple binary search algorithm, finding the highest index in
# the table such that beginning <= search address. We then return the
# associated AS number if the address is in range, or 0 if it is not.

import sys
import traceback
import ipaddress

class ip2as_line:
    def __init__(self):
        self.ip_first = ipaddress.ip_address("0.0.0.0")
        self.ip_last = ipaddress.ip_address("0.0.0.0")
        self.as_number = 0

    def load(self, s):
        ret = True
        st = s.strip()
        parts = st.split(",")
        try:
            self.ip_first = ipaddress.ip_address(parts[0].strip())
            self.ip_last = ipaddress.ip_address(parts[1].strip())
            self.as_number = int(parts[2].strip())
        except Exception as e:
            traceback.print_exc()
            print("For <" + st + ">: " + str(e))
            ret = False
        return(ret)

class ip2as_table:
    def __init__(self):
        self.table = []

    def load(self,file_name):
        ret = True
        try:
            first = True
            for line in open(file_name, "rt"):
                l = line.strip()
                if first and l == "ip_first, ip_last, as_number,":
                    first = False
                    continue
                il = ip2as_line()
                if il.load(l):
                    self.table.append(il)
        except Exception as e:
            traceback.print_exc()
            print("When loading <" + file_name + ">: " + str(e))
            ret = False
        return ret
    
    def get_asn(self, s):
        asn = 0
        i_first = 0
        i_last = len(self.table) - 1
        try:
            addr = ipaddress.ip_address(s)
            if addr >= self.table[i_first].ip_first:
                if addr >= self.table[i_last].ip_first:
                    i_first = i_last
                else:
                    while i_first + 1 < i_last:
                        i_med = int((i_first + i_last)/2)
                        if addr >= self.table[i_med].ip_first:
                            i_first = i_med
                        else:
                            i_last = i_med
                if addr <= self.table[i_first].ip_last:
                    asn = self.table[i_first].as_number
        except Exception as e:
            traceback.print_exc()
            print("When evaluation <" + s + ">: " + str(e))
            pass
        return asn

class asname:
    def __init__(self):
        self.table = dict()

    def load(self, file_name, test=False):
        ret = True
        nb_asn = 0
        try:
            for line in open(file_name, "rt"):
                l = line.strip()
                asn = 0
                as_check = l[0:2]
                asn_x = l[2:14].strip()
                name = l[14:]
                if test and nb_asn < 10:
                    print(as_check + "//" + asn_x + "//" + name)
                try:
                    asn = int(asn_x)
                    if not asn in self.table:
                        self.table[asn] = name
                        nb_asn += 1
                    elif test:
                        print("Duplicate: " + str(asn) + ", \"" + name + "\" (\"" + self.table[asn] + "\")")
                except Exception as e:
                    traceback.print_exc()
                    print("When parsing asn \"" + asn_x + "\": " + str(e))
                    ret = False
                    break
        except Exception as e:
            traceback.print_exc()
            print("When loading <" + file_name + ">: " + str(e))
            ret = False
        return ret

    def name(self, asn):
        n = 0
        if asn in self.table:
            n = self.table[asn]
        return n