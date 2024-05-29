#!/usr/bin/python
import sys
import os
import json
from os import walk
import traceback
#from os import path

class m5_summary:
    def __init__(self):
        self.counters = []
        self.i_count = 0
        self.total = []
        self.key_list = ["M5.1.1", "M5.1.2", "M5.1.3", "M5.1.4", "M5.1.5", "M5.1.6",
           "M5.2.1", "M5.2.2", "M5.2.3", "M5.3.1", "M5.3.2", "M5.4.1",
           "M5.4.2","M5.5", 
           "M5.6.1", "M5.6.2", "M5.6.3", "M5.6.4", "M5.6.5", "M5.6.6", "M5.6.7", "M5.6.8", 
           "M5.6.9", "M5.6.10", "M5.6.11", "M5.6.12", "M5.6.13", "M5.6.14", "M5.6.15", "M5.6.16",
           "M5.7.1", "M5.7.2", "M5.7.9", "M5.7.10"
           ]
        for x in self.key_list:
            self.counters.append(0)
            self.total.append(0.0)

    def add_text_to_summary(self, m5_text):
        m5_data = json.loads(m5_text)
        m5_i0 =  m5_data[0]
        for i in range(0, len(self.key_list)):
             if not self.key_list[i] in m5_i0:
                 print("Could not find: " + self.key_list[i])
                 raise Exception("incomplete list of submetrics.")
        for i in range(0, len(self.key_list)):
            try:
                self.total[i] += m5_i0[self.key_list[i]]
                self.counters[i] += 1
            except:
                pass
            i += 1

    def salvage_summary(self, dirpath, file_list):
        m5_text = "[{"
        has_comma = True
        for file_name in file_list:
            file_path = os.path.join(dirpath, file_name)
            print("Opening " + file_path)
            for line in open(file_path, "r"):
                sline = line.strip()
                # ignore the comment lines 
                if sline.startswith("\"M5."):
                    if not has_comma:
                        m5_text += ","
                    has_comma = sline.endswith(",")
                    m5_text += sline
        m5_text += "}]\n"
        summary.add_text_to_summary(m5_text)
        print("Salvage succeeded!")
    
    def add_to_summary (self, f_name):
        "Parse a JSON file and add the results to the summary"
        print("parsing " + f_name)
        with open(f_name) as m5_file :
            self.i_count += 1
            m5_text = m5_file.read()
            print(m5_text)
            summary.add_text_to_summary(m5_text)
    
    def average(self):
        "Divide the results by the number of summaries counted"
        i = 0
        while i < len(self.key_list) :
            if self.counters[i] > 0:
                self.total[i] /= self.counters[i]
            i += 1
    
    def save_as_csv(self, f_name, date_string):
        "Save the summary as a CSV file"
        csv_file = open(f_name, "w")
        i = 0
        while i < len(self.key_list):
            csv_file.write(self.key_list[i] + "," + date_string + ",v1.06, , " + str(self.total[i]) + ",\n")
            i += 1
        csv_file.close()

# Main 
summary = m5_summary()
mypath = sys.argv[1]
print("Walking: " + mypath)
for (dirpath, dirnames, filenames) in walk(mypath):
    print(dirpath)
    other_files = []
    got_summary = False
    for file_name in filenames :
        print(file_name)
        if (file_name.startswith("m5-")):
            try:
                file_path = os.path.join(dirpath, file_name)
                summary.add_to_summary(file_path)
                got_summary = True
            except Exception as e:
                traceback.print_exc()
                print('\nFile %s generated an exception: %s' % (file_name, str(e)))
                print("Could not extract data from: " + file_path)
        elif (file_name.startswith("m5.")):
            other_files.append(file_name)
    if not got_summary and len(other_files) == 7:
        print("Trying to salvage the results from detail files.")
        try:
            summary.salvage_summary(dirpath, other_files)
        except Exception as e:
            traceback.print_exc()
            print('\nSalvage generated an exception: %s' % (str(e)))
            print("Could not extract data from: " + str(other_files))
        
print("Found " + str(summary.i_count) + " summaries.")
summary.average()
i = 0
checkNull = True
while i < len(summary.key_list) :
    if summary.total[i] > 0:
        checkNull = False
        break
    i += 1
if checkNull:
    print("All values of metric M5 are NULL!")
summary.save_as_csv(sys.argv[2], sys.argv[3])
