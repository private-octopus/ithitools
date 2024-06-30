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

def file_has_header(imrs_file):
    has_header = False
    # get the first line
    line = ""
    for line in open(imrs_file, "r"):
        break
    if len(line) > 0:
        parts = line.split(",")
        if len(parts) > 1:
            try:
                queries = int(parts[1])
                print("No header for " + imrs_file + ": " + parts[0] + ", " + parts[1] + ", ...") 
                has_header = False
            except:
                print("header for " + imrs_file + ": " + parts[0] + ", " + parts[1] + ", ...") 
                has_header = True
    return has_header

def load_imrs_to_frame(imrs_file):
    if file_has_header(imrs_file):
        df = pd.read_csv(imrs_ratio_file)
    else:
        df = pd.read_csv(imrs_file, header=None, names=imrs.imrs_headers, dtype={"network": str}, index_col = False)
    return df

def protected_ratio(v, d):
    r = 0
    if d > 0:
        r = v/d
    return r

def protected_count(x, r, list):
    s = r
    if r > 0:
        mx = 0
        for k in list:
            if x[k] > mx:
                mx = x[k]
        s = r*mx
    count = 0
    for k in list:
        if x[k] > s:
            count += 1
    return count

def reset_d31(x, list):
    s = 0
    for k in list:
        s += x[k]
    d31 = x["queries"] - s
    if d31 < 0:
        d31 = 0
    return d31

def compute_nb_tlds(x):
    tld_count = 0
    for tld in [ "COM", "NET", "ORG", "INFO", "CN", "IN", "DE", "US" ]:
        if x[tld] > 0:
            tld_count += 1
    tld_count += int(x["TLDs"])
    return tld_count

def compute_nb_slds(x):
    sld_count = 0
    for sld in [ "RESOLVER", "EC2", "CLOUD", "WPAD", "CORP", "MAIL", "_TCP", "PROD" ]:
        if x[sld] > 0:
            sld_count += 1
    sld_count += int(x["SLDs"])
    if sld_count < 1:
        sld_count = 1
    return sld_count

def compute_l10_sa(x, y, n, intercept):
    d = float(intercept)
    for i in range(len(y)):
        d += float(x[n[i]]*y[i])
    return d

def print_stats(x_df, name):
    print(name)
    x_des = x_df.describe()
    print(x_des.transpose())
    x_cor = x_df.corr()
    print(x_cor)

def plot_or_save(plot_dir, image_name):
    if plot_dir == "-":
        plt.show()
    else:
        image_path = join(plot_dir, image_name)
        plt.savefig(image_path)

def example_and_count(df, name):
    count = df.shape[0]
    all_rows = df.shape[1]
    queries = 0
    network = ""
    sample = df.sample(13)
    nb_rows = sample.shape[0]
    # print(name + ": samples = " + str(nb_rows) + ", out of " + str(all_rows))
    sdp = sample[["network", "queries"]]
    sdp_np = sdp.to_numpy()
    # print("Sample shape: " + str(sdp.shape))
    # print("Sdp_np shape: " + str(np.shape(sdp_np)))
    for i in range(np.shape(sdp_np)[0]):
        if sdp_np[i,1] > queries:
            queries = sdp_np[i,1]
            network = str(sdp_np[i,0])
    print(name + ": count=" + str(count) + ", network=" + network)
    return count, network

# main
if len(sys.argv) != 2 and len(sys.argv) != 3:
    for x in range(0, len(sys.argv)):
        print(str(x) + ":" + sys.argv[x])
    print("Usage: imrs_classifier.py <imrs_ratio csv file> [<img_folder>]")
    exit(1)
imrs_file = sys.argv[1]
plot_dir = "-"
if len(sys.argv) == 3:
    plot_dir = sys.argv[2]

full_df = load_imrs_to_frame(imrs_file)
print("Loaded full")
# apply corections for day overlow bug:
# ignore d00, it is always 0
# compute d31 = queries - sum (d01..d30)
# compute arpa = arpa0 - d31
days = [
    "d01", "d02", "d03", "d04", "d05", "d06", "d07", "d08", "d09", "d10", \
    "d11", "d12", "d13", "d14", "d15", "d16", "d17", "d18", "d19", "d20", \
    "d21", "d22", "d23", "d24", "d25", "d26", "d27", "d28", "d29", "d30", \
    "d31" ]
full_df["d31"] = full_df.apply(lambda x: reset_d31(x, days[:-1]), axis=1)
full_df["arpa"] = full_df["arpa0"] - full_df["d31"]

print("Computed corrections")
# compute the good column
full_df["good"] = full_df.apply(lambda x: x["queries"] - x["no_such"], axis=1)
# compute the ratio of good over APNIC
full_df["r_good_apnic"] = full_df.apply(lambda x: protected_ratio(x["good"], x["APNIC"]), axis=1)

# compute log10 column of queries and apnic
full_df["l10_q"] = np.log10(full_df["queries"])
full_df["l10_a"] = np.log10(2*full_df["APNIC"] + 1)
full_df["l10_g"] = np.log10(2*full_df["good"] + 1)
full_df["l_tld"] = np.log10(2*full_df["TLDs"] + 1)
full_df["l_sld"] = np.log10(2*full_df["SLDs"] + 1)
# add columns for ratios
for d in [ "no_such", "AAAA", "NS", "PTR", "NSEC", "SOA", "APNIC" ]:
    r_d = "r_" + d
    full_df[r_d] = full_df[d] / full_df["queries"]

full_df["r_arpa"] = full_df["arpa"] / (2*full_df["queries"])

full_df["r_COM"] = full_df.apply(lambda x: protected_ratio(x["COM"], x["queries"] - x["no_such"]), axis=1)
full_df["r_INFO"] = full_df.apply(lambda x: protected_ratio(x["INFO"], x["queries"] - x["no_such"]), axis=1)
print("Computed ratios")

hours = ["h00", "h01", "h02", "h03", "h04", "h05", "h06", "h07", "h08", "h09", \
    "h10", "h11", "h12", "h13", "h14", "h15", "h16", "h17", "h18", "h19", \
    "h20", "h21", "h22", "h23" ]

full_df["h_count"] = full_df.apply(lambda x: protected_count(x, 0, hours), axis=1)
full_df["d_count"] = full_df.apply(lambda x: protected_count(x, 0, days), axis=1)
print("Computed hours")

# First, study the APNIC Data
# get APNIC subset
apnic_df = full_df[full_df["l10_a"] > 0]

# select 4 subsets, based on 2 variables
apnic_loneg_df = apnic_df[apnic_df["r_no_such"] < 0.1]
apnic_hineg_df = apnic_df[apnic_df["r_no_such"] >= 0.1]

apnic_loneg_loap_df = apnic_loneg_df[apnic_loneg_df["r_good_apnic"] < 300]
apnic_loneg_hiap_df = apnic_loneg_df[apnic_loneg_df["r_good_apnic"] >= 300]
apnic_hineg_loap_df = apnic_hineg_df[apnic_hineg_df["r_good_apnic"] < 300]
apnic_hineg_hiap_df = apnic_hineg_df[apnic_hineg_df["r_good_apnic"] >= 300]

# select 4 subsets, based on 2 variables
apnic_loneg_df = apnic_df[apnic_df["r_no_such"] < 0.1]
apnic_hineg_df = apnic_df[apnic_df["r_no_such"] >= 0.1]

apnic_loneg_loap_df = apnic_loneg_df[apnic_loneg_df["r_good_apnic"] < 300]
apnic_loneg_hiap_df = apnic_loneg_df[apnic_loneg_df["r_good_apnic"] >= 300]
apnic_hineg_loap_df = apnic_hineg_df[apnic_hineg_df["r_good_apnic"] < 300]
apnic_hineg_hiap_df = apnic_hineg_df[apnic_hineg_df["r_good_apnic"] >= 300]

# plot four quadrants
axa = apnic_loneg_loap_df.plot.scatter(x="r_no_such", y="r_good_apnic", alpha=0.5, logx=False, logy=True, color="blue")
apnic_loneg_hiap_df.plot.scatter(ax=axa, x="r_no_such", y="r_good_apnic", alpha=0.5, color="green")
apnic_hineg_loap_df.plot.scatter(ax=axa, x="r_no_such", y="r_good_apnic", alpha=0.5, color="orange")
apnic_hineg_hiap_df.plot.scatter(ax=axa, x="r_no_such", y="r_good_apnic", alpha=0.5, color="red")
plot_or_save(plot_dir, "apnic-quadrants.jpg")

# plot APNIC/Queries
axb = apnic_loneg_loap_df.plot.scatter(x="queries", y="APNIC", alpha=0.5, logx=True, logy=True, color="blue")
apnic_loneg_hiap_df.plot.scatter(ax=axb, x="queries", y="APNIC", alpha=0.5, color="green")
apnic_hineg_loap_df.plot.scatter(ax=axb, x="queries", y="APNIC", alpha=0.5, color="orange")
apnic_hineg_hiap_df.plot.scatter(ax=axb, x="queries", y="APNIC", alpha=0.5, color="red")
plot_or_save(plot_dir, "apnic-queries.jpg")

# plot APNIC/Queries/no_such
axb = apnic_loneg_loap_df.plot.scatter(x="queries", y="r_no_such", alpha=0.5, logx=True, logy=False, color="blue")
apnic_loneg_hiap_df.plot.scatter(ax=axb, x="queries", y="r_no_such", alpha=0.5, color="green")
apnic_hineg_loap_df.plot.scatter(ax=axb, x="queries", y="r_no_such", alpha=0.5, color="orange")
apnic_hineg_hiap_df.plot.scatter(ax=axb, x="queries", y="r_no_such", alpha=0.5, color="red")
plot_or_save(plot_dir, "apnic-queries-no_such.jpg")

example_and_count(apnic_loneg_loap_df, "apnic_loneg_loap_df (blue)")
example_and_count(apnic_loneg_hiap_df, "apnic_loneg_hiap_df (green)")
example_and_count(apnic_hineg_loap_df, "apnic_hineg_loap_df (orange)")
example_and_count(apnic_hineg_hiap_df, "apnic_hineg_hiap_df (red)")

# study the APNIC correlations
# get a view of only the important columns
apnic_selected = [ "network", "l10_q", "r_no_such", "h_count", "d_count", "r_arpa", "r_COM", "l_tld", "l_sld", "l10_a" ]
full_selected_df = full_df[apnic_selected]
apnic_selected_df = full_selected_df[full_selected_df["l10_a"] > 0]

# compute a linear regression to explain l10_a from other coefficients
apnic_coeffs = [ "l10_q", "r_no_such", "h_count", "d_count", "r_arpa", "r_COM", "l_tld", "l_sld", "l10_a" ]
full_data_df = full_df[apnic_coeffs]
apnic_data_df = full_data_df[full_selected_df["l10_a"] > 0]
data = apnic_data_df.to_numpy()
#print("data: " + str(np.shape(data)))
#print("data.T: " + str(np.shape(data.T)))


X, y = data.T[:-1], data.T[-1:]
#print("X: " + str(np.shape(X)))
#print("y: " + str(np.shape(y)))
lr = LinearRegression()
lr.fit(X.T, y.T)
print("Linear Coefficients:")
print(lr.coef_)
print(lr.intercept_)
full_df["l10_sa"] = full_df.apply(lambda x: compute_l10_sa(x, lr.coef_.T, apnic_coeffs[:-1], lr.intercept_[0]), axis=1)
full_df["l10_gsa"] = full_df.apply(lambda x: x["l10_g"] - x["l10_sa"], axis=1)
#print(list(full_df))
apnic_coeffs_x = [ "network", "l10_sa", "l10_gsa", "l10_q", "r_no_such", "l10_a", "queries" ]
full_data_x_df = full_df[apnic_coeffs_x]
# print(list(full_data_x_df))
# print("shape: " + str(full_data_x_df.shape))
# print("types: " + str(full_data_x_df.dtypes))
apnic_data_x_df = full_data_x_df[full_data_x_df["l10_a"] > 0]
# print(list(apnic_data_x_df))
# print("shape: " + str(apnic_data_x_df.shape))
axp = apnic_data_x_df.plot.scatter(x="l10_a", y="l10_gsa", alpha=0.5, logx=False, logy=False, color="blue")
plot_or_save(plot_dir, "l10_a-l10_sa.jpg")
#print_stats(apnic_data_x_df["l10_sa", "l10_a"], "apnic_df")
#print_stats(full_df["l10_sa", "l10_a"], "full_df")

# apply regression to classify not APNIC traffic
notap_df = full_data_x_df[full_data_x_df["l10_a"] == 0]

axp = notap_df.plot.scatter(x="queries", y="r_no_such", alpha=0.1, logx=True, logy=False, color="blue")
apnic_df.plot.scatter(ax=axp, x="queries", y="r_no_such", alpha=0.2, color="orange")
plot_or_save(plot_dir, "apnic-and-notap-queries.jpg")

#print("shape: " + str(notap_df.shape))
#print("types: " + str(notap_df.dtypes))
#print_stats(notap_df, "not_ap")

notap_loneg_df = notap_df[notap_df["r_no_such"] < 0.1]
notap_hineg_df = notap_df[notap_df["r_no_such"] >= 0.1]
#print("shape: " + str(notap_hineg_df.shape))
#print("types: " + str(notap_hineg_df.dtypes))
#print_stats(notap_hineg_df, "notap_hineg_df")

notap_loneg_loap_df = notap_loneg_df[notap_loneg_df["l10_gsa"] < 2.5]
notap_loneg_hiap_df = notap_loneg_df[notap_loneg_df["l10_gsa"] >=  2.5]
notap_hineg_loap_df = notap_hineg_df[notap_hineg_df["l10_gsa"] <  2.5]
notap_hineg_hiap_df = notap_hineg_df[notap_hineg_df["l10_gsa"] >=  2.5]

# plot four quadrants
axc = notap_loneg_loap_df.plot.scatter(x="r_no_such", y="l10_gsa", alpha=0.5, logx=False, logy=False, color="blue")
notap_loneg_hiap_df.plot.scatter(ax=axc, x="r_no_such", y="l10_gsa", alpha=0.5, color="green")
notap_hineg_loap_df.plot.scatter(ax=axc, x="r_no_such", y="l10_gsa", alpha=0.5, color="orange")
notap_hineg_hiap_df.plot.scatter(ax=axc, x="r_no_such", y="l10_gsa", alpha=0.5, color="red")
plot_or_save(plot_dir, "notap-quadrants.jpg")

# plot Not APNIC/Queries
axd = notap_loneg_loap_df.plot.scatter(x="queries", y="l10_sa", alpha=0.5, logx=True, logy=False, color="blue")
notap_loneg_hiap_df.plot.scatter(ax=axd, x="queries", y="l10_sa", alpha=0.5, color="green")
notap_hineg_loap_df.plot.scatter(ax=axd, x="queries", y="l10_sa", alpha=0.5, color="orange")
notap_hineg_hiap_df.plot.scatter(ax=axd, x="queries", y="l10_sa", alpha=0.5, color="red")
plot_or_save(plot_dir, "notap-queries-l10_sa.jpg")

axd = notap_loneg_loap_df.plot.scatter(x="queries", y="r_no_such", alpha=0.5, logx=True, logy=False, color="blue")
notap_loneg_hiap_df.plot.scatter(ax=axd, x="queries", y="r_no_such", alpha=0.5, color="green")
notap_hineg_loap_df.plot.scatter(ax=axd, x="queries", y="r_no_such", alpha=0.5, color="orange")
notap_hineg_hiap_df.plot.scatter(ax=axd, x="queries", y="r_no_such", alpha=0.5, color="red")
plot_or_save(plot_dir, "notap-queries-no_such.jpg")

example_and_count(notap_loneg_loap_df, "notap_loneg_loap_df (blue)")
example_and_count(notap_loneg_hiap_df, "notap_loneg_hiap_df (green)")
example_and_count(notap_hineg_loap_df, "notap_hineg_loap_df (orange)")
example_and_count(notap_hineg_hiap_df, "notap_hineg_hiap_df (red)")