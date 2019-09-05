
#!/usr/bin/python
# coding=utf-8
#
# This scripts tries to estimate the variance of a few M3 test metrics.

import codecs
import sys
import m3summary

# By default, we expect to test with the file "data/tiny_capture.csv, which has 589 lines.
if len(sys.argv) < 4:
    print("usage: " + sys.argv[0] + " projection sum_m3_file.csv projected.csv")
    ret = -1
else:
    ret = 0

if ret == 0:
    if sys.argv[1] == "country":
        projection = m3summary.projection.country
    elif sys.argv[1] == "city":
        projection = m3summary.projection.city
    elif sys.argv[1] == "country-day":
        projection = m3summary.projection.country_day
    elif sys.argv[1] == "country-weekday":
        projection = m3summary.projection.country_weekday
    elif sys.argv[1] == "country-hour":
        projection = m3summary.projection.country_hour
    else:
        print("Incorrect parameter: " + sys.argv[1])
        print("projection should be one of: country, city, country-day, country-weekday, country-hour")

if ret == 0:
    msl = m3summary.m3summary_list()
    ret = msl.load_file(sys.argv[2])
    if ret == 0:
        p = msl.project(projection)
        psl = m3summary.m3summary_list()
        psl.summary_list = p
        ret = psl.save_file(sys.argv[3])

if ret == 0:
    print("Success!")

exit(ret)
