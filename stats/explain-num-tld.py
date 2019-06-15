
#!/usr/bin/python
# coding=utf-8
#
# This script reformats a list of leaked names and extracts the subset of TLD that have
# numeric formats.

import codecs
import sys
import anomdns
import ithifile

# Main loop

if len(sys.argv) != 4:
    print("Usage: " + sys.argv[0] + " <input-file> <full-list-file> <short-list-file>\n")
    exit(1)

list = []
file = codecs.open(sys.argv[1], "r", "UTF-8")

anonymize = 1
anom = anomdns.anonymizer()
anom.set_key("ithi-privacy")

for line in file:
    try:
        line = line.rstrip()
        d_line = ithifile.ip_domain_line()
        d_line.file_line(line, anonymize, anom)
        if (d_line.ip_tld != ""):
            list.append(d_line)
    except:
        e = sys.exc_info()[0]
        print ( "Error" + str(e) + "\n")
        pass

file.close()

print("Found " + str(len(list)) + " domains")

list.sort()

l = len(list)
i = 0
start_ip_tld = 0
nb_ip_tld = 0

while ( i < l):
    nb_ip_tld += list[i].total
    if ((i+1 >= l) or (list[i].ip_tld != list[i+1].ip_tld)):
        while (start_ip_tld <= i):
            list[start_ip_tld].n_ip_tld = nb_ip_tld
            start_ip_tld += 1
        nb_ip_tld = 0
    i += 1

list.sort(reverse=True)

file_out = codecs.open(sys.argv[2], "w", "UTF-8")
file_short = codecs.open(sys.argv[3], "w", "UTF-8")
nb_in_ip = 0
nb_ip = 0
i = 0
while ( i < l):
    file_out.write(list[i].to_string() + "\n")
    nb_in_ip += 1
    if (nb_in_ip == 1 and nb_ip < 100):
        nb_ip += 1
        file_short.write(list[i].to_string() + "\n")
    if ((i+1 >= l) or (list[i].ip_tld != list[i+1].ip_tld)):
        nb_in_ip = 0
    i += 1
file_short.close();
file_out.close();
