#!/usr/bin/python
# coding=utf-8
#
# Build an IP Address to AS number conversion table.
# The input should be the table ip2asn-v4.tsv from  

import sys
import traceback
import ipaddress

class ipdomv4_line:
    def __init__(self):
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
    print("Usage: " + sys.argv[0] + " ip2asn_file + mapping_file")
    exit(1)

ip2asn_file = sys.argv[1]
csv_file = sys.argv[2]
nb_line = 0
nb_duplicates = 0
nb_parse_error = 0
nb_bad_line = 0
nb_ipv4 = 0
nb_ipv6 = 0

initial_list = []

try:
    for line in open(ip2asn_file , "rt", encoding='utf-8'):
        nb_line += 1
        strip_line = line.strip()
        parts = strip_line.split("\t")
        if len(parts) < 3:
            if nb_bad_line  < 10:
                printf("Malformed line: " + strip_line)
            nb_bad_line += 1
        else:
            ip_dv4 = ipdomv4_line()
            try:
                ip_dv4.ip_first = ipaddress.ip_address(parts[0])
                ip_dv4.ip_last = ipaddress.ip_address(parts[1])
                ip_dv4.as_number = int(parts[2])
                if len(initial_list) < 10:
                    print(str(ip_dv4.ip_first) + " ... " + str(ip_dv4.ip_last) + " --> " + str(ip_dv4.as_number))
                initial_list.append(ip_dv4)
                nb_ipv4 += 1        
            except Exception as e:
                traceback.print_exc()
                print("Got exception: " + str(e))
                nb_parse_error += 1
                if nb_parse_error < 10:
                    print("Parse error: <" + line.strip() + ">")
    print("Found " + str(nb_line) + " lines in " + ip2asn_file )
except:
    traceback.print_exc()
    print("Got exception: " + str(e))
    print("Error after " + str(nb_line) + " lines in " + ip2asn_file)

print("Entries: " + str(len(initial_list)))
print("Duplicates: " + str(nb_duplicates))
print("Parse_error: " + str(nb_parse_error))
print("Bad_line: " + str(nb_parse_error))
print("IPv4: " + str(nb_ipv4))

sorted_list = sorted(initial_list)

# display the beginning of the sorted list 

nb_item = 0

for item in sorted_list:
    nb_item += 1
    if nb_item < 20:
         print(str(item.ip_first) + "..." + str(item.ip_last) + " : " + str(item.as_number))
    else:
        break

# Create a compact list by reducing overlapping ranges

compact_list = []
heap = []

for item in sorted_list:
    if len(heap) == 0 or heap[0].ip_first == item.ip_first:
        heap.append(item)
    else:
        # sort the heap so the shorted range is last
        heap = sorted(heap)
        # process the ranges in heap that precede the next item
        while len(heap) > 0 and heap[len(heap) - 1].ip_last < item.ip_first:
            heap_last = heap.pop()
            append_item_to_compact_list(heap_last, compact_list, heap)
        # manage partial overlap with last item
        if len(heap) > 0 and heap[len(heap) - 1].ip_first < item.ip_first:
            added_range = heap[len(heap) - 1]
            added_range.ip_last = item.ip_first - 1       
            append_item_to_compact_list(added_range, compact_list, heap)
        # add current item to the heap
        heap.append(item)

# Add the reminder of the heap to the compacted ranges
while len(heap) > 0:
    heap_last = heap.pop()
    append_item_to_compact_list(heap_last, compact_list, heap)

print("Non overlapping ranges: " + str(len(compact_list)))

# Display the beginning of the list

nb_item = 0

for item in compact_list:
    nb_item += 1
    if nb_item < 10:
         print(str(item.ip_first) + "..." + str(item.ip_last) + " : " + str(item.as_number))
    else:
        break

# save the result file
try:
    nb_saved = 0
    file_out = open(csv_file, "w")
    file_out.write("ip_first, ip_last, as_number,\n")
    for item in compact_list:
        file_out.write(str(item.ip_first) + "," + str(item.ip_last) + "," + str(item.as_number) + ",\n")
        nb_saved += 1
    file_out.close()
    print("Saved " + str(nb_saved) + " ranges to " + csv_file)
except Exception as e:
    traceback.print_exc()
    print("Cannot write <" + csv_file+ ">, error: " + str(e));

