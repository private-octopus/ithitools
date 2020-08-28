#!/usr/bin/python
# coding=utf-8
#
# Extract the values of M3.3.3 "length" from M3 file, and store as CSV.

import sys
import os
from os import listdir
from os.path import isfile, join
import traceback

m3path = sys.argv[1]
m3files = [f for f in listdir(m3path) if isfile(join(m3path, f))]

f_out = open(sys.argv[2],"wt")
s = "date,dga,bad_syntax,numeric,binary,ipv4,"
for i in range(1,64):
    s += "l" + str(i) + ","
s += "\n"
f_out.write(s)

for file_name in m3files:
    file_path = join(m3path, file_name)
    len_tab = []
    for i in range(64):
        len_tab.append("")
    line_date = ""
    bad_syntax = ""
    numeric = ""
    binary = ""
    ipv4 = ""
    for line in open(file_path,"r"):
        parts = line.split(",")
        if parts[0]=="M3.3.3" and len(parts) >= 5:
            line_date = parts[1]
            lp = parts[3].strip()
            if lp.startswith("length_"):
                try:
                    name_len = int(lp[7:])
                    len_tab[name_len] = parts[4].strip()
                except:
                    traceback.print_exc()
                    print("Can't parse: " + line.strip())
            elif lp == "bad_syntax":
                bad_syntax = parts[4].strip()
            elif lp == "numeric":
                numeric = parts[4].strip()
            elif lp == "binary":
                binary = parts[4].strip()
            elif lp == "ipv4":
                ipv4 = parts[4].strip()
               
    dga = 0.0
    for i in range(7,16):
        try:
            dga += float(len_tab[i])
        except:
            print("For " + file_name + " could not convert length[" + str(i) + "]: " + len_tab[i])
    csv_line = line_date + "," + str(dga) + "," + bad_syntax + "," + numeric + "," + binary + "," + ipv4 + ","
    for i in range(1,64):
        csv_line += len_tab[i] + ","
    csv_line += "\n"
    f_out.write(csv_line)
f_out.close()

