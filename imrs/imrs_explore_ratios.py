#
# Exploration of the "ratios" reported for each network, as
# computed in "imrs_ratios.py"
#
# Usage: imrs_explore.py <input_file> 
#

import sys
import traceback
import random
import time
import concurrent.futures
import math
import os
from os import listdir
from os.path import isfile, isdir, join
import imrs
from imrs import parse_imrs_volume_only, apnic_record
import pandas as pd

# main
if len(sys.argv) != 2:
    print("Usage: imrs_explore.py <imrs_ratio csv file>")
    exit(1)
imrs_ratio_file = sys.argv[1]

full_df = pd.read_csv(imrs_ratio_file)
with_apnic_df = full_df[full_df["APNIC"] > 0]

selected_columns = [ "network", "queries", "arpa", "no_such", "ns_res", "ns_frq", "ns_chr", "COM", "TLDs", "SLDs", "NS", "SOA", "NSEC3", "APNIC" ]
narrow_df = with_apnic_df[selected_columns]
print(narrow_df)
narrow_corr = narrow_df.corr()
print(narrow_corr)

big_apnic_df = narrow_df[(narrow_df["queries"]/narrow_df["APNIC"]) > 100]
print(big_apnic_df)
big_apnic_corr = big_apnic_df.corr()
print( big_apnic_corr)

small_apnic_df = narrow_df[(narrow_df["queries"]/narrow_df["APNIC"]) <= 100]
print(small_apnic_df)
small_apnic_corr = small_apnic_df.corr()
print( small_apnic_corr)