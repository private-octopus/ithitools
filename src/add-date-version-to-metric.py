#!/usr/bin/python
#
# This script is expected to run just once, to convert the metrics stored in the ITHI 
# directory to the new format that includes data and version.

import sys
import os
import json
import codecs

from os.path import isfile, join

def ithiwalk(file_list, path):
    print(path)
    for x in os.listdir(path):
        y = join(path, x)
        if isfile(y):
            file_list.append(y)
        else:
            ithiwalk(file_list, y)

def get_date(file_path):
    date=""
    pslash = file_path.split("/")
    if len(pslash) == 1:
        pslash = file_path.split("\\")
    file_name = pslash[len(pslash)-1]
    pdot = file_name.split(".")
    pdash = pdot[0].split("-")
    if len(pdot) == 2 and pdash[0].startswith("M") and \
        pdot[1] == "csv" and len(pdash) == 4 and len(pdash[1]) == 4 and len(pdash[2]) == 2 and len(pdash[3]) == 2 :
        date = pdash[1] + "-" + pdash[2] + "-" + pdash[3]
    else:
        print("Could not find a date in <" + file_path + ">")
    return date

def load_file(file_name):
    m_list = []
    try:
        met_file = codecs.open(file_name, "r", "UTF-8")
        for line in met_file:
            m_list.append(line)
        met_file.close()
        if len(m_list) == 0:
            print("File is empty")
    except Exception as e:
        print("Cannot load <" + file_name + ">" + "\nexception: " + str(e));
    return m_list

def save_file(file_name, m_list, m_date, m_version):
    try:
        saved_file = codecs.open(file_name, "w", "UTF-8")
        for line in m_list:
            cparts = line.split(",")
            n = len(cparts)
            if n == 3 or n == 4:
                line = cparts[0] + "," + m_date + "," + m_version + "," + cparts[1] + "," + cparts[2] + "\n"
            saved_file.write(line);
        saved_file.close()
        return True
    except Exception:
        print("Cannot write <" + file_name + ">");
        return False

def convert_file(file_name, save_name):
    ret = False
    met_date = get_date(file_name)
    if len(met_date) > 0:
        met_list = load_file(file_name)
        if len(met_date) > 0 and len(met_list) > 0:
            if save_file(save_name, met_list, met_date, "v1.06"):
                print("Success, saved "+ save_name + " with date " + met_date)
                ret = True
    return ret

def convert_folder(mypath):
    file_list = []
    ithiwalk(file_list,mypath)
    nb_loaded = 0
    for file_name in file_list:
        # If this is a proper metric file, convert it
        if convert_file(file_name, file_name):
            nb_loaded += 1
    print("In " + mypath + " found " + str(len(file_list)) + " files, loaded " + str(nb_loaded) + " metrics.")

convert_folder(sys.argv[1])
