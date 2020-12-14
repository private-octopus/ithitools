#!/usr/bin/env python
# coding=utf-8
#
# Compute the name part distribution from a list of names.
# The output can be used to verify manually that the count extracted by ithitools is correct.
# However, the "name" output only lists erroneous names. Should that change?

import codecs
import sys

# Main loop
if len(sys.argv) != 2:
    print("Usage: " + sys.argv[0] + " name_file \n")
    exit(1)

file_name = sys.argv[1]

name_parts_count = []
alpha_1_len_count = []
alpha_n_len_count = []
for i in range(0,64):
    name_parts_count.append(0)
    alpha_1_len_count.append(0)
    alpha_n_len_count.append(0)

alpha_tld = dict()
alpha_tld_many_parts = dict()

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
    name_parts = d_name.split(".")
    nb_parts = len(name_parts)
    if nb_parts < 64:
        name_parts_count[nb_parts] += d_count
    tld = name_parts[nb_parts-1].strip()
    if tld.isalpha():
        d_category = csv_parts[2].strip()
        if d_category == "dga" or \
            d_category == "frequent"  or \
            d_category == "jumbo" or \
            d_category == "other":
            if tld in alpha_tld:
                alpha_tld[tld] += d_count
            else:
                alpha_tld[tld] = d_count
            if nb_parts > 1 and not tld in alpha_tld_many_parts:
                alpha_tld_many_parts[tld] = 1

for tld in alpha_tld:
    l = len(tld)
    if l < 64:
        alpha_n_len_count[l] += alpha_tld[tld]
        if alpha_tld[tld] < 3 and not tld in alpha_tld_many_parts:
            alpha_1_len_count[l] += alpha_tld[tld]

print("Breakout by name parts:")
total = 0
for nb_parts in range(0,64):
    if name_parts_count[nb_parts] > 0:
        print(str(nb_parts) + ": " + str(name_parts_count[nb_parts]))
        total += name_parts_count[nb_parts]
print("Total: " + str(total))

print("Alphanum domains by length:")
for l in range(0,64):
    if alpha_n_len_count[l] > 0:
        print(str(l)+ ": " + str(alpha_n_len_count[l]) + ", " + str(alpha_1_len_count[l]))