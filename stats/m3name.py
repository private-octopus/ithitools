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

# parse file names like
# /home/rarends/data/20190609/aa01-in-bom.l.dns.icann.org/20190609-132848_300-aa01-in-bom.l.dns.icann.org.csv
# or 20180512-105748_300-bah01.l.root-servers.org.csv
class m3name:
    def __init__(self):
        self.m3_date = ""
        self.m3_hour = ""
        self.duration = 0
        self.country_code = ""
        self.city_code = ""
        self.address_id = ""

    def parse_file_id(self, file_id):
        parts = file_id.split("/")
        file_name = parts[len(parts) - 1]
        name_parts = file_name.split("-")
        if (len(name_parts) >= 5):
            self.address_id = name_parts[2]
            self.country_code = name_parts[3]
            city_parts = name_parts[4].split(".")
            self.city_code = city_parts[0]
            if (len(self.country_code) != 2):
                print("Wrong Country Code in " + file_name)
                return -1
            if (len(self.city_code) != 3):
                print("Wrong City Code id in " + file_name)
                return -1
        elif (len(name_parts) >= 3):
            self.country_code = "??"
            old_city_parts = name_parts[2].split(".")
            if len(old_city_parts[0]) != 5:
                print("Wrong Country Code in " + file_name)
                return -1
            else:
                old_city = old_city_parts[0]
                self.address_id = "aa" + old_city[3:5]
                self.city_code = old_city[0:3]
        else:
            print("Wrong syntax in " + file_name)
            return -1
        if (len(name_parts[0]) != 8):
            print("Wrong date in " + file_name)
            return -1
        s_date = name_parts[0]
        self.m3_date = s_date[0:4] + "/" + s_date[4:6] + "/" + s_date[6:8]
        time_parts = name_parts[1].split("_")
        if (len(time_parts) != 2):
            print("Wrong cc in " + file_name)
            return -1
        if (len(time_parts[0]) != 6):
            print("Wrong hour in " + file_name)
            return -1
        s_hour = time_parts[0]
        self.m3_hour = s_hour[0:2] + ":" + s_hour[2:4] + ":" + s_hour[4:6]
        try:
            self.duration = int(time_parts[1],10)
        except:
            print("Wrong duration in " + file_name)
            return -1
        
        return 0

def m3name_test():
    test_file = [
        "/home/rarends/data/20190609/aa01-in-bom.l.dns.icann.org/20190609-132848_300-aa01-in-bom.l.dns.icann.org.csv",
        "/home/rarends/data/20190614/aa01-mx-mty.l.dns.icann.org/20190614-143947_300-aa01-mx-mty.l.dns.icann.org.csv",
        "/home/rarends/data/20190609/aa01-fr-par.l.dns.icann.org/20190609-144834_300-aa01-fr-par.l.dns.icann.org.csv",
        "20190609-144834_25-aa01-fr-par.l.dns.icann.org.csv",
        "20180512-105748_300-bah01.l.root-servers.org.csv"]
    test_date = [ "2019/06/09", "2019/06/14", "2019/06/09", "2019/06/09", "2018/05/12" ]
    test_hour = [ "13:28:48", "14:39:47", "14:48:34", "14:48:34", "10:57:48"]
    test_duration = [ 300, 300, 300, 25, 300] 
    test_country_code = [ "in", "mx", "fr", "fr", "??" ]
    test_city_code = [ "bom", "mty", "par", "par", "bah" ]
    test_address_id = ["aa01", "aa01", "aa01", "aa01", "aa01" ]

    i = 0
    ret = 0
    while i < len(test_file):
        m3n = m3name()
        if m3n.parse_file_id(test_file[i]) != 0:
            print ("Cannot parse " + test_file[i])
        if (test_date[i] != m3n.m3_date):
            print("Error, " + test_file[i] + ", date: " + m3n.m3_date)
            ret = -1
        if (test_hour[i] != m3n.m3_hour):
            print("Error, " + test_file[i] + ", hour: " + m3n.m3_hour)
            ret = -1
        if (test_duration[i] != m3n.duration):
            print("Error, " + test_file[i] + ", duration: " + str(m3n.duration))
            ret = -1
        if (test_address_id[i] != m3n.address_id):
            print("Error, " + test_file[i] + ", address_id: " + m3n.address_id)
            ret = -1
        if (test_country_code[i] != m3n.country_code):
            print("Error, " + test_file[i] + ", country_code: " + m3n.country_code)
            ret = -1
        if (test_city_code[i] != m3n.city_code):
            print("Error, " + test_file[i] + ", city_code: " + m3n.city_code)
            ret = -1
        i += 1
    return ret

    

