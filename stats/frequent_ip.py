#!/usr/bin/python
# coding=utf-8
#
# Parsing APNIC's list of "frequent IP". The list has several columns:
# resolver
# originAS
# resolver CC
# validating? (yes/no)
# proportion of user share (weighted)
# proportion of user share that asks validating (weighted)
# proportion of user share (unweighted)
# proportion of user share that asks validating (unweighted)
# count of users for this IP addr (weighted)
# count of users asking validation (wieghted)
# count of users for this IP addr
# count of users asking validation

import traceback

class frequent_ip_line:
    def __init__(self):
        self.ip = ""
        self.originAS = 0
        self.resolverCC = ""
        self.validating = False
        self.p_user_share_weighted = 0.0       
        self.p_user_share_validating_weighted = 0.0
        self.p_user_share_unweighted = 0.0       
        self.p_user_share_validating_unweighted = 0.0
        self.count_users_weighted = 0.0
        self.count_users_validating_weighted = 0.0
        self.count_users = 0
        self.count_users_validating = 0

    def load(self, line):
        parts = line.split(",")
        try: 
            self.ip = parts[0].strip()
            as_text = parts[1].strip()
            if as_text.startswith("AS"):
                self.originAS = int(as_text[2:])
            self.resolverCC = parts[2].strip()
            if parts[3].strip() == "yes":
                self.validating = True
            self.p_user_share_weighted = float(parts[4].strip())       
            self.p_user_share_validating_weighted = float(parts[5].strip()) 
            self.p_user_share_unweighted = float(parts[6].strip())        
            self.p_user_share_validating_unweighted = float(parts[7].strip()) 
            self.count_users_weighted = float(parts[8].strip()) 
            self.count_users_validating_weighted = float(parts[9].strip()) 
            self.count_users = int(parts[10].strip()) 
            self.count_users_validating = int(parts[11].strip())
        except:
            traceback.print_exc()
            print("Cannot parse: " + line.strip())
            self.ip = ""

    def compare(self, other):
        if (self.count_users_weighted < other.count_users_weighted):
            return -1
        elif (self.count_users_weighted > other.count_users_weighted):
            return 1
        elif (self.count_users < other.count_users):
            return -1
        elif (self.count_users > other.count_users):
            return 1
        elif (self.ip < other.ip):
            return -1
        elif (self.ip > other.ip):
            return 1
        else:
            return 0
    
    def __lt__(self, other):
        return self.compare(other) < 0
    def __gt__(self, other):
        return self.compare(other) > 0
    def __eq__(self, other):
        return self.compare(other) == 0
    def __le__(self, other):
        return self.compare(other) <= 0
    def __ge__(self, other):
        return self.compare(other) >= 0
    def __ne__(self, other):
        return self.compare(other) != 0

class frequent_ip:
    def __init__(self):
        self.table = dict()
        self.limit_10000 = 0
        self.largest = 0
        self.smallest = 0

    def set_10000_limit(self):
        v = sorted(self.table.values(), reverse=True)
        self.largest = v[1].count_users_weighted
        self.smallest = v[len(v)-1].count_users_weighted
        if len(self.table) > 10001:
            self.limit_10000 = (v[10000].count_users_weighted + v[10001].count_users_weighted)/2
        else:
            self.limit_10000 = self.smallest

    def load(self, file_name):
        for line in open(file_name,"rt"):
            fip = frequent_ip_line()
            fip.load(line)
            if len(fip.ip) > 0:
                self.table[fip.ip] = fip
        self.set_10000_limit()

    def is_frequent(ip):
        r = False
        if ip in self.table:
            r = True
        return r



