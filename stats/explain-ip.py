
#!/usr/bin/python
# coding=utf-8
#
# This script reformats a list of leaked names
import codecs
import sys
import anomdns

class ip_line:
    def __init__(self):
        self.x_type = 0
        self.ip = 0
        self.total = 0
        self.ip_total = 0

    def file_line(self, line):
        csv = line.split(",")
        lcsv = len(csv)
        if (lcsv >= 4 and csv[0] == "ADDRESS_LIST"):
            self.total = int(csv[3])
            tokens = csv[2].split("/")
            l = len(tokens)
            if (l > 0):
                self.ip = tokens[0]
            if (l > 1):
                self.x_type = int(tokens[1])

    def to_string(self):
        l_str = str(self.ip_total)  + "," + self.ip + "," + str(self.total) + ","  + str(self.x_type)
        return l_str

    def getKey(self):
        return self.to_string()

    def myComp(self, other):
        if (self.ip_total < other.ip_total):
            return -1
        elif (self.ip_total > other.ip_total):
            return 1
        elif (self.ip < other.ip):
            return -1
        elif (self.ip > other.ip):
            return 1
        elif (self.total < other.total):
            return -1
        elif (self.total > other.total):
            return 1
        elif (self.x_type < other.x_type):
            return -1
        elif (self.x_type > other.x_type):
            return 1
        else:
            return 0
    
    def __lt__(self, other):
        return self.myComp(other) < 0
    def __gt__(self, other):
        return self.myComp(other) > 0
    def __eq__(self, other):
        return self.myComp(other) == 0
    def __le__(self, other):
        return self.myComp(other) <= 0
    def __ge__(self, other):
        return self.myComp(other) >= 0
    def __ne__(self, other):
        return self.myComp(other) != 0


class IPFrequent:
    def __init__(self):
        self.ip = ""

    def load(self, addr):
        self.ip = addr

        return 1


    def myComp(self, other):
        if (self.ip < other.ip):
            return -1
        elif (self.ip > other.ip):
            return 1
        else:
            return 0

    def __lt__(self, other):
        return self.myComp(other) < 0
    def __gt__(self, other):
        return self.myComp(other) > 0
    def __eq__(self, other):
        return self.myComp(other) == 0
    def __le__(self, other):
        return self.myComp(other) <= 0
    def __ge__(self, other):
        return self.myComp(other) >= 0
    def __ne__(self, other):
        return self.myComp(other) != 0

class IPFrequentList:
    def __init__(self):
        self.ip_list = []
        
    def find_target(self, target, debug_on):
        found = -1
        if (len(self.ip_list) > 0):
            low = 0
            high = len(self.ip_list)-1
            if (self.ip_list[low] == target):
                found = low
            elif (self.ip_list[high] == target):
                found = high
            elif (target > self.ip_list[low] and target < self.ip_list[high]):
                while (low + 2 <= high):
                    mid = int((low+high)/2)
                    if (self.ip_list[mid] == target):
                        found = mid
                        break;
                    elif (self.ip_list[mid] > target):
                        high = mid
                    else:
                        low = mid
            if (debug_on > 0 and found < 0):
                if (target < self.ip_list[low]):
                    print (target.addr + " < " + self.ip_list[low].addr)
                elif (target > self.ip_list[high]):
                    print (target.addr + " > " + self.ip_list[high].addr)
                else:
                    print ("Search error for : " + target.addr)
                    print ("Low [" + str(low) + "]" + self.ip_list[low].addr + " - " + self.ip_list[low].str_mask())
                    print ("High [" + str(high) + "]" + self.ip_list[high].addr + " - " + self.ip_list[high].str_mask())
        return found

    def find(self, addr, debug_on):
        found = -1
        target = IPFrequent()
        if (target.load(addr)):
            found = self.find_target(target, debug_on)
        return found

    def load(self, csvFile):
        csv_in = codecs.open(csvFile, "r", "UTF-8")
        for line in csv_in:
            column=line.split(",")
            if (len(column) >= 1):
                ip_entry = IPFrequent()
                if (ip_entry.load(column[0]) > 0):
                    if (ip_list.find_target(ip_entry, 0) < 0):
                        self.ip_list.append(ip_entry)
        csv_in.close()
        self.ip_list.sort()

# Main loop


if len(sys.argv) != 5:
    print("Usage: " + sys.argv[0] + " <input-file> <resolvers-by-ip.csv> <raw-output.csv> <table-output.csv>\n")
    exit(1)

anonymize = 1
anom = anomdns.anonymizer()
anom.set_key("ithi-privacy")

ip_list = IPFrequentList()
print("Opening: " + sys.argv[2])
ip_list.load(sys.argv[2])

list = []
file = codecs.open(sys.argv[1], "r", "UTF-8")

for line in file:
    try:
        line = line.rstrip()
        i_line = ip_line()
        i_line.file_line(line)
        if (i_line.total > 0):
            if (ip_list.find(i_line.ip, 0) < 0):
                i_line.ip = anom.anonymizeAddress(i_line.ip)           
            list.append(i_line)
    except:
        e = sys.exc_info()[0]
        print ( "Error" + str(e) + "\n")
        pass

file.close()

list.sort()

file_out = codecs.open(sys.argv[3], "w", "UTF-8")
file_out.write("ip_total , ip , total , x_type,\n")
for i_line in list:
    file_out.write(i_line.to_string() + "\n")
file_out.close()

l = len(list)
i = 0
start_ip = 0
nb_ip = 0
total = 0

while ( i < l):
    total += list[i].total
    nb_ip += list[i].total
    if ((i+1 >= l) or (list[i].ip != list[i+1].ip)):
        while (start_ip <= i):
            list[start_ip].ip_total = nb_ip
            start_ip += 1
        nb_ip = 0
    i += 1

list.sort(reverse=True)

file_out = codecs.open(sys.argv[4], "w", "UTF-8")
file_out.write("IP, frequent, total, good, binary, bad-syntax, ip, numeric, rfc6761, one-part, many-parts,\n");
i=0
l=len(list)
xt = [0, 0, 0, 0, 0, 0, 0, 0]
while ( i < l):
    xt[list[i].x_type] = list[i].total
    if ((i+1 >= l) or (list[i].ip != list[i+1].ip)):
        is_frequent = 0
        if (ip_list.find(list[i].ip, 0) >= 0):
            is_frequent = 1
        file_out.write( list[i].ip + "," + str(is_frequent) + "," + str(list[i].ip_total) + ",")
        j = 0
        while (j < 8):
            file_out.write(str(xt[j]) + ",")
            xt[j] = 0
            j += 1
        file_out.write("\n")
    i += 1

file_out.close();
