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
import os
from os import listdir
from os.path import isfile, isdir, join


# main

if len(sys.argv) != 4:
    print("Usage: imrs_sample.py <input_file> <sampling rate in %> <output_file>")
    exit(1)
file_name_in = sys.argv[1]
rate_text = sys.argv[2]
file_name_out = sys.argv[3]
sampling_rate = 0
if not rate_text.endswith("%"):
    print("sampling rate should be e.g. 5\%, 0.1%, not " + rate_text)
    exit(1)
try:
    rate_percent = float(rate_text[:-1])
    sampling_rate = rate_percent/100.0
except Exception as e:
    traceback.print_exc()
    print("Cannot parse <" + rate_percent  + ">\nException: " + str(e))
    exit(1)

nb_lines_in = 0
nb_lines_out = 0
with open(file_name_out,"wt") as F_OUT:
    for line in open(file_name_in, "rt"):
        nb_lines_in += 1
        if random.random() < sampling_rate:
            F_OUT.write(line)
            nb_lines_out += 1

if nb_lines_in == 0:
    print("Input file " + file_name_in + " is empty.")
else:
    print("Input file " + file_name_in + ": " + str(nb_lines_in) + " lines")
    print("Output file " + file_name_out + ": " + str(nb_lines_out) + " lines")
    print("Sampling rate requested: " + str(sampling_rate))
    print("Sampling rate actual: " + str(nb_lines_out/nb_lines_in))
