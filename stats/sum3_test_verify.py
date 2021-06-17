#!/usr/bin/python
# coding=utf-8
#
# Comparison of two "sum3" files for the sum3 CI test.
# The test compares the output of `load_l_root-data` to a reference sum3 file.
# Text comparison is inadequate, because the order of the lines depends on
# the order of files returned by a directory walk, which in turns depends 
# on operating system version and environment. Instead, the procedure
# reads the two file, sorts theresult list in canonical order, and
# compares the values.

import sys
import codecs
import traceback
import m3summary

def check_sum3(candidate_file, reference_file):
    candidate = m3summary.m3summary_list()
    reference = m3summary.m3summary_list()
    ret = False
    if candidate.load_file(candidate_file) and reference.load_file(reference_file):
        nb_candidates = len(candidate.summary_list)
        nb_references = len(reference.summary_list)
        if not nb_candidates == nb_references:
            print("Found " + str(nb_candidates) + " entries in " + candidate_file + ", expected " + str(nb_references))
        else:
            candidate.Sort()
            reference.Sort()
            ret = True
            for i in range(0, nb_candidates):
                if not candidate.summary_list[i] == reference.summary_list[i]:
                    print("Found:\n" + candidate.summary_list[i].to_string() + "\nin " + candidate_file + "\nExpected: " + reference.summary_list[i].to_string())
                    ret = False
                    break
    return ret

# Main
if len(sys.argv) != 3:
    print("Usage: " + sys.argv[0] + " candidate.sum3 reference.sum3")
    exit(-1)
elif check_sum3(sys.argv[1], sys.argv[2]):
    print("File contents match.")
    exit(0)
else:
    print("File contents differ.")
    exit(-1)

