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
from imrs_pandas import print_stats, plot_or_save, example_and_count, print_names, print_mean

def compute_l10_sa(x, y, n, intercept):
    d = float(intercept)
    for i in range(len(y)):
        d += float(x[n[i]]*y[i])
    return d

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

full_df = imrs_pandas.load_imrs_to_frame(imrs_file)
print("Loaded full")

imrs_pandas.imrs_corrections(full_df)
print("Applied corrections")

# First, study the APNIC Data
# get APNIC subset
apnic_df = full_df[full_df["l10_a"] > 0]
apnic_selected = [ "network", "queries", "r_no_such", "h_count", "d_count", "r_arpa", "r_COM", "l10_q", "l_com", "l_tld", "l_sld", "l10_a", "APNIC", "COM"  ]

# select 4 subsets, based on 2 variables
apnic_loneg_df = apnic_df[apnic_df["r_no_such"] < 0.1]
apnic_hineg_df = apnic_df[apnic_df["r_no_such"] >= 0.1]

apnic_loneg_loap_df = apnic_loneg_df[apnic_loneg_df["r_good_apnic"] < 300]
apnic_loneg_hiap_df = apnic_loneg_df[apnic_loneg_df["r_good_apnic"] >= 300]
apnic_hineg_loap_df = apnic_hineg_df[apnic_hineg_df["r_good_apnic"] < 300]
apnic_hineg_hiap_df = apnic_hineg_df[apnic_hineg_df["r_good_apnic"] >= 300]

print_names(apnic_loneg_loap_df)
print_mean(apnic_loneg_loap_df,"apnic_loneg_loap_df", apnic_selected)
print_mean(apnic_loneg_hiap_df,"apnic_loneg_hiap_df", apnic_selected)
print_mean(apnic_hineg_loap_df,"apnic_hineg_loap_df", apnic_selected)
print_mean(apnic_hineg_hiap_df,"apnic_hineg_hiap_df", apnic_selected)


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

axtld = apnic_df.plot.scatter(x="APNIC", y="TLDs", alpha=0.5, logx=True, logy=False, color="blue")
plot_or_save(plot_dir, "tlds-apnic.jpg")
axcom = apnic_df.plot.scatter(x="APNIC", y="COM", alpha=0.5, logx=True, logy=False, color="blue")
plot_or_save(plot_dir, "com-apnic.jpg")
axnosuch = full_df.plot.scatter(x="queries", y="r_no_such", alpha=0.5, logx=True, logy=False, color="blue")
apnic_df.plot.scatter(ax=axnosuch, x="queries", y="r_no_such", alpha=0.5, color="orange")
plot_or_save(plot_dir, "no_such-queries.jpg")

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
notap_df = full_df[full_df["l10_a"] == 0]

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

print_names(notap_loneg_loap_df)
print_mean(notap_loneg_loap_df,"notap_loneg_loap_df", apnic_selected)
print_mean(notap_loneg_hiap_df,"notap_loneg_hiap_df", apnic_selected)
print_mean(notap_hineg_loap_df,"notap_hineg_loap_df", apnic_selected)
print_mean(notap_hineg_hiap_df,"notap_hineg_hiap_df", apnic_selected)