#/usr/bin/python

class tok_instance:
    def __init__(self, instance, count):
        self.i_num =int(instance)
        self.i_count = int(count)

def add_to_list (list, instance, count, list_name):
    "Add the count if the instance is in list, create it otherwise"
    tok = tok_instance(instance, count)
    found_it = 0

    print(list_name + ":");
    for inst in list :
            print("    " +  str(inst.i_num) + ": " + str(inst.i_count) )
            if (inst.i_num == tok.i_num):
                print("    Found" + instance + "(" + str(tok.i_num) + "), adding " + str(tok.i_count) + " to " + str(inst.i_count))
                inst.i_count += tok.i_count
                found_it = 1

    if (found_it == 0):
        print("Adding [" + instance + ", " + count + "] to " + list_name + ".")
        list += [tok]

def write_list_to_file (file, list, list_name):
    "Write one line on the file per instance"
    for inst in list:
        file.write(list_name + ",0," + str(inst.i_num) + "," + str(inst.i_count) + ",\n")

#Main loops
csv = open("tlsa-2018-03-13.csv", "r");

file = open("testfile.txt", "w");

cert_usage = []
selector = []
matching_type = []


print(csv);
print(file);
for line in csv: 
    print(line);
    clean = line.rstrip();
    tokens = clean.split(",");
    if (len(tokens) == 4):
        add_to_list(cert_usage, tokens[1], tokens[0], "DANE_CertUsage")
        add_to_list(selector, tokens[2], tokens[0], "DANE_TlsaSelector")
        add_to_list(matching_type, tokens[3], tokens[0], "DANE_TlsaMatchingType")

write_list_to_file(file, cert_usage, "DANE_CertUsage")
write_list_to_file(file, selector, "DANE_TlsaSelector")
write_list_to_file(file, matching_type, "DANE_TlsaMatchingType")

csv.close();
file.close();
