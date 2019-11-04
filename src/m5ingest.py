#!/usr/bin/python
import sys
import os
import json
from os import walk
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
           "M5.6.9", "M5.6.10", "M5.6.11", "M5.6.12", "M5.6.13", "M5.6.14", "M5.6.15", "M5.6.16"]
        for x in self.key_list:
            self.counters.append(0)
            self.total.append(0.0)
    
    def add_to_summary (self, f_name):
        "Parse a JSON file and add the results to the summary"
        with open(f_name) as m5_file :
            self.i_count += 1
            m5_text = m5_file.read()
            m5_data = json.loads(m5_text)
            i = 0
            while i < len(self.key_list) :
                try:
                    self.total[i] += m5_data[0][self.key_list[i]]
                    self.counters[i] += 1
                except:
                    pass
                i += 1
            m5_file.close()
    
    def average(self):
        "Divide the results by the number of summaries counted"
        i = 0
        while i < len(self.key_list) :
            if self.counters[i] > 0:
                self.total[i] /= self.counters[i]
            i += 1
    
    def save_as_csv(self, f_name):
        "Save the summary as a CSV file"
        csv_file = open(f_name, "w")
        i = 0
        while i < len(self.key_list):
            csv_file.write(self.key_list[i] + ", , " + str(self.total[i]) + ",\n")
            i += 1
        csv_file.close()

# Main 

summary = m5_summary()
mypath = sys.argv[1]
for (dirpath, dirnames, filenames) in walk(mypath):
    for file_name in filenames :
        if (file_name.startswith("m5-")):
            try:
                print("Found: " + file_name)
                file_path = os.path.join(dirpath, file_name)
                summary.add_to_summary(file_path)
            except:
                print("Could not extract data from: " + file_path)

print("Found " + str(summary.i_count) + " summaries.")
summary.average()
i = 0
while i < len(summary.key_list) :
    print("m5[" + summary.key_list[i] + "] = " + str(summary.total[i]))
    i += 1
summary.save_as_csv(sys.argv[2])
