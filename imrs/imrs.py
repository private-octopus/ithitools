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
import math

# Just process the first argument in a "line", when working fast.
def parse_imrs_volume_only(line):
    ok = False
    ip = ""
    count = 0
    try:
        parts = line.split(",")
        ip = parts[0].strip()
        count = int(parts[1].strip())
        ok = True
    except Exception as e:
        traceback.print_exc()
        print("Cannot parse IMRS Record:\n" + line.strip()  + "\nException: " + str(e))
    return ok, ip, count

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

def imrs_add_one_vector(v1, v2):
    for i in range(0, len(v1)):
        v1[i] += v2[i]

def imrs_vector_to_string(v):
    s = ""
    for i in range(0, len(v)):
        s += str(v[i]) + ","
    return s

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
    def assess(self):
        # First, compute the "indicator" of the m=16 registers
        # Z = 1 / sum (1 / 2^hll[j])
        # The indicator is NOT the same as the harmonic mean. The relation is:
        # harmonic_mean = m * Z
        divider = 0.0;
        for j in range(0, len(self.hllv)):
            divider += 1.0 / (1 << self.hllv[j]);
        Z = 1.0 / divider;
        # Then, compute E using a precomputed coefficient to correct the bias of the formula
        # E = alpha_m * (m^2) * 
        # For m = 16, use the precomputed value alpha_m = 0.673
        # 0.673 * (16^2) = 0.673*256 = 172.288
        self.E = Z * 172.288;

        # For small values (E < (5/2)*m , use Linear counting
        if self.E < 40.0:
            V = 0
            for j in range(0, len(self.hllv)):
                if self.hllv[j] == 0:
                    V += 1
            if V > 0:
                self.E = 16 * math.log(16.0 / V)

    def add(self, other):
        for i in range(0, len(self.hllv)):
            if self.hllv[i] < other.hllv[i]:
                self.hllv[i] = other.hllv[i]
        self.assess()

    def to_string(self):
        s = str(self.E)+","
        for i in range(0, len(self.hllv)):
            s += str(self.hllv[i])+","
        return s

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
        self.apnic_count = 0
        self.server_count = 1

    def parse_imrs(self, line):
        ok = False
        try:
            parts = line.split(",")
            self.ip = parts[0].strip()
            parsed = 1
            self.query_volume, parsed = imrs_parse_one_number(parts, parsed)
            parsed = imrs_parse_one_vector(parts, parsed, self.hourly_volume)
            parsed = imrs_parse_one_vector(parts, parsed, self.daily_volume)
            self.arpa_count, parsed = imrs_parse_one_number(parts, parsed)
            self.no_such_domain_queries, parsed = imrs_parse_one_number(parts, parsed)
            self.no_such_domain_reserved, parsed = imrs_parse_one_number(parts, parsed)
            self.no_such_domain_frequent, parsed = imrs_parse_one_number(parts, parsed)
            self.no_such_domain_chromioids, parsed = imrs_parse_one_number(parts, parsed)
            parsed = imrs_parse_one_vector(parts, parsed, self.tld_counts)
            parsed = self.tld_hyperlog.parse(parts, parsed)
            parsed = imrs_parse_one_vector(parts, parsed, self.sld_counts)
            parsed = self.sld_hyperlog.parse(parts, parsed)
            parsed = imrs_parse_one_vector(parts, parsed, self.name_parts)
            parsed = imrs_parse_one_vector(parts, parsed, self.rr_types)
            parsed = imrs_parse_one_vector(parts, parsed, self.locales)
            if parsed < len(parts):
                self.apnic_count = imrs_parse_one_number(parts, parsed)
            if parsed < len(parts):
                self.server_count = imrs_parse_one_number(parts, parsed)
            ok = True
        except Exception as e:
            traceback.print_exc()
            print("Cannot parse IMRS Record after " + str(parsed) + " parts:\n" + line.strip()  + "\nException: " + str(e))
        return ok

    def parse_volume_only(self, line):
        ok = False
        try:
            parts = line.split(",")
            self.ip = parts[0].strip()
            parsed = 1
            self.query_volume, parsed = imrs_parse_one_number(parts, parsed)
            ok = True
        except Exception as e:
            traceback.print_exc()
            print("Cannot parse IMRS Record after " + str(parsed) + " parts:\n" + line.strip()  + "\nException: " + str(e))
        return ok

    def add(self, other, is_new_ip=False):
        self.query_volume += other.query_volume
        imrs_add_one_vector(self.hourly_volume, other.hourly_volume)
        imrs_add_one_vector(self.daily_volume, other.daily_volume)
        self.arpa_count += other.arpa_count
        self.no_such_domain_queries += other.no_such_domain_queries
        self.no_such_domain_reserved += other.no_such_domain_reserved
        self.no_such_domain_frequent += other.no_such_domain_frequent
        self.no_such_domain_chromioids += other.no_such_domain_chromioids
        imrs_add_one_vector(self.tld_counts, other.tld_counts)
        self.tld_hyperlog.add(other.tld_hyperlog)
        imrs_add_one_vector(self.sld_counts, other.sld_counts)
        self.sld_hyperlog.add(other.sld_hyperlog)
        imrs_add_one_vector(self.name_parts, other.name_parts)
        imrs_add_one_vector(self.rr_types, other.rr_types)
        imrs_add_one_vector(self.locales, other.locales)
        if is_new_ip:
            self.server_count += other.server_count

    def to_string(self):
        s =""
        s += self.ip + ","
        s += str(self.query_volume) + ","
        s += imrs_vector_to_string(self.hourly_volume)
        s += imrs_vector_to_string(self.daily_volume)
        s += str(self.arpa_count) + ","
        s += str(self.no_such_domain_queries) + ","
        s += str(self.no_such_domain_reserved) + ","
        s += str(self.no_such_domain_frequent) + ","
        s += str(self.no_such_domain_chromioids) + ","
        s += imrs_vector_to_string(self.tld_counts)
        s += self.tld_hyperlog.to_string()
        s += imrs_vector_to_string(self.sld_counts)
        s += self.sld_hyperlog.to_string()
        s += imrs_vector_to_string(self.name_parts)
        s += imrs_vector_to_string(self.rr_types)
        s += imrs_vector_to_string(self.locales)
        s += str(self.apnic_count) + ","
        s += str(self.server_count) + ","
        return s

class apnic_record:
    def __init__(self):
        self.ip = ""
        self.use_count = 0
        self.seen_in_imrs = False
        self.imrs_count = 0

    def parse(self, line):
        parts = line.split(",")
        nb_parts = len(parts)
        if nb_parts >= 4:
            try:
                self.ip = parts[0].strip()
                self.use_count = int(parts[3].strip())
            except Exception as e:
                traceback.print_exc()
                print("Cannot parse APNIC Record:\n" + line.strip()  + "\nException: " + str(e))
                return False
        return True

def apnic_load(apnic_file):
    apnic_dict = dict()
    for line in open(apnic_file,"r"):
        apnic = apnic_record()
        if apnic.parse(line):
            apnic_dict[apnic.ip] = apnic
    return apnic_dict