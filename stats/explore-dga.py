#!/usr/bin/python
# coding=utf-8
#
# This script parses a file of extracted names and explore the occurence of multipart DGA,
# versus the total number of DGA. We want to check whether we should create new categories
# for multipart DGA names, or maybe combined categories such as DGA(7) and Length(7) in
# the analysis.
# The script thakes three arguments:
# 1- the CSV file containing the name list to be analyzed - output of ithitools -E option.
# 2- the CSV file where the list of multipart names cataloged as DGA will be written.
# 3- the CSV file whre the statistics on multi-name TLD mistaken as DGA will be written

import sys
import codecs
import os
import traceback

class name_list_entry:
    def __init__(self):
        self.name = ""
        self.nx_domain = 0
        self.name_type = ""
        self.count = 0
        self.tld = ""
        self.nb_parts = 0

    def is_header_line(text_line):
        if text_line == "Name, nx_domain, name_type, count":
            return True
        else:
            return False

    def load(self, text_line):
        is_good_line = False
        parts = text_line.split(",")
        if len(parts) == 4:
            try:
                self.name = parts[0]
                self.nx_domain = int(parts[1],10)
                self.name_type = parts[2]
                self.count = int(parts[3],10)
                name_parts = parts[0].split(".")
                self.nb_parts = len(name_parts)
                if (self.nb_parts > 0):
                    self.tld = name_parts[self.nb_parts - 1]
                is_good_line = True
            except:
                print("Cannot parse input line <" + text_line + ">")
        return is_good_line

    
    def compare(self, other):
        if (self.tld < other.tld):
            return -1
        elif (self.tld > other.tld):
            return 1
        elif (self.name < other.name):
            return -1
        elif (self.name > other.name):
            return 1
        if (self.nx_domain < other.nx_domain):
            return -1
        elif (self.nx_domain > other.nx_domain):
            return 1
        elif (self.name_type < other.name_type):
            return -1
        elif (self.name_type > other.name_type):
            return 1
        elif (self.nb_parts < other.nb_parts):
            return -1
        elif (self.nb_parts > other.nb_parts):
            return 1
        elif (self.count < other.count):
            return -1
        elif (self.count > other.count):
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

class tld_summary:
    def __init__(self):
        self.tld = ""
        self.nx_domain = 0
        self.name_type = ""
        self.count = 0
        self.max_parts = 0

    def add_list_entry(self, list_entry):
        if self.tld == "":
            self.tld = list_entry.tld
            self.nx_domain = list_entry.nx_domain
            self.name_type = list_entry.name_type
        elif self.tld != list_entry.tld:
            return False
        self.count += list_entry.count
        if self.max_parts < list_entry.nb_parts:
            self.max_parts = list_entry.nb_parts
        return True

class tld_list:
    def __init__(self):
        self.tld_list = []
        self.is_sorted = False

    def save(self, file_name):
        try:
            tld_file = codecs.open(file_name, "w", "UTF-8")
            tld_file.write("tld, nx_domain, name_type, max_parts, count\n")
            for tl in self.tld_list:
                tld_file.write(tl.tld + ","+ str(tl.nx_domain) + ","  + tl.name_type + ","  + str(tl.max_parts)  +  ","  + str(tl.count) + "\n");
            tld_file.close()
            return True
        except Exception as e:
            traceback.print_exc()
            print("Cannot write <" + file_name + ">, error: " + str(e));
            return False 

class name_list:
    def __init__(self):
        self.name_list = []
        self.is_sorted = False
        self.headline_found = False
        self.sum_count = 0
        self.sum_dga = 0
        self.sum_dga_multi = 0

    def load_line(self, line):
        name_line = name_list_entry()
        ret = name_line.load(line)
        if ret:
            self.name_list.append(name_line)
        return ret
    
    def load_file(self, file_name):
        try:
            self.__init__()
            name_file = codecs.open(file_name, "r", "UTF-8")
            nb_fail = 0
            for line in name_file:
                if (not self.headline_found) and name_list_entry.is_header_line(line.strip()):
                    self.headline_found = True
                elif not self.load_line(line):
                    nb_fail += 1
            name_file.close()
            if nb_fail > 1:
                print("Could not parse " + str(nb_fail) + " lines.")
            if len(self.name_list) == 0:
                print("File <" + file_name + "> is empty")
                return False
            return True
        except Exception:
            traceback.print_exc()
            print("Cannot open <" + file_name + ">");
            return False
        
    def Sort(self):
        if not self.is_sorted:
            self.name_list = sorted(self.name_list)
            self.is_sorted = True

    def save(self, file_name):
        try:
            name_file = codecs.open(file_name, "w", "UTF-8")
            name_file.write("tld, name, nx_domain, name_type, nb_parts, count\n")
            for tl in self.name_list:
                name_file.write(tl.tld + "," + tl.name + ","  + str(tl.nx_domain) + ","  + tl.name_type + "," + str(tl.nb_parts) + "," + str(tl.count) + "\n");
            name_file.close()
            return True
        except Exception as e:
            traceback.print_exc()
            print("Cannot write <" + file_name + ">, error: " + str(e));
            return False 

    def count_dga(self):
        self.sum_count = 0
        self.sum_dga = 0
        self.sum_dga_multi = 0
        for name_entry in self.name_list:
            if name_entry.name_type == "dga":
                if name_entry.nb_parts > 1:
                    self.sum_dga_multi += name_entry.count
                else:
                    self.sum_dga += name_entry.count
            else:
                self.sum_count += name_entry.count

    def extract(self, name_type, min_parts):
        mdlist = name_list()
        for name_entry in self.name_list:
            if (name_entry.name_type == name_type) and (name_entry.nb_parts >= min_parts):
                mdlist.name_list.append(name_entry)
        return mdlist

    def extract_tld_list(self):
        self.Sort()
        tll = tld_list()
        tl = tld_summary()
        for name_entry in self.name_list:
            if not tl.add_list_entry(name_entry):
                tll.tld_list.append(tl)
                tl = tld_summary()
                tl.add_list_entry(name_entry)
        if tl.count > 0:
            tll.tld_list.append(tl)
        return tll

# Main program: obtain the list of multi-name-part TLD that would be confused with DGA.

if len(sys.argv) < 2:
    print("usage: " + "names.csv")
    ret = -1
else:
    ret = 0
    nl = name_list()
    if nl.load_file(sys.argv[1]):
        nl.count_dga()
        print("dga: " + str(nl.sum_dga) + ", multi-dga: " + str(nl.sum_dga_multi) + ", others: " + str(nl.sum_count))

        if len(sys.argv) > 2:
            multi_nl = nl.extract("dga", 2)
            if multi_nl.save(sys.argv[2]):
                print("Saved multi-dga names in " + sys.argv[2])
                if len(sys.argv) > 3 and ret == 0:
                    tll = multi_nl.extract_tld_list()
                    if tll.save(sys.argv[3]):
                        print("Saved multi-dga TLD names in " + sys.argv[3])
                    else:
                        ret = -1
            else:
                ret = -1
        
        
    else:
        ret = -1
exit(ret)
