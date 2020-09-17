#!/usr/bin/python
# coding=utf-8
#
# Build an IP Address to AS number conversion table.
# Start with a BGP table, then produce an ordered list of ip addresses.
# Lookup will load this ordered list and answer the question.
# Get the BGP table from https://bgp.potaroo.net/as6447/bgptable.txt (or other AS)
# or if v6 https://bgp.potaroo.net/as6447/v6/bgptable.txt 
# Format: 1.0.0.0/24 147.28.7.2 3130 2914 13335
#         subnet     next hop    as    as  as (owner of IP)
# sometimes ">" before first line.
# Final result is a table where each line has three items:
#    ip_first: first IP address in range
#    ip_last: last address IP address in range
#    as_number: autonomous system number for that range
# The final table aggregates all consecutive ranges that point to the same AS.

import sys
import traceback
import ipaddress

class ipdom_line:
    def __init__(self):
        self.prefix_length = 0
        self.as_number = 0
        self.duplicates = 0
        self.ip_first = ipaddress.ip_address("0.0.0.0")
        self.ip_last = ipaddress.ip_address("255.255.255.255")

    def compare(self, other):
        if (self.ip_first < other.ip_first):
            return -1
        elif (self.ip_first > other.ip_first):
            return 1
        elif (self.ip_last > other.ip_last):
            return -1
        elif (self.ip_last < other.ip_last):
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

def append_item_to_compact_list(item, compact_list, heap):
    ip_next = item.ip_last + 1
    if item.ip_first < ip_next:
        if len(compact_list) > 0 and \
           item.as_number == compact_list[len(compact_list) -1].as_number and \
           item.ip_first == compact_list[len(compact_list) -1].ip_last + 1:
           compact_list[len(compact_list) -1].ip_last = item.ip_last
        else:
           compact_list.append(item)
        i = 0
        while i < len(heap):
            heap[i].ip_first = ip_next
            i += 1

# Parsing program starts here

if len(sys.argv) != 3:
    print("Usage: " + sys.argv[0] + " bgp_file + mapping_file")
    exit(1)

bgp_file = sys.argv[1]
csv_file = sys.argv[2]
nb_line = 0
nb_malformed = 0
nb_bad_prefix = 0
nb_sup_prefix = 0
nb_duplicates = 0
nb_parse_error = 0
nb_bad_line = 0
nb_ipv4 = 0
nb_ipv6 = 0

ipdom = dict()

try:
    for line in open(bgp_file , "rt"):
        try:
            nb_line += 1
            strip_line = line.strip()
            parts = strip_line.split(" ")
            prefix = parts[0]
            if prefix.startswith('>'):
                prefix_minus = prefix[1:]
                nb_sup_prefix += 1
                if nb_sup_prefix < 10:
                    print("Replaced prefix " + prefix + " by " + prefix_minus)
                prefix = prefix_minus
            prefix_parts = prefix.split("/")
            if len(parts) < 2:
                nb_malformed += 1
            elif len(prefix_parts) != 2:
                nb_bad_prefix += 1
                if nb_bad_prefix < 10:
                    print("Bad prefix: <" + strip_line + "> prefix: <" + prefix + "> prefix parts: " + str(len(prefix_parts))) 
            elif prefix in ipdom:
                nb_duplicates += 1
                ipdom[prefix].duplicates += 1
            else:
                ip_d_l = ipdom_line()
                try:
                    subnet = ipaddress.ip_network(prefix)
                    if subnet.version == 4:
                        nb_ipv4 += 1
                    else:
                        nb_ipv6 += 1
                    ip_d_l.ip_first = subnet.network_address
                    ip_d_l.ip_last = subnet.network_address + subnet.num_addresses - 1
                    if len(ipdom) < 10:
                        print(prefix + " = " + str(ip_d_l.ip_first) + " ... " + str(ip_d_l.ip_last))
                    if parts[len(parts)-1].startswith("{"):
                        ip_d_l.as_number = int(parts[len(parts)-2])
                    else:
                        ip_d_l.as_number = int(parts[len(parts)-1])
                    ipdom[prefix] = ip_d_l
                except Exception as e:
                    traceback.print_exc()
                    print("Got exception: " + str(e))
                    nb_parse_error += 1
                    if nb_parse_error < 10:
                        print("Bad line: <" + line.strip() + ">") 
        except Exception as e:
            traceback.print_exc()
            print("Got exception: " + str(e))
            nb_bad_line += 1
            if nb_bad_line < 10:
                print("Bad line: <" + line.strip() + ">") 
    print("Found " + str(nb_line) + " lines in " + bgp_file )
except:
    print("Error after " + str(nb_line) + " lines in " + bgp_file)

print("Entries: " + str(len(ipdom)))
print("IPv4: " + str(nb_ipv4))
print("IPv6: " + str(nb_ipv6))
print("Malformed: " + str(nb_malformed))
print("Bad_prefix: " + str(nb_bad_prefix))
print("Duplicates: " + str(nb_duplicates))
print("Parse_error: " + str(nb_parse_error))
print("Sup_prefix: " + str(nb_sup_prefix))
print("Bad_line: " + str(nb_parse_error))

sorted_list = sorted(ipdom.values())
nb_item = 0

for item in sorted_list:
    nb_item += 1
    if nb_item < 20:
         print(str(item.ip_first) + "..." + str(item.ip_last) + " : " + str(item.as_number))
    else:
        break

compact_list = []
heap = []
for item in sorted_list:
    if len(heap) == 0 or heap[0].ip_first == item.ip_first:
        heap.append(item)
    else:
        # sort the heap so the shortest range is last
        heap = sorted(heap)
        # process the ranges in heap that precede the next item
        while len(heap) > 0 and heap[len(heap) - 1].ip_first < item.ip_first:
            if heap[len(heap) - 1].ip_last < item.ip_first:
                # no overlap. Just pop and store the last item
                heap_last = heap.pop()
                append_item_to_compact_list(heap_last, compact_list, heap)
            else:
                # manage partial overlap with last item
                n_item = ipdom_line()
                n_item.ip_first = heap[len(heap) - 1].ip_first
                n_item.ip_last = item.ip_first - 1
                n_item.as_number = heap[len(heap) - 1].as_number
                append_item_to_compact_list(n_item, compact_list, heap)
        # add current item to the heap
        heap.append(item)
# Add the reminder of the heap to the ranges
while len(heap) > 0:
    heap_last = heap.pop()
    append_item_to_compact_list(heap_last, compact_list, heap)

print("Non overlapping ranges: " + str(len(compact_list)))
nb_item = 0

for item in compact_list:
    nb_item += 1
    if nb_item < 20:
         print(str(item.ip_first) + "..." + str(item.ip_last) + " : " + str(item.as_number))
    else:
        break

# save the result file
try:
    nb_saved = 0
    file_out = open(csv_file, "w")
    file_out.write("ip_first, ip_last, as_number,\n")
    for item in compact_list:
        if item.ip_first < (item.ip_last + 1):
            file_out.write(str(item.ip_first) + "," + str(item.ip_last) + "," + str(item.as_number) + ",\n")
            nb_saved += 1
    file_out.close()
    print("Saved " + str(nb_saved) + " ranges to " + csv_file)
except Exception as e:
    traceback.print_exc()
    print("Cannot write <" + csv_file+ ">, error: " + str(e));
