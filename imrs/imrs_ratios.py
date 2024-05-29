#
# This script processs the "networks" file, extract the
# networks that have at least one APNIC match, and creates
# a table of ratios to explore the correlation between
# various parameters and the "total queries/apnic queries"
# ratios.
#
# We assume that the APNIC queries count is an indication
# of the number of "real" users served by the server group.
# Logically, the total number of APNIC queries should be
# proportional to the number of users served.

#
# Usage: imrs_ratios <ipstats file>
#

import sys
import traceback
import ipaddress
import os
from os import listdir
from os.path import isfile, isdir, join
import imrs

# Main
if len(sys.argv) != 3:
    print("Usage: imrs_ratios.py <stats_file> <output_file>")
    exit(1)
stats_file = sys.argv[1]
output_file = sys.argv[2]

with open(output_file,"w") as F:
    F.write("network,queries," + imrs.imrs_record.ratio_headers() + "\n")
    for line in open(stats_file,"r"):
        rec = imrs.imrs_record()
        if rec.parse_imrs(line):
            ratios = rec.ratios()
            F.write(rec.ip + "," + str(rec.query_volume) + ",")
            for ratio in ratios:
                F.write(str(ratio) + ",")
            F.write("\n")


