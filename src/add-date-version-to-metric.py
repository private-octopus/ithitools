#!/usr/bin/python
import sys
import os
import json
import codecs

def get_date(file_path):
    date=""
    pslash = file_path.split("/")
    if len(pslash) == 1:
        pslash = file_path.split("\\")
    file_name = pslash[len(pslash)-1]
    pdot = file_name.split(".")
    pdash = pdot[0].split("-")
    if len(pdash) == 4:
        date = pdash[1] + "-" + pdash[2] + "-" + pdash[3]
    else:
        print("Could not find a date in <" + file_path + ">")
        print("File name: " + file_name)
        print("pdot[0]: " + pdot[0])
        print("pdash[" + str(len(pdash)) + "], pdot[0] = " + pdot[0])
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


file_name = sys.argv[1]
if len(sys.argv) > 2:
    save_name =  sys.argv[2]
else:
    save_name = file_name

met_date = get_date(file_name)
met_list = load_file(file_name)
if len(met_date) > 0 and len(met_list) > 0:
    if save_file(save_name, met_list, met_date, "v1.06"):
        print("Success, saved "+ save_name + " with date " + met_date)