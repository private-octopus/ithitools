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



# Argv[1] = name of the summary file to read.
# argv[2] = name of the evaluation file

m3s = m3summary.m3summary_list()

if not m3s.load_file(sys.argv[1]):
    exit(-1)

m3s.compute_daytime_stats()

m3s.save_for_evaluation(sys.argv[2])