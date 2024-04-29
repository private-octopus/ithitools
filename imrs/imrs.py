#
# This script will try to build a sample of the input file.
# The purpose of the sample is, get a realistic test file
# that is small enough for iterative development, measures,
# etc., yet big enough to obtain statistically significant
# results.
#
# Usage: imrs_sample.py <input_file> <sampling rate in %> <output_file>
#

import sys
import traceback
import random
import time
import concurrent.futures
import math
import os
from os import listdir
from os.path import isfile, isdir, join


def imrs_parse_one_number(parts, parsed):
    v = 0
    p = parts[parsed].strip()
    v = int(parts[parsed])
    parsed += 1
    return v, parsed

def imrs_parse_one_vector(parts, parsed, v):
    for i in range(0, len(v)):
        v[i],parsed = imrs_parse_one_number(parts, parsed)
    return parsed

class imrs_hyperloglog:
    def __init__(self):
        self.E = 0.0
        self.hllv=[0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0]
        pass;
    def parse(self, parts, parsed):
        self.E = float(parts[parsed].strip())
        parsed += 1
        for i in range(0, len(self.hllv)):
            self.hllv[i], parsed = imrs_parse_one_number(parts,parsed)
        return parsed

class imrs_record:
    def __init__(self):
        self.ip = ""
        self.query_volume = 0
        self.hourly_volume = [ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
        self.daily_volume = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
        self.arpa_count = 0
        self.no_such_domain_queries = 0
        self.no_such_domain_reserved = 0
        self.no_such_domain_frequent = 0
        self.no_such_domain_chromioids = 0
        self.tld_counts = [0,0,0,0,0,0,0,0]
        self.tld_hyperlog = imrs_hyperloglog()
        self.sld_counts = [0,0,0,0,0,0,0,0]
        self.sld_hyperlog = imrs_hyperloglog()
        self.name_parts = [0,0,0,0,0,0,0,0]
        self.rr_types = [0,0,0,0,0,0,0,0]
        self.locales = [0,0,0,0,0,0,0,0]

    def parse_imrs(self, line):
        ok = False
        try:
            parts = line.split(",")
            print(line.strip)
            self.ip = parts[0].strip()
            parsed = 1
            self.query_volume, parsed = imrs_parse_one_number(parts, parsed)
            parsed = imrs_parse_one_vector(parts, parsed, self.hourly_volume)
            parsed = imrs_parse_one_vector(parts, parsed, self.daily_volume)
            print("Arpacount: " + parts[parsed])
            self.arpa_count, parsed = imrs_parse_one_number(parts, parsed)
            print("No such: " + parts[parsed])
            self.no_such_domain_queries, parsed = imrs_parse_one_number(parts, parsed)
            print("Reserved: " + parts[parsed])
            self.no_such_domain_reserved, parsed = imrs_parse_one_number(parts, parsed)
            print("Frequent: " + parts[parsed])
            self.no_such_domain_frequent, parsed = imrs_parse_one_number(parts, parsed)
            print("Chromioids: " + parts[parsed])
            self.no_such_domain_chromioids, parsed = imrs_parse_one_number(parts, parsed)
            parsed = imrs_parse_one_vector(parts, parsed, self.tld_counts)
            print(str(parsed))
            parsed = self.tld_hyperlog.parse(parts, parsed)
            imrs_parse_one_vector(parts, parsed, self.sld_counts)
            parsed = self.sld_hyperlog.parse(parts, parsed)
            parsed = imrs_parse_one_vector(parts, parsed, self.name_parts)
            parsed = imrs_parse_one_vector(parts, parsed, self.rr_types)
            parsed = imrs_parse_one_vector(parts, parsed, self.locales)
            ok = True
        except Exception as e:
            traceback.print_exc()
            print("Cannot parse IMRS Record after " + str(parsed) + " parts:\n" + line.strip()  + "\nException: " + str(e))
        return ok
