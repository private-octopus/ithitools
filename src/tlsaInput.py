import csv
#/usr/bin/python

class tok_instance:
    def __init__(self, instance, count):
        self.i_num =int(instance)
        self.i_count = int(count)

def add_to_list (list, instance, count):
    "Add the count if the instance is in list, create it otherwise"
    tok = tok_instance(instance, count)
    found_it = 0

    for inst in list :
        if (inst.i_num == tok.i_num):
                inst.i_count += tok.i_count
                found_it = 1

    if (found_it == 0):
        list += [tok]

def write_list_to_file (file, list, list_name):
    "Write one line on the file per instance"
    for inst in list:
        file.write(list_name + ",0," + str(inst.i_num) + "," + str(inst.i_count) + ",\n")

def load_file_data (file_name, cert_usage, selector, matching_type):
    "Load the file content into the tables"
    line_count = 0
    csv = open(file_name, "r")
    for line in csv: 
        clean = line.rstrip();
        tokens = clean.split(",");
        if (len(tokens) == 4):
            add_to_list(cert_usage, tokens[1], tokens[0])
            add_to_list(selector, tokens[2], tokens[0])
            add_to_list(matching_type, tokens[3], tokens[0])
            line_count += int(tokens[0])
    csv.close()
    print("Close file " + file_name  + " after " + str(line_count) + " records")
    return(line_count)


#Main loops
import sys
import glob

cert_usage = []
selector = []
matching_type = []
total_count = 0

pattern=sys.argv[2] + "-*.csv"
print("Pattern matching to <" +  pattern + ">")
name_list = glob.glob(pattern)
if (len(name_list) > 0):
    for name in name_list:
        total_count += load_file_data(name, cert_usage, selector, matching_type)
    file = open(sys.argv[1], "w")
    write_list_to_file(file, cert_usage, "DANE_CertUsage")
    write_list_to_file(file, selector, "DANE_TlsaSelector")
    write_list_to_file(file, matching_type, "DANE_TlsaMatchingType")
    file.write("RR Type,0,52," + str(total_count) + ",\n")

file.close();
