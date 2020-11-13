#!/usr/bin/env python
# coding=utf-8
#
# Compute the name part distribution from a list of names.
# The output can be used to verify manually that the count extracted by ithitools is correct.
# However, the "name" output only lists erroneous names. Should that change?

import codecs
import sys

# Main loop
if len(sys.argv) != 3:
    print("Usage: " + sys.argv[0] + " name_file counted_name_file \n")
    exit(1)

file_name = sys.argv[1]
counted_name_file = sys.argv[2]

name_parts_count = dict()

with open(counted_name_file, "w") as w:
    w.write("nb_parts,d_count,d_name\n")
    for line in open(file_name, "rt"):
        csv_parts = line.split(",")
        if len(csv_parts) != 4:
            print("Unexpected: " + line.strip())
            continue
        d_name = csv_parts[0].strip()
        t_count = csv_parts[3].strip()
        if d_name == "Name" and t_count == "count":
            continue
        d_count = int(t_count)
        nb_parts = len(d_name.split("."))
        if nb_parts in name_parts_count:
            name_parts_count[nb_parts] += d_count
        else:
            name_parts_count[nb_parts] = d_count
        w.write(str(nb_parts)+","+str(d_count)+","+d_name+"\n")

total = 0
for nb_parts in name_parts_count:
    print(str(nb_parts) + ":" + str(name_parts_count[nb_parts]))
    total += name_parts_count[nb_parts]
print("Total: " + str(total))
