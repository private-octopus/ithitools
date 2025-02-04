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

from math import nan
from ssl import ALERT_DESCRIPTION_DECRYPT_ERROR
import country
import traceback
import pandas as pd
import ip2as
import open_rsv

class rsv_log_line:
    def __init__(self):
        self.query_time = 0.0
        self.resolver_IP = "0.0.0.0"
        self.resolver_port = 0
        self.resolver_AS = ""
        self.resolver_cc = ""
        self.resolver_tag = ""
        self.query_experiment = ""
        self.query_user_id = ""
        self.query_cc = ""
        self.query_AS = ""
        self.query_ad_time = 0
        self.query_ip = ""
        self.rr_class = ""
        self.rr_type = ""
        self.query_edns = ""
        self.is_results = False
        self.is_anomalous = False
        self.is_cretinous = False
        self.is_starquery = False
        self.invalid_query = ""
        self.server = ""

    def parse_query_name_params(self, query_parts, query_name):
        is_valid = True
        self.query_experiment = query_parts[0]
        if query_parts[1] == "results":
            self.is_results = True
            query_parts = query_parts[1:]
        self.query_user_id = query_parts[1]

        self.query_cc = country.country_code_from_c999(query_parts[2])

        query_AS_str = query_parts[3]
        if query_AS_str.startswith("a"):
            as_num = int(query_AS_str[1:], 16)
            self.query_AS = "AS" + str(as_num)
            as_parsed = 1
        else:
            #print("Bad AS:" + query_name )
            self.query_AS = "AS0"
            as_parsed = 0
        ad_time_str = query_parts[3+as_parsed]
        if ad_time_str.startswith("s"):
            self.query_ad_time = int(ad_time_str[1:])
        else:
            print("Bad Time:" + query_name )
            is_valid = False
        query_ip_str = query_parts[4+as_parsed]
        if query_ip_str.startswith("i"):
            ip_num = int(query_ip_str[1:], 16)
            self.query_ip = str(ip_num>>24)+ "." + \
                str((ip_num>>16)&255)+ "." + \
                str((ip_num>>8)&255)+ "." + \
                str(ip_num&255)
        else:
            print("Bad IP:" + query_name )
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
        query_AS_str = query_parts[7]
        if query_AS_str.startswith("a"):
            as_num = int(query_AS_str[1:], 16)
            self.query_AS = "AS" + str(as_num)
            as_parsed = 1
        else:
            print("Bad AS:" + query_AS_str )
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
            is_valid = self.parse_query_name_params(query_parts, query_name)
        return is_valid

    def parse_line(self, s):
        is_valid = False
        s = s.strip()
        parts = s.split(" ")
        if len(parts) >= 8 and \
            parts[1] == "client" and \
            parts[3] == "query:":
            try:
                self.query_time = float(parts[0])
                # parse IP and port
                ip_ports_str = parts[2]
                if ip_ports_str.endswith(":"):
                    ip_ports_str = ip_ports_str[:-1]
                ip_ports = ip_ports_str.split("#")
                self.resolver_IP = ip_ports[0]
                self.resolver_port = int(ip_ports[1])
                # parse the query class and type
                self.rr_class = parts[5]
                self.rr_type = parts[6]
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

    # The filter function applies common filters:
    # - query time at most 10 second later than ad time
    # - rr_class = [ "odu" ]
    # - rr_types = [ "A", "AAAA" ]
    # - is_results = False
    # - query_ASes = [] (could be a specific set of ASes)
    def filter(self, query_delay=10, experiment=["0du"], rr_types=["A", "AAAA"], is_results=[False], query_ASes=[]):
        filter_OK = True
        if query_delay > 0:
            qd = int(self.query_time) - self.query_ad_time
            filter_OK = (qd <= query_delay)

        if filter_OK and len(experiment) > 0:
            filter_OK = False
            for ex in experiment:
                if ex == self.query_experiment:
                    filter_OK = True
                    break
        if filter_OK and len(rr_types) > 0:
            filter_OK = False
            for qt in rr_types:
                if qt == self.rr_type:
                    filter_OK = True
                    break
        if filter_OK and len(is_results) > 0:
            filter_OK = False
            for ir in is_results:
                if ir == self.is_results:
                    filter_OK = True
                    break
        if filter_OK and len(query_ASes) > 0:
            filter_OK = False
            for asn in query_ASes:
                if asn == self.query_AS:
                    filter_OK = True
                    break
        return filter_OK
    
    # set_resolver_AS checks the AS number associated with the source address
    # of the query, and the country code for that AS. Using the source address
    # and the AS number, it find whether this matches an "open resolver".
    # If it does not, it sets the "same AS" flag, and if this is False it
    # sets the "same CC" flag.
    #
    # The additional arguments are the table mapping IPv4 addresses
    # to ASes (ip2a4), IPv6 addresses to (ip2a6) and the AS number
    # to a CC (as_table)
    def set_resolver_AS(self, ip2a4, ip2a6, as_table):
        parts6 = self.resolver_IP.split(":")
        if len(parts6) > 1:
            asn = ip2a6.get_asn(self.resolver_IP)
        else:
            asn = ip2a4.get_asn(self.resolver_IP)
        self.resolver_AS = "AS" + str(asn)
        self.resolver_cc = as_table.cc(self.resolver_AS)
        self.resolver_tag = open_rsv.get_open_rsv(self.resolver_IP, self.resolver_AS)
        if len(self.resolver_tag) == 0:
            if self.resolver_AS == self.query_AS:
                self.resolver_tag = "Same_AS"
            elif self.resolver_cc == self.query_cc:
                self.resolver_tag = "Same_CC"
            else:
                self.resolver_tag = "Others"


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

        s = str(self.query_time) + ", " + \
            self.resolver_IP + ", " + \
            str(self.resolver_port) + ", " + \
            self.query_experiment + ", " + \
            self.query_user_id + ", " + \
            self.query_cc + ", " + \
            self.query_AS + ", " + \
            str(self.query_ad_time) + ", " + \
            self.query_ip + ", " + \
            self.rr_class  + ", " + \
            self.rr_type  + ", " + \
            self.server  + ", " + \
            r  + ", " + \
            a  + ", " + \
            c  + ", " + \
            q  + ", " + \
            "\"" + self.query_edns + "\", " + \
            self.invalid_query
        return s

    def header():
        header = [ 'query_time', \
            'resolver_IP', \
            'resolver_port', \
            'resolver_AS', \
            'resolver_tag', \
            'resolver_cc', \
            'experiment_id', \
            'query_user_id', \
            'query_cc', \
            'query_AS', \
            'query_ad_time', \
            'query_IP', \
            'rr_class', \
            'rr_type', \
            'server', \
            'is_result', \
            'is_anomalous', \
            'is_cretinous', \
            'is_starquery', \
            'query_edns', \
            'invalid_query' ]
        return header

    def row(self):
        r = [ self.query_time, \
            self.resolver_IP, \
            self.resolver_port, \
            self.resolver_AS, \
            self.resolver_tag, \
            self.resolver_cc, \
            self.query_experiment, \
            self.query_user_id, \
            self.query_cc, \
            self.query_AS, \
            self.query_ad_time, \
            self.query_ip, \
            self.rr_class, \
            self.rr_type, \
            self.server, \
            self.is_results, \
            self.is_anomalous, \
            self.is_cretinous, \
            self.is_starquery, \
            self.query_edns , \
            self.invalid_query ]
        return r


class rsv_log_file:
    def __init__(self, filtering=True):
        self.m = []
        self.filtering = filtering
    def load(self, file_name, ip2a4, ip2a6, as_table, rr_types=[], experiment=[], query_ASes=[]):
        for line in open(file_name, "r"):
            try:
                x = rsv_log_line()
                if x.parse_line(line):
                    if (not self.filtering or x.filter(rr_types=rr_types, experiment=experiment)):
                        x.set_resolver_AS(ip2a4, ip2a6, as_table)
                        self.m.append(x.row())
                else:
                    print("Bad line:" + line.strip())
            except Exception as exc:
                traceback.print_exc()
                print('\nCode generated an exception: %s' % (exc))
                print("Cannot parse:\n" + line + "\n")
                break
        print("Data matrix has " + str(len(self.m)) + " lines.")

    def get_frame(self):
        df = pd.DataFrame(self.m,columns=rsv_log_line.header())
        return df

# pivot per query, produce a view keeping first reply per query.
# result frame will have one column per dsp, with time of first arrival.
class pivoted_per_query:
    def __init__(self):
        self.rqt = dict()
        self.tried = 0
        self.tags = [ 'Same_AS', 'Same_CC', 'Others', 'googlepdns', 'cloudflare', \
            'opendns', 'quad9', 'level3', 'neustar', 'he' ]
        self.user_ids = set()

    class pivoted_record:
        def __init__(self,x):
            qt = x['query_time']
            tag = x['resolver_tag']
            self.query_AS = x['query_AS']
            self.query_user_id = x['query_user_id']
            self.first_tag = tag
            self.first_time = qt
            self.rsv_times = dict()
            self.rsv_times[tag] = qt


    def compute_times(self, x):
        uid = x['query_user_id']
        key = uid
        qt = x['query_time']
        tag = x['resolver_tag']
        if key in self.rqt:
            if (not tag in self.rqt[key].rsv_times) or \
                qt < self.rqt[key].rsv_times[tag]:
                self.rqt[key].rsv_times[tag] = qt
            if qt < self.rqt[key].first_time:
                self.rqt[key].first_tag = tag
                self.rqt[key].first_time = qt
        else:
            self.rqt[key] = pivoted_per_query.pivoted_record(x)
            if not uid in self.user_ids:
                self.user_ids.add(uid)

    def process_log(self, df):
        df.apply(lambda x: self.compute_times(x), axis=1)

    def get_frame_delta_t(self):
        # compose the headers
        headers = [ 'query_AS', \
            'query_user_id', \
            'first_tag', \
            'first_time' ]
        for tag in self.tags:
            headers.append(tag)
        # create a list of rows
        rl = []
        for key in self.rqt:
            qt = self.rqt[key]
            r = [ \
               qt.query_AS, \
               qt.query_user_id, \
               qt.first_tag, \
               qt.first_time ]
            for tag in self.tags:
                t = 1000000.0 + qt.first_time
                if tag in qt.rsv_times:
                    r.append(qt.rsv_times[tag] - qt.first_time)
                else:
                    r.append(1000000)
            rl.append(r)
        # create a data frame
        df = pd.DataFrame(rl,columns=headers)
        return df

    def get_frame_asns(self, first_only):
        # compose the headers
        headers = [ 'q_AS', \
            'uids',
            'q_total',
            'isp',
            'public',
            'both' ]
        for tag in self.tags:
            headers.append(tag)
        xt = dict()
        uis = dict()
        for key in self.rqt:
            qt = self.rqt[key]
            
            if not qt.query_AS in xt:
                r = [ \
                    qt.query_AS,
                    0,
                    0,
                    0,
                    0,
                    0
                ]
                for tag in self.tags:
                    r.append(0)
                xt[qt.query_AS] = r
                uis[qt.query_AS] = set()
            xt[qt.query_AS][2] += 1
            if not qt.query_user_id in uis[qt.query_AS]:
                uis[qt.query_AS].add(qt.query_user_id)
                xt[qt.query_AS][1] += 1
            if qt.category == 'isp':
                xt[qt.query_AS][3] += 1
            elif qt.category == 'public':
                xt[qt.query_AS][4] += 1
            else:
                xt[qt.query_AS][5] += 1
            rank = 6
            for tag in self.tags:
                if (tag == qt.first_tag) or ((first_only == 0) and (tag in qt.rsv_times)):
                    xt[qt.query_AS][rank] += 1
                rank += 1
        xl = list(xt.values())
        xl.sort(key=lambda x: x[1], reverse=True)
        df = pd.DataFrame(xl,columns=headers)
        return df




        
# Prepare a plot of times per queries, one line per mode.

#def plot_times(df, tags):
#    for tag in tags:
#        dft = df[tag]
#        dftg = dft[
#    colors = [ "black", "blue", "green", "orange", "red", "purple", "magenta", "brown", "violet", "yellow"]
#    axa = without_apnic_df.plot.scatter(x="queries", y="n_slds", alpha=0.5, logx=True, logy=True, color="blue")
#small_apnic_df.plot.scatter(ax=axa, x="queries", y="n_slds", alpha=0.5, color="orange")
#big_apnic_df.plot.scatter(ax=axa, x="queries", y="n_slds", alpha=0.5, color="red")
#plt.show()









