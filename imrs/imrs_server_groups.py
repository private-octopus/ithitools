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

# Main
if len(sys.argv) < 2 or len(sys.argv) > 3:
    print("Usage: imrs_server_group.py <imrs_file> [<output_file>]")

imrs_file = sys.argv[1]
output_file = ""
need_output = False
if len(sys.argv) == 3:
    need_output = True
    output_file = sys.argv[2]

nb_server_farms = 0
ips_in_farm = 0
ips_total = 0
queries_from_farm = 0
queries_total = 0

F = sys.stdout
if need_output:
    F = open(output_file, w)

previous_ip = "0.0.0.0"
previous_ipaddr = ipaddress.ip_address(previous_ip)
previous_network = ipaddress.ip_network(previous_ip + "/24", strict=False)
previous_queries_low = 0
previous_queries_high = 0
this_group = 0
this_queries = 0
this_parsed = imrs.imrs_record()
for line in open(imrs_file, "r"):
    ok,ip,queries = imrs.parse_imrs_volume_only(line)
    if not ok:
        print("Bad input: " + line.strip()[0:32] + "...")
        continue
    ips_total += 1
    queries_total += queries
    ipaddr = ipaddress.ip_address(ip)
    if ipaddr.version < previous_ipaddr.version or \
        (ipaddr.version == previous_ipaddr.version and \
         ipaddr < previous_ipaddr):
        print("Out of order, " + str(ipaddr) + " after " + str(previous_ipaddr))
        break
    suffix = "/24"
    if ipaddr.version == 6:
        suffix = "/64"
    network = ipaddress.ip_network(ip + suffix, strict=False)

    if network == previous_network and \
       queries >= previous_queries_low and \
       queries <= previous_queries_high:
        # continuation of previous group
        if (queries > 16):
            print("match: " + ip + ", " + previous_ip + suffix + ", " + str(previous_queries_low) + \
                " <= " + str(queries) + " <= " + str(previous_queries_high))
        this_group += 1
        this_queries += queries
    else:
        # start of new group. First add statistics.
        if this_group < 2:
            # was not a group
            pass
        else:
            # was a group, add group statistics
            nb_server_farms += 1
            ips_in_farm += this_group
            queries_from_farm += this_queries
            if this_queries > 32:
                print(previous_ip + "," + str(this_group) + "," + str(this_queries))
        # reset for new group
        previous_ip = ip
        previous_ipaddr = ipaddr
        previous_network = network
        previous_queries_low = queries - (queries >> 5)
        previous_queries_high = queries + (queries >> 5)
        this_group = 1
        this_queries = queries
        if need_output:
            this_parsed = imrs.imrs_record()
            this_parsed.parse_imrs(line)

print("server_farms: " + str(nb_server_farms))
print("server_ips: " + str(ips_in_farm) + " / " + str(ips_total))
print("server_queries: " + str(queries_from_farm) + " / " + str(queries_total))


if need_output:
    F.close()


    