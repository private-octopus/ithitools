# Parsing of the APNIC resolver data  file.
#
# here's a sample record:
#
#
# 1730419200.001728 client 172.68.246.89#37826: query: 0du-results-uf8c998ed-c233-a1ef2-s1730419189-i00000000-0.am.dotnxdomain.net. IN DS -ED () 1914810962 0
#
# Field 1 - time the query was received by the auth server
# Field 2 - the word "client"
# Field 3 - the source IP address and port number (i.e. the IP address of the recursive resolver that passed us the query)
# Field 4- the word "query"
# Field 5 - The query name (see below)
# Field 6 - The Query class ("IN")
# Field 7 - the Query type (in this case DS)
# Field 8 - EDNS values E = EDNS, D = DNSSEC OK If client subnet is being used it is found here (e.g. "IN HTTPS -EDS6/56/0|2001:56a:7636:6e00::2000:0 () 1914810962 0")
#
# The query name is a sequence of fields delineated by a hyphen
#
# 0du-u0b7cf17d-c13-a04C5-s1730796660-i6e8d88e1-0.ap.dotnxdomain.net
#
# 1. Experiment codes 0du
#
# 0du - dual stack, not signed with DNSSEC
# 04u - V4 only resource record, not signed with DNSSEC
# 06u - V4 only resource record, not signed with DNSSEC
# 0ds - dual stack - dnssec signed
# 0di - dual stack, invalid DNSSEC signature
# fdu - dual stack - always returns servfail response code
# 
# I suggest ignoring all else and just look at 0du and fdu entries
# 
# 2. User identifier - u0b7cf17da hex-encoded uuid value. 
# 
# All queries with a common user identifier value were from the same initial ad presentation.
# 
# 3. COuntry code - c13
# 
# 
# The country where the end user is located. 13 is Country AU = Australia
# The table of country codes is in TABLE 1.
#
# 4. Origin AS - a04C5
# 
# The hex value of the origin AS - hex 4c5 is 1221
# 
# 5 - time of ad generation - s1730796660
# 
# unix timestamp value (seconds since 1 Jan 1970)
# 
# 
# 6 - IPv4 address of client - i6e8d88e1
#
# IF the ad was originally delivered using IPv4 (and many are not) then the
# ipv4 address of the client is encoded here in hex
# 
# 6e8d88e1 = 110.141.135.225

import country
import traceback

class rsv_log_line:
    def __init__(self):
        self.time_query = 0.0
        self.resolver_IP = "0.0.0.0"
        self.resolver_port = 0
        self.query_experiment = ""
        self.query_user_id = ""
        self.query_cc = ""
        self.query_as = ""
        self.query_ad_time = 0
        self.query_ip = ""
        self.query_class = ""
        self.query_type = ""
        self.query_edns = ""
        self.is_results = False
        self.is_anomalous = False
        self.is_cretinous = False
        self.is_starquery = False
        self.invalid_query = ""
        self.server = ""

    def parse_query_name_params(self, query_parts):
        is_valid = True
        self.query_experiment = query_parts[0]
        if query_parts[1] == "results":
            self.is_results = True
            query_parts = query_parts[1:]
        self.query_user_id = query_parts[1]

        self.query_cc = country.country_code_from_c999(query_parts[2])

        query_as_str = query_parts[3]
        if query_as_str.startswith("a"):
            as_num = int(query_as_str[1:], 16)
            self.query_as = "AS" + str(as_num)
        else:
            print("Bad AS:" + query_as_str )
            is_valid = False
        ad_time_str = query_parts[4]
        if ad_time_str.startswith("s"):
            self.query_ad_time = int(ad_time_str[1:])
        else:
            print("Bad Time:" + ad_time_str )
            is_valid = False
        query_ip_str = query_parts[5]
        if query_ip_str.startswith("i"):
            ip_num = int(query_ip_str[1:], 16)
            self.query_ip = str(ip_num>>24)+ "." + \
                str((ip_num>>16)&255)+ "." + \
                str((ip_num>>8)&255)+ "." + \
                str(ip_num&255)
        else:
            print("Bad IP:" + query_ip_str )
            is_valid = False
        return is_valid

    def parse_query_name_anomalous(self, query_parts):
        #query: 000-000-000a-0000-0006-e7b5bab7-233-a55A8-1736378116-ac380eb6-0
        # query: 04u-uf8c0aa51-c185-a40625-s1730422799-iaa54ac12-0
        is_valid = True
        if len(query_parts) < 10:
            return False
        delimiter = "-"
        self.query_experiment = delimiter.join(query_parts[:5])
        self.query_user_id = query_parts[5]
        self.query_cc = country.country_code_from_c999("c" + query_parts[6])
        query_as_str = query_parts[7]
        if query_as_str.startswith("a"):
            as_num = int(query_as_str[1:], 16)
            self.query_as = "AS" + str(as_num)
        else:
            print("Bad AS:" + query_as_str )
            is_valid = False
        self.query_ad_time = int(query_parts[8])
        ip_num = int(query_parts[9], 16)
        self.query_ip = str(ip_num>>24)+ "." + \
            str((ip_num>>16)&255)+ "." + \
            str((ip_num>>8)&255)+ "." + \
            str(ip_num&255)
        return is_valid
        
    def parse_query_name(self, query_name):
        is_valid = True
        query_name = query_name.strip()
        name_parts = query_name.split(".")
        if len(name_parts[-1]) == 0:
            name_parts = name_parts[:-1]
        if len(name_parts) < 3:
            delimiter = "."
            self.server = delimiter.join(name_parts)
        else:
            delimiter = "."
            self.server = delimiter.join(name_parts[-3:])
        query_string = name_parts[0]
        query_parts = query_string.split("-")
        if query_string.startswith("000-000-000"):
            self.is_anomalous = True
            is_valid = self.parse_query_name_anomalous(query_parts)
        elif len(query_parts) < 6:
            if self.server.endswith(".starnxdomain.net"):
                self.is_starquery = True
            else:
                self.is_cretinous = True
                self.invalid_query = query_string
        else:
            is_valid = self.parse_query_name_params(query_parts)
        return is_valid


    def parse_line(self, s):
        is_valid = False
        s = s.strip()
        parts = s.split(" ")
        if len(parts) >= 8 and \
            parts[1] == "client" and \
            parts[3] == "query:":
            try:
                self.time_query = float(parts[0])
                # parse IP and port
                ip_ports_str = parts[2]
                if ip_ports_str.endswith(":"):
                    ip_ports_str = ip_ports_str[:-1]
                ip_ports = ip_ports_str.split("#")
                self.resolver_IP = ip_ports[0]
                self.resolver_port = int(ip_ports[1])
                # parse the query class and type
                self.query_class = parts[5]
                self.query_type = parts[6]
                # parse the EDNS data
                delimiter = " "
                self.query_edns = delimiter.join(parts[7:])
                # parse the query name
                is_valid = self.parse_query_name(parts[4])
            except Exception as exc:
                traceback.print_exc()
                print('\nCode generated an exception: %s' % (exc))
                print("Cannot parse:\n" + s + "\n")
                is_valid = False
        return is_valid

    def pretty_string(self):
        r = ""
        a = ""
        c = ""
        q = ""
        if self.is_results:
            r = "R"
        if self.is_anomalous:
            a = "A"
        if self.is_cretinous:
            c = "C"
        if self.is_starquery:
            q = "S"

        s = str(self.time_query) + ", " + \
            self.resolver_IP + ", " + \
            str(self.resolver_port) + ", " + \
            self.query_experiment + ", " + \
            self.query_user_id + ", " + \
            self.query_cc + ", " + \
            self.query_as + ", " + \
            str(self.query_ad_time) + ", " + \
            self.query_ip + ", " + \
            self.query_class  + ", " + \
            self.query_type  + ", " + \
            self.server  + ", " + \
            r  + ", " + \
            a  + ", " + \
            c  + ", " + \
            q  + ", " + \
            "\"" + self.query_edns + "\", " + \
            self.invalid_query
        return s

             


