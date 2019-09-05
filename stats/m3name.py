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

# Some files use the old format
# we need a function that derives country code from city code

city_list = ["aad","aad","abj","akl","amm","ams","anc","ank","anr","apw","arn","asu","atl","azo",
                "bab","bah","bak","bcl","bcn","beg","bel","bey","bfc","bfh","bfj","bfz","bhz","bjd",
                "bkk","blq","blz","bne","bnk","bog","bom","brf","bru","bsb","bts","byk","cbb","cbg",
                "ccs","ccu","cdg","cdo","cfc","cgh","chc","cll","cmb","cmn","cnf","cph","cpq","cpt",
                "crn","cxh","dar","dca","den","dkr","dmm","dnd","dsm","dtm","dun","dus","dwc","dxb",
                "epb","esb","evn","eza","eze","fdk","fln","flr","fms","for","fzh","gbq","gig","gme",
                "goz","gru","gum","gva","gyd","ham","hel","her","hgt","hir","hkl","hlm","hmm","hnd",
                "hnl","hrk","iad","icn","iev","ilg","isb","jkt","jnb","jog","jqb","jrs","kah","kbp",
                "kiv","ksa","kwi","lax","lba","lcl","lcy","ldb","lhe","lim","lio","ltn","lwc","lys",
                "mad","mae","mai","maj","mas","maw","mbv","mct","mdl","meb","mel","mia","mis","mma",
                "mmx","mnl","mot","mow","mrs","mru","msh","msq","msu","mty","mvd","nan","nat","nbe",
                "ncp","noi","nou","ntc","ods","opo","ord","ory","osl","oti","ott","oua","par","pdx",
                "pek","per","phb","phx","plx","pni","poa","pom","ppt","prg","pvg","rba","rcs","rgn",
                "rmh","rml","rno","rov","rtv","run","sah","sal","sao","scl","sdq","sdu","sea","sez",
                "sjc","sjk","sjo","sju","skt","smc","sof","ssa","sto","stp","suv","svo","svx","syd",
                "tkx","tlv","tor","tpe","tun","udi","uio","vat","vcp","vko","wnp","xuy","yek","yow",
                "ytz","yvr","ywg","yyz","zse"]
country_list = ["xz","xz","ci","nz","jo","nl","us","tr","be","ws","se","py","us","us","sk","bh",
                "az","br","es","rs","br","lb","fr","br","fj","lb","br","cn","th","it","mw","au",
                "in","co","in","gb","be","br","sk","ci","bo","kw","ve","in","fr","pt","br","us",
                "nz","pe","lk","ma","br","dk","br","za","br","ca","tz","us","us","sn","sa","gb",
                "us","de","gb","de","ae","ae","es","tr","am","ar","ar","us","br","it","br","br",
                "cn","bh","br","by","bg","br","gu","ch","az","de","fi","gr","gu","sb","gr","us",
                "nl","jp","us","ua","us","kr","ua","us","pk","id","za","id","do","il","au","ua",
                "ua","fm","kw","us","gb","gb","gb","br","pk","pe","fr","it","us","fr","es","nz",
                "ve","mh","au","sc","fr","om","mm","au","au","us","ca","se","se","ph","ch","ru",
                "fr","mu","om","by","ls","mx","uy","fj","br","tn","ph","ru","nc","tw","ua","pt",
                "us","fr","no","ro","ca","bf","fr","us","cn","au","br","us","kz","in","br","pg",
                "pf","cz","cn","ma","gb","mm","ps","lk","us","ru","us","re","ye","sv","br","cl",
                "do","br","us","us","us","br","cr","pr","pk","ar","bg","br","se","xz","fj","ru",
                "ru","au","jp","is","ca","tw","tn","br","ec","fi","br","ru","ca","cz","ru","ca",
                "ca","ca","ca","ca","re"]

def country_from_city(city):
    low = 0
    high = len(city_list) - 1
    if city == city_list[low]:
        return country_list[low]
    elif city < city_list[low]:
        return "??"
    elif str(city) == city_list[high]:
        return country_list[high]
    elif city > city_list[high]:
        return "??"
    else:
        while high - low > 1:
            middle = int((high + low)/2)
            if city != city_list[middle]:
                if city < city_list[middle]:
                    high = middle
                else:
                    low = middle
            else:
                return country_list[middle]
    return "??"

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
        if len(parts) == 1:
            parts = file_id.split("\\")
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
            old_city_parts = name_parts[2].split(".")
            if len(old_city_parts[0]) != 5:
                print("Wrong Country Code in " + file_name)
                return -1
            else:
                old_city = old_city_parts[0]
                self.address_id = "aa" + old_city[3:5]
                self.city_code = old_city[0:3]
                self.country_code = country_from_city(self.city_code)
        else:
            print("Wrong syntax in " + file_name)
            return -1
        if (len(name_parts[0]) != 8):
            print("Wrong date in " + file_name + " got: " + name_parts[0])
            return -1
        s_date = name_parts[0]
        self.m3_date = s_date[0:4] + "-" + s_date[4:6] + "-" + s_date[6:8]
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

def city_test():
    test_city = [ "aaa", "aad", "aae", "zsd", "zse", "zzz", "bex", "bey", "bez", "pha", "phb", "phc"]
    test_country = [ "??", "xz", "??", "??", "re", "??", "??", "lb", "??", "??", "br", "??"]
    t = 0
    ret = 0
    while t < len(test_city):
        cc = country_from_city(test_city[t]);
        if cc != test_country[t]:
            print("for city <" + test_city[t] + "> expected <" + test_country[t] + "> got <" +
                  cc + ">")
            ret = -1
        t += 1

    if ret == 0:
        t = 0
        while t < len(city_list):
            cc = country_from_city(city_list[t]);
            if cc != country_list[t]:
                print("for city <" + city_list[t] + "> expected <" + country_list[t] + "> got <" +
                        cc + ">")
                ret = -1
            t += 1


    return ret

def m3name_test():
    test_file = [
        "/home/rarends/data/20190609/aa01-in-bom.l.dns.icann.org/20190609-132848_300-aa01-in-bom.l.dns.icann.org.csv",
        "/home/rarends/data/20190614/aa01-mx-mty.l.dns.icann.org/20190614-143947_300-aa01-mx-mty.l.dns.icann.org.csv",
        "/home/rarends/data/20190609/aa01-fr-par.l.dns.icann.org/20190609-144834_300-aa01-fr-par.l.dns.icann.org.csv",
        "20190609-144834_25-aa01-fr-par.l.dns.icann.org.csv",
        "20180512-105748_300-bah01.l.root-servers.org.csv"]
    test_date = [ "2019-06-09", "2019-06-14", "2019-06-09", "2019-06-09", "2018-05-12" ]
    test_hour = [ "13:28:48", "14:39:47", "14:48:34", "14:48:34", "10:57:48"]
    test_duration = [ 300, 300, 300, 25, 300] 
    test_country_code = [ "in", "mx", "fr", "fr", "bh" ]
    test_city_code = [ "bom", "mty", "par", "par", "bah" ]
    test_address_id = ["aa01", "aa01", "aa01", "aa01", "aa01" ]

    if city_test() == 0:
        print("City test passes")
    else:
        return -1

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

    

