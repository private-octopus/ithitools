#!/usr/bin/python
# coding=utf-8
#
# This scripts tries to estimate the variance of a few M3 test metrics.

import codecs
import sys
import m3name
import captures
import m3summary
import os
from os.path import isfile, join
import traceback

m3_outlier_cat = ["useful", "non_cached", "dga", "jumbo", "other"]

def outlier_title_line():
    s = "address_id,"
    s += "cc,"
    s += "city,"
    s += "date,"
    s += "hour,"
    s += "duration,"
    s += "nb_queries,"
    s += "nb_nx_domains,"
    s += "ref_average,"
    s += "ref_stdev,"
    s += "ref_q3,"
    s += "ref_iqd,"
    s += "query_ratio,"
    s += "stdev_ratio,"
    s += "iqd_ratio,"
    s += "excess_cat_index,"
    s += "excess_cat_name"
    i = 0
    while i < len(m3_outlier_cat):
        s += "," + m3_outlier_cat[i]
        i += 1
    return s

class m3_outlier:
    def __init__(self):
        self.nb_outliers = 0
        self.address_id = ""
        self.cc = ""
        self.city = ""
        self.date = ""
        self.hour = ""
        self.duration = 0
        self.nb_queries = 0
        self.nb_nx_domains = 0
        self.ref_average = 0.0
        self.ref_stdev = 0.0
        self.ref_q3 = 0.0
        self.ref_iqd = 0.0
        self.query_ratio = 0.0
        self.stdev_ratio = 0.0
        self.iqd_ratio = 0.0
        self.excess_cat_index = 0
        self.excess_cat_name = "useful"
        self.cat_v = [0.0, 0.0, 0.0, 0.0, 0.0]

    def compare(self, other):
        if self.query_ratio < other.query_ratio:
            return 1
        elif self.query_ratio > other.query_ratio:
            return -1
        elif self.stdev_ratio < other.stdev_ratio:
            return 1
        elif self.stdev_ratio > other.stdev_ratio:
            return -1
        elif self.iqd_ratio < other.iqd_ratio:
            return 1
        elif self.iqd_ratio > other.iqd_ratio:
            return -1
        elif self.nb_queries < other.nb_queries:
            return 1
        elif self.nb_queries > other.nb_queries:
            return -1
        elif self.nb_nx_domains< other.nb_nx_domains:
            return 1
        elif self.nb_nx_domains > other.nb_nx_domains:
            return -1
        elif self.cc < other.cc:
            return -1
        elif self.cc > other.cc:
            return 1
        elif self.city < other.city:
            return -1
        elif self.city > other.city:
            return 1
        elif self.date < other.date:
            return -1
        elif self.date > other.date:
            return 1
        elif self.date < other.date:
            return -1
        elif self.hour > other.hour:
            return -1
        elif self.hour < other.hour:
            return 1
        elif self.date > other.date:
            return -1
        elif self.date < other.date:
            return 1
        else:
            return 0

    def __lt__(self, other):
        return self.compare(other) < 0
    def __gt__(self, other):
        return self.compare(other) > 0
    def __eq__(self, other):
        return self.compare(other) == 0
    def __le__(self, other):
        return self.compare(other) <= 0
    def __ge__(self, other):
        return self.compare(other) >= 0
    def __ne__(self, other):
        return self.compare(other) != 0

    def to_string(self):
        s = ""
        s += str(self.address_id) + ","
        s += str(self.cc) + ","
        s += str(self.city) + ","
        s += str(self.date) + ","
        s += str(self.hour) + ","
        s += str(self.duration) + ","
        s += str(self.nb_queries) + ","
        s += str(self.nb_nx_domains) + ","
        s += str(self.ref_average) + ","
        s += str(self.ref_stdev) + ","
        s += str(self.ref_q3) + ","
        s += str(self.ref_iqd) + ","
        s += str(self.query_ratio) + ","
        s += str(self.stdev_ratio) + ","
        s += str(self.iqd_ratio) + ","
        s += str(self.excess_cat_index) + ","
        s += self.excess_cat_name
        i = 0
        while i < len(m3_outlier_cat):
            s += "," + str(self.cat_v[i]) 
            i += 1
        return s

def m3_outlier_by_node(outlier):
    key = outlier.cc + "," + outlier.city + "," + outlier.address_id + "," + outlier.date + "," + outlier.hour
    return key

class m3_outlier_list:
    def __init__(self):
        self.outlier_list = []
        self.is_sorted = False

    def add_m3_summary(m3s):
        pass

    def add_m3_summary_file(self, file_name):
        m3s = m3summary.m3summary_list()
        ret = m3s.load_file(file_name)
        if ret:
            m3s.compute_daytime_stats()
            limit = m3s.day_time_average + 3*m3s.day_time_stdev
            if limit < 100000.0:
                limit = 100000.0
            if limit < 2*m3s.day_time_average:
                limit = 2*m3s.day_time_average
            for summary in m3s.summary_list:
                if summary.nb_queries > limit:
                    outlier = m3_outlier()
                    outlier.address_id = summary.address_id
                    outlier.cc = summary.cc
                    outlier.city = summary.city
                    outlier.date = summary.date
                    outlier.hour = summary.hour
                    outlier.duration = summary.duration
                    outlier.nb_queries = summary.nb_queries
                    outlier.nb_nx_domains = summary.nb_nx_domains
                    outlier.ref_average = m3s.day_time_average
                    outlier.ref_stdev = m3s.day_time_stdev
                    outlier.ref_q3 = m3s.day_time_q3
                    outlier.ref_iqd = m3s.day_time_iqd
                    if outlier.ref_average > 0:
                        outlier.query_ratio = (outlier.nb_queries - outlier.ref_average)/outlier.ref_average
                    if outlier.ref_stdev > 0:
                        outlier.stdev_ratio = (outlier.nb_queries - outlier.ref_average)/outlier.ref_stdev
                    if outlier.ref_iqd > 0:
                        outlier.iqd_ratio = (outlier.nb_queries - outlier.ref_q3)/outlier.ref_iqd
                    outlier.cat_v = [ summary.nb_useful, summary.nb_useless, \
                        summary.dga, summary.jumbo, summary.nb_nx_others]
                    outlier.excess_cat_index = 0
                    outlier.excess_cat_name = "useful"
                    v_max = 0
                    i = 0
                    while i < len(outlier.cat_v):
                        if outlier.cat_v[i] > v_max:
                            outlier.excess_cat_index = i
                            outlier.excess_cat_name = m3_outlier_cat[i]
                            v_max = outlier.cat_v[i]
                        i += 1
                    self.outlier_list.append(outlier)
                    self.is_sorted = False
        return ret

    def Sort(self):
        if not self.is_sorted:
            self.outlier_list = sorted(self.outlier_list)
            self.is_sorted = True

    def save_file(self, file_name):
        try:
            outlier_file = codecs.open(file_name, "w", "UTF-8")
            outlier_file.write(outlier_title_line() + "\n")
            for outlier in self.outlier_list:
                outlier_file.write(outlier.to_string() + "\n");
            outlier_file.close()
            return True
        except Exception as e:
            traceback.print_exc()
            print("Cannot open <" + file_name + ">, error:" + str(e));
            return False

    def sort_by_city(self):
        self.outlier_list.sort(key=m3_outlier_by_node)
        is_sorted = False

    def project_by_city(self, file_name):
        city_list = []
        self.sort_by_city()
        city_out = m3_outlier()
        i = 0
        while i < len(self.outlier_list):
            x = self.outlier_list[i]
            if city_out.cc != x.cc or city_out.city != x.city or city_out.address_id != x.address_id:
                if city_out.nb_outliers > 0:
                    city_list.append(city_out)
                city_out = m3_outlier()
                city_out = x
                city_out.nb_outliers = 1
            elif city_out.nb_queries < x.nb_queries:
                n = city_out.nb_outliers
                city_out = x
                city_out.nb_outliers = n + 1
            else:
                city_out.nb_outliers += 1
            i += 1
        if city_out.nb_outliers > 0:
            city_list.append(city_out)

        print("Saving " + str(len(city_list)) + " cities.")
        try:
            city_file = codecs.open(file_name, "w", "UTF-8")
            city_file.write("nb_outliers," + outlier_title_line() + "\n")
            for outlier in city_list:
                city_file.write(str(outlier.nb_outliers) + "," + outlier.to_string() + "\n");
            city_file.close()
        except Exception as e:
            traceback.print_exc()
            print("Cannot write <" + file_name + ">, error:" + str(e));

        return city_list

# Load all the summaries in specified folder, and extract outliers .

my_sum3_folder = sys.argv[1]
outlier_file = sys.argv[2]
print("Finding outliers in: " + my_sum3_folder)
file_list = os.listdir(my_sum3_folder)
print("Found: " + str(len(file_list)) + " files.")
outliers = m3_outlier_list()
n = 0
nf = 0
for file_name in file_list:
    file_path = join(my_sum3_folder, file_name)
    if outliers.add_m3_summary_file(file_path):
        n += 1
    nf += 1
    if (nf % 10) == 0:
        print("Processed " + str(nf) + " files, found " + str(n) + " m3 summary files, " + str(len(outliers.outlier_list)) + " outliers.")
print("Processed " + str(n) + " m3 summary files, found " + str(len(outliers.outlier_list)) + " outliers.")
outliers.sort_by_city()
if outliers.save_file(outlier_file):
    print("Saved " + str(len(outliers.outlier_list)) + " outliers in <" + outlier_file + ">")

city_list = outliers.project_by_city(sys.argv[3])



