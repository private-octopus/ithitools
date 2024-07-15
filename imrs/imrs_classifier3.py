#
# Exploration of the ipstats file for each network
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
import matplotlib.pyplot as plt
import numpy as np
from sklearn.linear_model import LinearRegression
import random
import imrs_pandas
from imrs_pandas import print_stats, save_stats, save_selected_stats, \
                 plot_or_save, plot_and_explore, example_and_count, \
                 print_names, print_mean


# main
if len(sys.argv) != 2 and len(sys.argv) != 3:
    for x in range(0, len(sys.argv)):
        print(str(x) + ":" + sys.argv[x])
    print("Usage: imrs_classifier.py <imrs_ratio csv file> [<img_folder>]")
    exit(1)
imrs_file = sys.argv[1]
plot_dir = "-"
out_file = sys.stdout
if len(sys.argv) == 3:
    plot_dir = sys.argv[2]
    csv_path = join(plot_dir, "stats-v3.csv")
    out_file = open(csv_path, "w")
    out_file.write("frame, property, count, mean, std, min, c25%, c50%, c75%, max\n")


full_df = imrs_pandas.load_imrs_to_frame(imrs_file)
print("Loaded full")

imrs_pandas.imrs_corrections(full_df)
print("Applied corrections")
explored =  [ 'r_arpa', 'l_tld', 'l_com', 'r_COM', 'r_no_such' ]

tracked = [ "network", "queries", "r_no_such", "h_count", "d_count", "r_arpa", "r_COM", "l10_q", "l_com", "l_tld", "l_sld", "l10_a", "APNIC", "COM"  ]
save_selected_stats(out_file, full_df, tracked, "full_df")
plot_and_explore(full_df, plot_dir, "full", 'queries', explored, lx=True, ly=False)

# First, isolate the "small" nodes, defined
# as sending fewer than 40 queries.
small_df = full_df[full_df["queries"] <= 100]
big_df = full_df[full_df["queries"] > 100]

save_selected_stats(out_file,  small_df, tracked,"small")
save_selected_stats(out_file,  big_df, tracked,"big")
plot_and_explore(small_df, plot_dir, "small", 'queries', explored, lx=True, ly=False)
plot_and_explore(big_df, plot_dir, "big", 'queries', explored, lx=True, ly=False)

# then, create two subsets of the big sites:
# ns_medium: < 1M queries
# ns_many: >= 1M

ns_medium = big_df[big_df['queries'] < 100000 ]
ns_large = big_df[big_df['queries'] >= 100000 ]

save_selected_stats(out_file,  ns_medium, tracked,"medium")
save_selected_stats(out_file,  ns_large, tracked,"large")
plot_and_explore(ns_medium, plot_dir, "medium", 'queries', explored, lx=True, ly=False)
plot_and_explore(ns_large, plot_dir, "large", 'queries', explored, lx=True, ly=False)

if out_file != sys.stdout:
    out_file.close()