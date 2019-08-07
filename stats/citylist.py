#!/usr/bin/python
# coding=utf-8
#
# This scripts tries to estimate the variance of a few M3 test metrics.

import codecs
import sys

# Main program
# Load a list of files from argv[1], and for each file compute the
# list of metrics and their contribution to variances. Then,
# compute the final values of the metrics and variances.

if len(sys.argv) != 3:
    print("Usage: " + argv[0] + " smummary_m3_file.csv city_file.csv")
    exit(-1)

sum_m3 = codecs.open(sys.argv[1], "r", "UTF-8")
city_out = codecs.open(sys.argv[2], "w", "UTF-8")
city_list = []

for line in sum_m3:
    cells = line.split(",")
    if len(cells) > 3 and (cells[1] != "CC" or cells[2] != "City"):
        city_name = cells[1] + "-" + cells[2]
        city_list.append(city_name)

sum_m3.close()

city_list.sort()
i = 0

while i < len(city_list):
    if i == 0 or city_list[i] != city_list[i-1]:
        city_out.write(city_list[i] + "\n")
    i += 1
city_out.close()

        
