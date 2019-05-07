#!/usr/bin/python
import sys
import os
import json
import datetime
from os import walk
from datetime import timedelta

class partner_summary:
    def __init__(self):
        self.key_list = ["M1", "M2", "m3", "nawala", "unlp", "uccgh", "kaznic", "twnic", "tlsa"]
        self.state = [ 0, 0, 0, 0, 0, 0, 0, 0, 0]

    def check_file(self, file_name, key):
        "check whether name in file matches partner and remember key"
        i = 0
        while (i < len(self.key_list)):
            if(file_name.find(self.key_list[i]) != -1):
                self.state[i] |= key
            i += 1;

    def save_as_json(self, f_name, year, month):
        "Save the summary as a CSV file"
        json_file = open(f_name, "w")
        json_file.write("{\n")
        json_file.write("date : \"" + str(year) + "-" + format(month, "02d") + "\",\n")
        json_file.write("year : " + str(year) + ",\n")
        json_file.write("month : " + str(month) + ",\n")
        json_file.write("partners : [\n")
        i = 0
        while (i < len(self.key_list)):
            if(i  > 0):
                json_file.write(",\n")
            m0= self.state[i]&1
            m1 = (self.state[i]>>1)&1
            json_file.write("[\"" + self.key_list[i] + "\"," + str(m0) + "," +  str(m1) + "]")
            i+=1
        json_file.write("]\n")
        json_file.write("}\n")
        json_file.close()

# Main

metric_dirs = [ "M1", "M2" ]
summary = partner_summary()
mypath = sys.argv[1]
ithi = sys.argv[2]
current = datetime.date.today()
previous = datetime.date(current.year, current.month, 1) - timedelta(days=1)
before = datetime.date(previous.year, previous.month, 1) - timedelta(days=1)
current_month = str(current.year) + "-" + format(current.month, "02d")
previous_month = str(previous.year) + "-" + format(previous.month, "02d")
before_month = str(before.year) + "-" + format(before.month, "02d")
print(current_month)
print(previous_month)
tlsa1 = "tlsa-data-" + current_month + ".csv"
tlsa2 = "tlsa-data-" + previous_month + ".csv"

for (dirpath, dirnames, filenames) in walk(mypath):
    for file_name in filenames :
        z = 0
        try :
            z = os.path.getsize(os.path.join(dirpath, file_name))
        except(OSError, IOError):
            z = 0
        if (z > 0):
            if (file_name.endswith("_this_month.txt")):
                summary.check_file(file_name, 1)
            if (file_name.endswith("previous_month.txt")):
                summary.check_file(file_name, 2)
            if (file_name == tlsa1):
                summary.check_file(file_name, 1)
            if (file_name == tlsa2):
                summary.check_file(file_name, 2)

i = 0
while (i < len(metric_dirs)):
    m_dir = ithi + "/" + metric_dirs[i]
    for (dirpath, dirnames,filenames) in walk(m_dir):
        for file_name in filenames :
            z = 0
            try :
                z = os.path.getsize(os.path.join(dirpath, file_name))
            except(OSError, IOError):
                z = 0
            if (z > 0):
                if(file_name.find(previous_month) != -1):
                    summary.check_file(file_name, 1)
                if (file_name.find(before_month) != -1):
                    summary.check_file(file_name, 2)
    i+=1

summary.save_as_json(sys.argv[3], current.year, current.month)
