#
# This script will build a list of IP addresses that appear to belong
# to a "server farm". This will be considered true if:
#
# - The IP addresses belong to a single prefix (/24 or /64 for IPv6)
# - The IP addresses are consecutive, i.e., follow each other in the input file.
# - The traffic looks similar, i.e., query volume same +- few % (implemented
#   as 32*delta < volume)
#
# The output file will contain one line per server farm, with the
# total volume being the sum of the server farm. We need to add
# a "farm" property to the list of parameters, maybe as an extension
# argument with default value 1.
#
# Usage: imrs_server_group.py <imrs_file> <output_file>
#

import sys
import traceback
import ipaddress
import os
from os import listdir
from os.path import isfile, isdir, join
import imrs

class server_process:
    def __init__(self, need_output):
        self.nb_server_farms = 0
        self.ips_in_farm = 0
        self.ips_total = 0
        self.queries_from_farm = 0
        self.queries_total = 0
        self.previous_ip = "0.0.0.0"
        self.previous_ipaddr = ipaddress.ip_address(self.previous_ip)
        self.previous_network = ipaddress.ip_network(self.previous_ip + "/24", strict=False)
        self.previous_queries_low = 0
        self.previous_queries_high = 0
        self.previous_line = ""
        self.this_group = 0
        self.this_queries = 0
        self.group_is_parsed = False
        self.group_record = imrs.imrs_record()
        self.is_parsed = False
        self.parsed = imrs.imrs_record()
        self.nb_apnic_found = 0
        self.nb_server_farms_apnic = 0
        self.need_output = need_output

    def continue_with_group(self, queries, line):
        # continuation of previous group
        if self.need_output:
            if not self.group_is_parsed:
                self.group_record = imrs.imrs_record()
                self.group_record.parse_imrs(self.previous_line)
                self.group_is_parsed = True
            if not self.is_parsed:
                self.parsed = imrs.imrs_record()
                self.parsed.parse_imrs(line)
                self.is_parsed = True
            self.group_record.add(self.parsed, is_new_ip=True)
        self.this_group += 1
        self.this_queries += queries

    def finalize_group(self, F):
        if self.this_group < 2:
            # was not a group.
            # If output needed, just copy the previous line
            if self.need_output and self.this_group > 0:
                if self.is_parsed:
                    F.write(self.parsed.to_string() + "\n")
                else:
                    F.write(self.previous_line)
        else:
            # was a group, add group statistics
            self.nb_server_farms += 1
            self.ips_in_farm += self.this_group
            self.queries_from_farm += self.this_queries
            if self.need_output:
                F.write(self.group_record.to_string() + "\n")
                if self.this_queries > 10000 or self.group_record.apnic_count > 1000:
                    print(self.previous_ip + "," + \
                        str(self.this_group) + "," + \
                        str(self.this_queries) + "," + \
                        str(self.group_record.apnic_count))
                if self.group_record.apnic_count > 0:
                    self.nb_server_farms_apnic += 1
            else:
                this_apnic = 0
                if self.is_parsed:
                    this_apnic = self.parsed.apnic_count
                if this_queries > 32 or this_apnic > 0:
                    print(self.previous_ip + ", " \
                       + str(self.this_group) + ", " \
                       + str(self.this_queries) + ", " + str(this_apnic))

    def reset_group(self, ip, ipaddr, network, queries):
        self.previous_ip = ip
        self.previous_ipaddr = ipaddr
        self.previous_network = network
        self.previous_queries_low = queries - (queries >> 5)
        self.previous_queries_high = queries + (queries >> 5)
        self.this_group = 1
        self.this_queries = queries
        self.group_is_parsed = self.is_parsed
        if self.is_parsed:
            self.group_record = self.parsed  

    def load_line(self, line):
        self.is_parsed = False
        ok,ip,queries = imrs.parse_imrs_volume_only(line)
        if not ok:
            print("Bad input: " + line.strip()[0:32] + "...")
            return True
        self.ips_total += 1
        self.queries_total += queries
        ipaddr = ipaddress.ip_address(ip)
        if ipaddr.version < self.previous_ipaddr.version or \
            (ipaddr.version == self.previous_ipaddr.version and \
             ipaddr < self.previous_ipaddr):
            print("Out of order, " + str(ipaddr) + " after " + str(self.previous_ipaddr))
            return False
        suffix = "/24"
        if ip in apnic_dict:
            self.parsed = imrs.imrs_record()
            self.parsed.parse_imrs(line)
            self.parsed.apnic_count = apnic_dict[ip].use_count
            self.is_parsed = True
            self.nb_apnic_found += 1
        if ipaddr.version == 6:
            suffix = "/64"
        network = ipaddress.ip_network(ip + suffix, strict=False)

        if network == self.previous_network and \
           queries >= self.previous_queries_low and \
           queries <= self.previous_queries_high:
            # continuation of previous group
            self.continue_with_group(queries, line)
        else:
            # start of new group. First add statistics.
            self.finalize_group(F)
            # reset for new group
            self.reset_group(ip, ipaddr, network, queries)
        self.previous_line = line
        return True

    def report(self):
        print("server_farms: " + str(self.nb_server_farms))
        print("server_farms with APNIC: " + \
            str(self.nb_server_farms_apnic) + " / " + \
            str(self.nb_apnic_found))
        print("server_ips: " + str(self.ips_in_farm) + " / " + \
            str(self.ips_total))
        print("server_queries: " + str(self.queries_from_farm) + " / " + \
            str(self.queries_total))


# Main
if len(sys.argv) < 3 or len(sys.argv) > 4:
    print("Usage: imrs_server_group.py <imrs_file> <apnic_file> [<output_file>]")

imrs_file = sys.argv[1]
apnic_file = sys.argv[2]
output_file = ""
need_output = False
if len(sys.argv) == 4:
    need_output = True
    output_file = sys.argv[3]

apnic_dict = imrs.apnic_load(apnic_file)

F = sys.stdout
if need_output:
    F = open(output_file, "w")

ctx = server_process(need_output)

for line in open(imrs_file, "r"):
    if not ctx.load_line(line):
        break
# save the last line, or the last group
ctx.finalize_group(F)
# Final report on stdout
ctx.report()

if need_output:
    F.close()


    