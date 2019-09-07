#!/usr/bin/python
# coding=utf-8
#
# This scripts tries to estimate the variance of a few M3 test metrics.

import codecs
import sys
import m3summary

if len(sys.argv) < 3:
    print("usage: " + sys.argv[0] + " sum_m3_file.csv dedup.csv")
    ret = -1
else:
    ret = 0

if ret == 0:
    msl = m3summary.m3summary_list()
    ret = msl.load_file(sys.argv[1])
    if ret == 0:
        msl.summary_list.sort()
        sum_file =  codecs.open(sys.argv[2], "w", "UTF-8")
        sum_file.write(m3summary.summary_title_line() + "\n")
        i=0
        nb_dups = 0
        while i < len(msl.summary_list):
            if i == 0 or msl.summary_list[i] != msl.summary_list[i-1]:
                sum_file.write(msl.summary_list[i].to_string() + "\n");
            else:
               nb_dups += 1
            i += 1
        sum_file.close()
        print("Out of " + str(len(msl.summary_list)) + " found " +
             str(nb_dups) + " duplicates.")

exit(ret)


