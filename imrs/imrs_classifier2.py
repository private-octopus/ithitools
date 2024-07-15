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
    csv_path = join(plot_dir, "stats.csv")
    out_file = open(csv_path, "w")
    out_file.write("frame, property, count, mean, std, min, c25%, c50%, c75%, max\n")


full_df = imrs_pandas.load_imrs_to_frame(imrs_file)
print("Loaded full")

imrs_pandas.imrs_corrections(full_df)
print("Applied corrections")

tracked = [ "network", "queries", "r_no_such", "h_count", "d_count", "r_arpa", "r_COM", "l10_q", "l_com", "l_tld", "l_sld", "l10_a", "APNIC", "COM"  ]
save_selected_stats(out_file, full_df, tracked, "full_df")
plot_and_explore(full_df, plot_dir, "full", 'queries', [ 'r_arpa', 'l_tld', 'l_com', 'r_COM'], lx=True, ly=False)

# First, isolate the "small" nodes, defined
# as sending fewer than 40 queries.
small_df = full_df[full_df["queries"] <= 100]
big_df = full_df[full_df["queries"] > 100]

save_selected_stats(out_file,  small_df, tracked,"small_df")
save_selected_stats(out_file,  big_df, tracked,"big_df")

# then, create three subsets of the big sites:
# ns_low: no-such < 5%
# ns_high: no-such > 90%
# ns_mid: in_between

ns_low = big_df[big_df["r_no_such"] < 0.05]
ns_other = big_df[big_df["r_no_such"] >= 0.05]
ns_high = ns_other[ns_other["r_no_such"] > 0.9]
ns_mid = ns_other[ns_other["r_no_such"] <= 0.9]

save_selected_stats(out_file,  ns_other, tracked,"ns_other")
plot_and_explore(ns_other, plot_dir, "other", 'queries', [ 'r_arpa', 'l_tld', 'l_com', 'r_COM'], lx=True, ly=False)

save_selected_stats(out_file,  ns_low, tracked,"ns_low")
save_selected_stats(out_file,  ns_high, tracked,"ns_high")
save_selected_stats(out_file,  ns_mid, tracked,"ns_mid")

# At this stage, we have separated 4 groups.
# We will ignore the "small" group for now, because in the absence of
# traffic it is hard to classify anything.

plot_and_explore(ns_low, plot_dir, "low", 'queries', [ 'r_arpa', 'l_tld', 'l_com', 'r_COM'], lx=True, ly=False)
plot_and_explore(ns_mid, plot_dir, "mid", 'queries', [ 'r_arpa', 'l_tld', 'l_com', 'r_COM'], lx=True, ly=False)
plot_and_explore(ns_high, plot_dir, "high", 'queries', [ 'r_arpa', 'l_tld', 'l_com', 'r_COM'], lx=True, ly=False)

# In the "low NS" group, the plot of TLDs versus queries shows a break
# at somewhere between 500 and 1000 TLDs seen. Above that line we find
# very few APNIC servers but many large non APNIC nodes. This could
# be nodes engaged in some kind of scanning process.

low_lt500t = ns_low[ns_low["TLDs"] <= 500]
low_gt500t = ns_low[ns_low["TLDs"] > 500]
save_selected_stats(out_file,  low_lt500t, tracked,"low_lt500t")
save_selected_stats(out_file,  low_gt500t, tracked,"low_gt500t")
plot_and_explore(low_lt500t, plot_dir, "low_lt500t", 'queries', [ 'r_arpa', 'l_tld', 'l_com', 'r_COM'], lx=True, ly=False)
plot_and_explore(low_gt500t, plot_dir, "low_gt500t", 'queries', [ 'r_arpa', 'l_tld', 'l_com', 'r_COM'], lx=True, ly=False)

# In the "high NS" group, there seems
# to be two interesting subgroups: more than 500 TLDs, as in the
# "low" case, and more than about 10^6 queries, which separates
# a bunch of high values from the bulk of APNNIC resolvers.

high_lt500t = ns_high[ns_high["TLDs"] <= 500]
high_gt500t = ns_high[ns_high["TLDs"] > 500]
save_selected_stats(out_file,  high_lt500t, tracked,"high_lt500t")
save_selected_stats(out_file,  high_gt500t, tracked,"high_gt500t")
plot_and_explore(high_lt500t, plot_dir, "high_lt500t", 'queries', [ 'r_arpa', 'l_tld', 'l_com', 'r_COM'], lx=True, ly=False)
plot_and_explore(high_gt500t, plot_dir, "high_gt500t", 'queries', [ 'r_arpa', 'l_tld', 'l_com', 'r_COM'], lx=True, ly=False)

# In the "mid" group, the pictures are murky. There seems to be
# a separation between resolvers with more than 1 million
# queries and others. (or is it 100K?)

mid_lt1Mq = ns_mid[ns_mid["queries"] <= 1000000]
mid_gt1Mq = ns_mid[ns_mid["queries"] > 1000000]

save_selected_stats(out_file,  mid_lt1Mq, tracked,"mid_lt1Mq")
save_selected_stats(out_file,  mid_gt1Mq, tracked,"mid_gt1Mq")
plot_and_explore(mid_lt1Mq, plot_dir, "mid_lt1Mq", 'queries', [ 'r_arpa', 'l_tld', 'l_com', 'r_COM'], lx=True, ly=False)
plot_and_explore(mid_gt1Mq, plot_dir, "mid_gt1Mq", 'queries', [ 'r_arpa', 'l_tld', 'l_com', 'r_COM'], lx=True, ly=False)

if out_file != sys.stdout:
    out_file.close()