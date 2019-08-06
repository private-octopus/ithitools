#!/usr/bin/python
# coding=utf-8
#
# This scripts tries to estimate the variance of a few M3 test metrics.

import codecs
import sys
import m3name

m3n1 = m3name.m3name()

ret = m3name.m3name_test()
if ret == 0:
    print("m3name_test passes")
else:
    print("m3name_test fails")
