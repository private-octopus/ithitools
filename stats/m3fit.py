#!/usr/bin/python
# coding=utf-8
#
# This scripts tries to estimate the variance of a few M3 test metrics.

import codecs
import sys
import datetime
import math
import m3name
import captures
import m3summary
import os
from os.path import isfile, join

def hour_slice(hour):
    slice = 0.0
    try:
        h_part = hour.split(':')
        hours = int(h_part[0])
        minutes = int(h_part[1])
        seconds = int(h_part[2])
        slice = seconds/300 + minutes/5 + hours*12
    except Exception as e:
        print("Cannot parse hour: " + hour + " Exception: " + str(e))
    return slice

def day_slice(day, hour):
    slice = hour_slice(hour)
    try:
        d_part = day.split('-')
        d = int(d_part[2])
        slice += 24*12*(d - 1)
    except Exception as e:
        print("Cannot parse day: " + day + " Exception: " + str(e))
    return slice

def add_slice(day, hour, v, d, sum_v, sum_d):
    slice = day_slice(day, hour)
    i_slice = int(slice)
    while i_slice >= len(sum_v):
        sum_v.append(0.0)
    while i_slice >= len(sum_d):
        sum_d.append(0.0)
    frac = slice - i_slice
    if (i_slice > 0 and frac > 0.0):
        d_v = frac*v
        d_d = frac*d
        sum_v[i_slice -1] += d_v
        sum_d[i_slice-1] += d_d
        v -= d_v
        d -= d_d
    sum_v[i_slice] += v
    sum_d[i_slice] += d

def smooth_curve(sum_v, l):
    smooth = []
    smoothed = 0.0
    l1 = int(l/2)
    l2 = l - l1 - 1
    smoothed += sum_v[0]*(l1 + 1)
    i = 0
    while i < l2:
        if i < len(sum_v):
            smoothed += sum_v[i]
        else:
            smoothed += sum_v[len(sum_v) - 1]
        i = i + 1
    i = 0
    while i < len(sum_v):
        smooth.append(smoothed/l)
        if i >= l1:
            smoothed -= sum_v[i-l1]
        else:
            smoothed -= sum_v[0]
        if i + l2 >= len(sum_v): 
            smoothed += sum_v[len(sum_v) - 1]
        else:
            smoothed += sum_v[i+l2]
        i += 1
    return smooth

def find_min_slice_index(smooth):
    i = 0
    i_min = 0
    ave_min = smooth[0]
    while i < 24*12:
        n = 0
        v = 0
        while n < 31:
            j = i + 24*12*n
            if j >= len(smooth):
                break
            v += smooth[j]
            n += 1
        if n > 0:
            ave = v / n
            if ave < ave_min:
                i_min = i
                ave_min = ave
        else:
            print ("For i = " + str(i) + " n <= 0")
        i += 1
    return i_min

def save_smooth(orig, sum_v, smooth, average, day_time_avg, day_time_stdev, file_name):
    try:
        sum_file = codecs.open(file_name, "w", "UTF-8")
        sum_file.write("orig, aligned, smoothed, day-average, average, stdev\n")
        i = 0
        while i < len(sum_v):
            sum_file.write(str(orig[i]) + ',' + str(sum_v[i]) + "," + str(smooth[i]) + "," +
                          str(average[i]) + "," + str(day_time_avg[i]) + "," + str(day_time_stdev) + "\n")
            i += 1
        sum_file.close()
        return True
    except Exception:
        print("Cannot write <" + file_name + ">");
        return False



# Open a summary file and find the best fit for peaks and valleys

tape = m3summary.m3summary_list()

if tape.load_file(sys.argv[1]) < 0:
    print("Cannot load <" + sys.argv[1] + ">")
    exit(-1)

sum_s = []
sum_d = []
orig = []

for s3 in tape.summary_list:
    i_slice = int(day_slice(s3.date, s3.hour))
    while len(orig) <= i_slice:
        orig.append(0.0)
    orig[i_slice] = s3.nb_queries
    add_slice(s3.date, s3.hour, s3.nb_queries, s3.duration, sum_s, sum_d)

while len(orig) < len(sum_s):
    orig.append(0.0)

i = 0
while i < len(sum_s):
    if (sum_d[i] > 0 and sum_d[i] != 300):
        sum_s[i] /= sum_d[i]
        sum_s[i] *= 300
    i += 1

smooth = smooth_curve(sum_s,25)

i_min = find_min_slice_index(smooth)

print("Low i = " + str(i_min)) 

nb_days = 0
day_average = []
day_time_x2 = 0.0
while len(day_average) < len(sum_s):
    day_average.append(0.0)
i = 0
i_night = i_min - 4*12
if i_night < 0:
    i_night += 24*12
i_min = i_night - 16*12
if i < i_min:
    i = i_min
while i < len(sum_s):
    day_tot = 0
    day_nb = 0
    i_0 = i
    if i_night >= len(sum_s):
        i_night = len(sum_s) -1;
    while i <= i_night:
        day_tot += sum_s[i]
        day_time_x2 += sum_s[i]*sum_s[i]
        day_nb += 1
        i += 1
    nb_days += 1
    day_av = day_tot/day_nb
    while i_0 < i_night:
        day_average[i_0] = day_av
        i_0 += 1
    i_night += 24*12
    i = i_night - 16*12

print("Found " + str(nb_days) + " days.")

day_time_avg = []
day_time_tot = 0.0
nb_day_time_tot = 0
for x in day_average:
    if x > 0.0:
        nb_day_time_tot += 1
        day_time_tot += x

day_time_average = day_time_tot / nb_day_time_tot
day_time_variance = day_time_x2 / nb_day_time_tot - day_time_average*day_time_average
day_time_stdev = math.sqrt(day_time_variance)

i = 0
while i < len(day_average):
    if day_average[i] == 0.0:
        day_time_avg.append(0.0)
    else:
        day_time_avg.append(day_time_average)
    i += 1

if not save_smooth(orig, sum_s, smooth, day_average, day_time_avg, day_time_stdev, sys.argv[2]):
    exit(-1)
    

