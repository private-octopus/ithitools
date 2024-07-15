#
# Common functions for parsing the imrs CSV files 
# in panda format.
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

def save_stats(f, x_df, name):
    x_des = x_df.describe()
    # print(x_des.transpose())
    for h in x_des:
        c = x_des[h]
        l = c.transpose()
        f.write(name + ", " + str(h))
        for w in l:
            f.write(", " + str(w))
        f.write("\n")

def save_selected_stats(out_file,  df, selection, name):
    df_view = df[selection]
    save_stats(out_file, df_view, name)

def print_stats(x_df, name):
    print(name)
    x_des = x_df.describe()
    print(x_des.transpose())
    if 'queries' in x_df.columns:
        print("Queries " + str(x_df['queries'].sum()))
    if 'APNIC' in x_df.columns:
        dfa = x_df[x_df['APNIC']>0] 
        print("APNIC   " + str(dfa['APNIC'].sum()) + ", "  + str(dfa['queries'].sum()) + ", " + str(dfa['APNIC'].count()))
    # x_cor = x_df.corr()
    # print(x_cor)


def plot_or_save(plot_dir, image_name):
    if plot_dir == "-":
        plt.show()
    else:
        image_path = join(plot_dir, image_name)
        plt.savefig(image_path)
        plt.close(fig="all")

def plot_and_explore(df, plot_dir, name, x_key, y_keys, lx=False, ly=False):
    dfa = df[ df['APNIC'] == 0 ]
    dfo = df[ df['APNIC'] != 0 ]
    for y_key in y_keys:
        axa = dfo.plot.scatter(x=x_key, y=y_key, alpha=0.5, logx=lx, logy=ly, color="blue")
        dfa.plot.scatter(ax=axa, x=x_key, y=y_key, alpha=0.5, color="orange")
        plot_or_save(plot_dir, name +"-"+x_key+"-"+y_key+".jpg")

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

def print_names(df):
    s = ""
    for n in list(df):
        if n != "network":
            s += n + ", "
    print(s)

def print_mean(df, name, n_list):
    s = name + ", "
    for n in n_list:
        if n != "network":
            s += str(df[n].median()) + ", "
    print(s)


def imrs_corrections(full_df): 
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
    full_df["l_com"] = np.log10(2*full_df["COM"] + 1)
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