#!/usr/bin/python
# coding=utf-8
#
# This modules defines the domain_line class, 
# which is used when parsing the ithifiles.

import codecs
import sys
import anomdns

class domain_line:
    def __init__(self):
        self.n_tld = 0
        self.tld = ""
        self.n_second = 0
        self.second = ""
        self.name = ""
        self.total = 0
    
    def __repr__(self):
        return '{}: {} {}  {} {} {} {}'.format(self.__class__.__name__,
                                  self.n_tld,
                                  self.tld,
                                  self.n_second,
                                  self.second,
                                  self.number,
                                  self.name)

    #Example: FULL_NAME_LIST,1,SENDIT.BOFSM.FM.INFOATRISK.LOCAL,1,
    def file_line(self, line, anonymize, anom):
        csv = line.split(",")
        lcsv = len(csv)
        if (lcsv >= 4 and csv[0] == "FULL_NAME_LIST"):
            self.total = int(csv[3])
            if (anonymize != 0):
                self.name = anom.anonymizeName(csv[2], 2)
            else:
                self.name = csv[2]
            tokens = self.name.split(".")
            l = len(tokens)
            if (l > 0):
                self.tld = tokens[l-1]
            if (l > 1):
                self.second = tokens[l-2]

    def to_string(self):
        l_str = self.tld + "," + str(self.n_tld) + "," + self.second + "," + str(self.n_second) + ","  + self.name + "," + str(self.total)
        return l_str

    def getKey(self):
        return self.to_string()

    def myComp(self, other):
        if (self.n_tld < other.n_tld):
            return -1
        elif (self.n_tld > other.n_tld):
            return 1
        elif (self.tld < other.tld):
            return -1
        elif (self.tld > other.tld):
            return 1
        elif (self.n_second < other.n_second):
            return -1
        elif (self.n_second > other.n_second):
            return 1
        elif (self.second < other.second):
            return -1
        elif (self.second > other.second):
            return 1
        elif (self.total < other.total):
            return -1
        elif (self.total > other.total):
            return 1
        elif (self.name < other.name):
            return -1
        elif (self.name > other.name):
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

class ip_domain_line:
    def __init__(self):
        self.ip_tld = ""
        self.n_ip_tld = 0
        self.name = ""
        self.total = 0
    
    def __repr__(self):
        return '{}: {} {} {} {}'.format(self.__class__.__name__,
                                               self.n_ip_tld, self.ip_tld, self.number, self.name)

    def is_num_tld(self):
        return is_num_token(self.tld)

    def file_line(self, line, anonymize, anom):
        d_line = domain_line()    
        d_line.file_line(line, 0, 0)
        if (d_line.total > 0 and d_line.tld.isnumeric()):
            # This could be an IPv4 TLD
            s = "."
            tokens = d_line.name.split(s)
            l = len(tokens)
            if (l >= 4 and anomdns.areIpv4Tokens(tokens, l-4) != 0):
                self.total = d_line.total
                self.ip_tld = tokens[l-4] + "." + tokens[l-3] + "." + tokens[l-2] + "." + tokens[l-1]
                if (anonymize != 0):
                    self.ip_tld = anom.anonymizeAddress(self.ip_tld)
                    if (l > 4):
                        l -= 4
                        while (len(tokens) > l):
                            del(tokens[l-1])
                        self.name = anom.anonymizeName(s.join(tokens), 0)
                        self.name += "." + self.ip_tld
                    else:
                        self.name = self.ip_tld
                else:
                    self.name = d_line.name

    def to_string(self):
        l_str = self.ip_tld + "," + str(self.n_ip_tld) + ","  + self.name + "," + str(self.total)
        return l_str

    def getKey(self):
        return self.to_string()

    def myComp(self, other):
        if (self.n_ip_tld < other.n_ip_tld):
            return -1
        elif (self.n_ip_tld > other.n_ip_tld):
            return 1
        if (self.ip_tld < other.ip_tld):
            return -1
        elif (self.ip_tld > other.ip_tld):
            return 1
        elif (self.total < other.total):
            return -1
        elif (self.total > other.total):
            return 1
        elif (self.name < other.name):
            return -1
        elif (self.name > other.name):
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