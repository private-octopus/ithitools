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

def protected_max(x, list):
    mx = 0
    for k in list:
        if x[k] > mx:
            mx = x[k]
    return mx

def protected_min(x, list):
    mx = 0
    for k in list:
        if x[k] < mx:
            mx = x[k]
    return mx

def protected_min_max_ratio(x, list):
    mx = 0
    mn = 99999999
    for k in list:
        if x[k] > mx:
            mx = x[k]
        if x[k] < mn:
            mn = x[k]
    r = 1.0
    if mx > 0:
        r = mn/mx
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


# main
if len(sys.argv) != 2:
    print("Usage: imrs_explore.py <imrs_ratio csv file>")
    exit(1)
imrs_file = sys.argv[1]

full_df = load_imrs_to_frame(imrs_file)
print("Loaded full")
# apply corections for day overlow bug:
# ignore d00, it is always 0
# compute d31 = quries - sum (d01..d30)
# compute arpa = arpa0 - d31
days = [
    "d01", "d02", "d03", "d04", "d05", "d06", "d07", "d08", "d09", "d10", \
    "d11", "d12", "d13", "d14", "d15", "d16", "d17", "d18", "d19", "d20", \
    "d21", "d22", "d23", "d24", "d25", "d26", "d27", "d28", "d29", "d30", \
    "d31" ]
full_df["d31"] = full_df.apply(lambda x: reset_d31(x, days[:-1]), axis=1)
full_df["arpa"] = full_df["arpa0"] - full_df["d31"]

print("Computed corrections")
# add columns for ratios
full_df["r_ns_res"] = full_df.apply(lambda x: protected_ratio(x["ns_res"], x["no_such"]), axis=1)
full_df["r_ns_frq"] = full_df.apply(lambda x: protected_ratio(x["ns_frq"], x["no_such"]), axis=1)
full_df["r_ns_chr"] = full_df.apply(lambda x: protected_ratio(x["ns_chr"], x["no_such"]), axis=1)

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

full_df["n_tlds"] = full_df.apply(lambda x: compute_nb_tlds(x), axis=1)
full_df["n_slds"] = full_df.apply(lambda x: compute_nb_slds(x), axis=1)
print("Computed TLDs, SLDs")

# get a view of only the important columns
r_selected = [ "network", "queries", "arpa", "d31", "r_arpa", "r_no_such", "r_ns_res", "r_ns_frq", "r_ns_chr", "r_COM", "r_INFO", "TLDs", "SLDs", "r_AAAA", "r_NS", "r_PTR", "r_NSEC", "r_SOA", "h_count", "d_count", "n_tlds", "n_slds", "r_APNIC", "APNIC" ]
r_df = full_df[r_selected]

r_df_des = r_df.describe()
print(r_df_des.transpose())
r_df_corr = r_df.corr()
print( r_df_corr)



print("\nAll entries with APNIC > 0:")
with_apnic_df = r_df[r_df["APNIC"] > 0]
# print(narrow_df)
narrow_des = with_apnic_df.describe()
print(narrow_des.transpose())


without_apnic_df = r_df[r_df["APNIC"] <= 0]



#narrow_corr = narrow_df.corr()
#print(narrow_corr)
print("\nAll entries with APNIC > 100:")
big_apnic_df = with_apnic_df[(with_apnic_df["APNIC"]) > 100]
big_apnic_des = big_apnic_df.describe()
print(big_apnic_des.transpose())
big_apnic_corr = big_apnic_df.corr()
print( big_apnic_corr)

print("\nAll entries with APNIC <= 100:")
small_apnic_df = with_apnic_df[(with_apnic_df["APNIC"]) <= 100]
small_apnic_des = small_apnic_df.describe()
print(small_apnic_des.transpose())
small_apnic_corr = small_apnic_df.corr()
print( small_apnic_corr)

axa = without_apnic_df.plot.scatter(x="queries", y="n_slds", alpha=0.5, logx=True, logy=True, color="blue")
small_apnic_df.plot.scatter(ax=axa, x="queries", y="n_slds", alpha=0.5, color="orange")
big_apnic_df.plot.scatter(ax=axa, x="queries", y="n_slds", alpha=0.5, color="red")
plt.show()


print("\nAll entries without APNIC:")
without_apnic_df = r_df[r_df["APNIC"] == 0]
without_apnic_des = without_apnic_df.describe()
print(without_apnic_des.transpose())
without_apnic_corr = without_apnic_df.corr()
print( without_apnic_corr)

print("\nAll entries with queries >= 1000:")
big1000_df = r_df[r_df["queries"] >= 1000]
big1000_des = big1000_df.describe()
print(big1000_des.transpose())
big1000_corr = big1000_df.corr()
print( big1000_corr)

print("\nAll entries with APNIC > 1000:")
apn1000_df = r_df[r_df["APNIC"] >= 1000]
apn1000_des = apn1000_df.describe()
print(apn1000_des.transpose())
apn1000_corr = apn1000_df.corr()
print( apn1000_corr)

print("\nAll entries with COM < 0.0001")
no_com_df = r_df[r_df["r_COM"] < 0.0001]
no_com_des = no_com_df.describe()
print(no_com_des.transpose())
no_com_corr = no_com_df.corr()
print( no_com_corr)

print("\nAll entries with APNIC > 1000 and no_such < 0.0001")
apn1000r_df = apn1000_df[apn1000_df["r_no_such"] < 0.0001]
apn1000r_des = apn1000r_df.describe()
print(apn1000r_des.transpose())
apn1000r_corr = apn1000r_df.corr()
print( apn1000r_corr)

print("\nAll entries with no_such == 0")
ns0_df = r_df[r_df["r_no_such"] == 0 ]
ns0_des = ns0_df.describe()
print(ns0_des.transpose())
ns0_corr = ns0_df.corr()
print( ns0_corr)
