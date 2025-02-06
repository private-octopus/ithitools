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

import ipaddress
from math import nan
from ssl import ALERT_DESCRIPTION_DECRYPT_ERROR
import country
import traceback
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
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


# pivot per query and per AS, produce a dictionary with
# one table per AS, containing the queries for that AS

tag_list = [ 'Same_AS', 'Same_CC', 'Others', 'googlepdns', 'cloudflare', \
            'opendns', 'quad9', 'level3', 'neustar', 'he' ]
tag_isp_set = set(['Same_AS', 'Same_CC', 'Others'])
color_list = [ 'blue', 'magenta', 'indigo', 'green', 'orange', 'red', 'violet', 'yellow', 'yellow', 'yellow', 'yellow' ]
dot_headers = [ 'rsv_type', 'rank', 'first_time', 'delay' ]

class pivoted_record:
    # Record is created for the first time an event appears in an AS record.
    def __init__(self,x):
        qt = x['query_time']
        tag = x['resolver_tag']
        self.query_AS = x['query_AS']
        self.query_user_id = x['query_user_id']
        self.first_tag = tag
        self.first_time = qt
        self.rsv_times = dict()
        self.rsv_times[tag] = qt
        self.delta_times = dict()
        self.has_isp = False
        self.has_public = False

    # add event records a new event after the tag has been created
    def add_event(self, x):
        qt = x['query_time']
        tag = x['resolver_tag']
        if (not tag in self.rsv_times) or \
            qt < self.rsv_times[tag]:
            self.rsv_times[tag] = qt
        if qt < self.first_time:
            self.first_tag = tag
            self.first_time = qt

    # delta time is computed once all events are recorded
    # We only consider the events that happen less that "delta_max"
    # (default= 0.5 second) from the first event. This cuts down the
    # noise of, for example, queries repeated to maintain a cache
    def compute_delta_t(self, delta_max = 0.5):
        for tag in self.rsv_times:
            delta_t = self.rsv_times[tag] - self.first_time
            if delta_t <= delta_max:
                self.delta_times[tag] = delta_t
                if tag in tag_isp_set:
                    self.has_isp = True
                else:
                    self.has_public = True
            
class pivoted_AS_record:
    def __init__(self,query_AS):
        self.query_AS = query_AS
        self.rqt = dict()
        self.user_ids = set()
        self.nb_isp = 0
        self.nb_public = 0
        self.nb_both = 0
        self.nb_total = 0

    # Compute time is called for each record "x" in the log file for that AS.
    # For each UID, we compute a pivoted record, which contains a dict() of "tags".
    # If a tag is present in the dict, we only retain the earliest time for that ta
    def compute_time(self, x):
        uid = x['query_user_id']
        key = uid

        if key in self.rqt:
            self.rqt[key].add_event(x)
        else:
            self.rqt[key] = pivoted_record(x)
            if not uid in self.user_ids:
                self.user_ids.add(uid)
    
    # Delta updates the per query record to compute the delta between the
    # arrival in a given category and the first query for that UID.
    # This computation can only be performed once all records have been logged.
    def compute_delta_t(self, delta_max = 0.5):
        for key in self.rqt:
            self.rqt[key].compute_delta_t(delta_max=delta_max)
            self.nb_total += 1
            if self.rqt[key].has_public:
                if self.rqt[key].has_isp:
                    self.nb_both += 1
                else:
                    self.nb_public += 1
            else:
                self.nb_isp += 1

    # Produce a one line summary record for the ASN   
    # Return a list of values:
    # r[0] = ASN
    # r[1] = total number of UIDs
    # r[2] = total number of queries (should be same as total number UIDs)
    # r[3] = total number of ISP only queries
    # r[4] = total number of public DNS only queries
    # r[5] = total number of queries served by both ISP and public DNS
    # r[6]..[5+N] = total number of queries served by a given category

    def get_summary(self, first_only):
        r = [
            self.query_AS,
            len(self.user_ids),
            self.nb_total,
            self.nb_isp,
            self.nb_public,
            self.nb_both
        ]
        for tag in tag_list:
            r.append(0)

        for key in self.rqt:
            rqt_r = self.rqt[key]
            rank = 6
            for tag in tag_list:
                if tag in rqt_r.rsv_times:
                    r[rank] += 1
                rank += 1
        return r

    # get_delta_t_both:
    # we produce a list of dots" records suitable for statistics and graphs
    def get_delta_t_both(self):
        dots = []
        for key in self.rqt:
            rqt_r = self.rqt[key]
            if len(rqt_r.delta_times) == 0:
                rqt_r.compute_delta_times()
            if rqt_r.has_public and rqt_r.has_isp:
                n_both = 0
                for tag in rqt_r.delta_times:
                    n_both += 1
                    dot_line = [ tag, n_both,  rqt_r.first_time, rqt_r.delta_times[tag]]
                    dots.append(dot_line)
        dot_df = pd.DataFrame(dots,columns=dot_headers)
        return dot_df

class pivoted_per_query:
    def __init__(self):
        self.ASes = dict()
        self.tried = 0
            
    def compute_times(self, x):
        query_AS = x['query_AS']

        if not query_AS in self.ASes:
            self.ASes[query_AS] = pivoted_AS_record(query_AS)

        self.ASes[query_AS].compute_time(x)

    def process_log(self, df):
        df.apply(lambda x: self.compute_times(x), axis=1)

    def AS_list(self):
        return list(self.ASes.keys())

    def compute_delta_t(self):
        for query_AS in self.ASes:
            self.ASes[query_AS].compute_delta_t()

    def get_summaries(self, AS_list, first_only):
        # compose the headers
        headers = [ 'q_AS', \
            'uids',
            'q_total',
            'isp',
            'public',
            'both' ]
        for tag in tag_list:
            headers.append(tag)
        s_list = []
        for target_AS in AS_list:
            if target_AS in self.ASes:
                s_list.append(self.ASes[target_AS].get_summary(first_only))
        s_list.sort(key=lambda x: x[1], reverse=True)
        df = pd.DataFrame(s_list,columns=headers)
        return df

    def get_delta_t_both(self, target_AS):
        return self.ASes[target_AS].get_delta_t_both()

def do_graph(asn, dot_df, image_file="", x_delay=False, log_y=False):
    if log_y:
        # replace 0 by low value so logy plots will work
        dot_df.loc[dot_df['delay'] == 0, 'delay'] += 0.00001
    is_first = True
    sub_df = []
    x_value = "rank"
    if x_delay:
        x_value = "first_time"

    for rsv in tag_list:
        sub_df.append(dot_df[dot_df['rsv_type'] == rsv])

    legend_list = []
    for i in range(0, len(tag_list)):
        rsv = tag_list[i]
        rsv_color = color_list[i]
        if len(sub_df[i]) > 0:
            if is_first:
                axa = sub_df[i].plot.scatter(x=x_value, y="delay", logy=log_y, alpha=0.25, color=rsv_color)
            else:
                sub_df[i].plot.scatter(ax=axa, x=x_value, y="delay", logy=log_y, alpha=0.25, color=rsv_color)
            is_first = False
            legend_list.append(rsv)
    plt.title("Delay (seconds) per provider for " + asn)
    plt.legend(legend_list)
    if len(image_file) == 0:
        plt.show()
    else:
        plt.savefig(image_file)
    plt.close()

    
def do_hist(asn, dot_df, image_file):
    # get a frame from the list
    dot_df.loc[dot_df['delay'] == 0, 'delay'] += 0.00001
    is_first = True
    clrs = []
    legend_list = []
    row_list = []
    x_min = 1000000
    x_max = 0.00001

    for i in range(0, len(tag_list)):
        rsv = tag_list[i]
        sdf_all = dot_df[dot_df['rsv_type'] == rsv]
        sdf = sdf_all['delay']
        sdf_max = sdf.max()
        if sdf_max > x_max:
            x_max = sdf_max
        sdf_min = sdf.min()
        if sdf_min < x_min:
            x_min = sdf_min
        l = sdf.values.tolist()
        if len(l) > 0:
            row_list.append(np.array(l))
            clrs.append(color_list[i])
            legend_list.append(rsv)
            is_first = False
    if x_min == 0:
        x_min = 0.00001

    if not is_first:
        logbins = np.logspace(np.log10(x_min),np.log10(x_max), num=20)
        axa = plt.hist(row_list, logbins, histtype='bar', color=clrs)
        plt.title("Histogram of delays (seconds) per provider for " + asn)
        plt.legend(legend_list)
        plt.xscale('log')
        if len(image_file) == 0:
            plt.show()
        else:
            plt.savefig(image_file)
        plt.close()








