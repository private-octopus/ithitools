
#!/usr/bin/python
# coding=utf-8
#
# This script reformats a list of leaked names produced by ithitools option "-E".
# The script takes three parameters:
# $1: the file produced by ithitools
# $2: the file on which the formatted list of leaked named will be written
# $3: the list of top names

import codecs
import sys
import anomdns
import ithifile

# Main loop

if len(sys.argv) != 4:
    print("Usage: " + sys.argv[0] + " <input-file> <full-list-file> <top-list-file>\n")
    exit(1)

list = []
file = codecs.open(sys.argv[1], "r", "UTF-8")
anonymize = 1
anom = anomdns.anonymizer()
anom.set_key("ithi-privacy")

for line in file:
    try:
        line = line.rstrip()
        d_line = ithifile.domain_line()
        d_line.file_line(line, anonymize, anom)
        if (d_line.total > 0):
            list.append(d_line)
    except:
        e = sys.exc_info()[0]
        print ( "Error" + str(e) + "\n")
        pass

file.close()

list.sort()

l = len(list)
i = 0
start_tld = 0
start_second = 0
nb_tld = 0
nb_second = 0

while ( i < l):
    nb_tld += list[i].total
    nb_second += list[i].total
    if ((i+1 >= l) or (list[i].tld != list[i+1].tld)):
        while (start_tld <= i):
            list[start_tld].n_tld = nb_tld
            start_tld += 1
        nb_tld = 0
    if ((i+1 >= l) or (list[i].tld != list[i+1].tld) or (list[i].second != list[i+1].second)):
        while (start_second <= i):
            list[start_second].n_second = nb_second
            start_second += 1
        nb_second = 0
    i += 1

list.sort(reverse=True)

file_out = codecs.open(sys.argv[2], "w", "UTF-8")
for d_line in list:
        file_out.write(d_line.to_string() + "\n")
file_out.close();

file_top = codecs.open(sys.argv[3], "w", "UTF-8")
i = 0
nb_tld = 1;
nb_second = 1;
start_second = 1

while ( i < l):  
    if (start_second != 0 and nb_second <= 10):
        file_top.write(list[i].to_string() + "\n")
        start_second = 0

    if ( i+1 >= l or list[i].tld != list[i+1].tld ):
        print ("End of Tld: " + list[i].tld);
        nb_tld += 1
        if (nb_tld > 10):
            break;
        nb_second = 0
        start_second = 1

    if ((i+1 >= l) or (list[i].tld != list[i+1].tld) or (list[i].second != list[i+1].second)):
        nb_second += 1
        start_second = 1

    i += 1

file_top.close();