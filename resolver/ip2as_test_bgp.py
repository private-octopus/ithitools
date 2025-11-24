# ip2as_test
# Check that addresses can be correctly resolver.

import sys
import os 
from pathlib import Path
import ip2as
import ipaddress
import time
import pandas as pd
import traceback

def usage():
    print("Usage: ip2as_test_bgp outdir failing bgp_table4 bgp_table6")

class fail_row:
    def __init__(self, subnet_text, uids):
        self.subnet = ipaddress.ip_network(subnet_text)
        self.uids = uids
        self.is_matched = False
        self.matching = self.subnet

def check_table(bgp_file, fail_table):
    sys.stdout.write('Processing ' + bgp_file + '\n')
    sys.stdout.flush()
    nb_line = 0
    nb_bad_line = 0
    previous_prefix = ""
    for line in open(bgp_file , "rt"):
        nb_line += 1
        strip_line = line.strip()
        
        parts = strip_line.split(" ")
        prefix = parts[0]
        if prefix.startswith('>'):
            prefix = prefix[1:]
        if prefix == previous_prefix:
            continue
        previous_prefix = prefix
        if prefix == '::/0':
            continue
        try:
            subnet = ipaddress.ip_network(prefix)
            for i in range(0, len(fail_table)):
                if not fail_table[i].is_matched and \
                    (fail_table[i].subnet == subnet or \
                    subnet.supernet_of(fail_table[i].subnet)):
                        fail_table[i].is_matched = True
                        fail_table[i].matching = subnet
        except Exception as exc:
            traceback.print_exc()
            print("Fail for " + prefix + ", line: \n" + line)
            if nb_line >= 25:
                break
        if nb_line%10000 == 0:
            sys.stdout.write('.')
            sys.stdout.flush()
    sys.stdout.write('\n')
    sys.stdout.flush()


def failing_from_df(x, fail_table4, fail_table6):
    r = fail_row(x['subnet'], x['uids'])
    if r.subnet.version == 4:
        fail_table4.append(r)
    else:
        fail_table6.append(r)


# main
if __name__ == "__main__":
    time_start = time.time()
    if len(sys.argv) < 5:
        usage()
        exit(-1)

    outdir = sys.argv[1]
    bgp_table4 = sys.argv[2]
    bgp_table6 = sys.argv[3]
    failing = sys.argv[4]

    # load the failing addresses in two table
    df_fail = pd.read_csv(failing)
    fail_table4 = []
    fail_table6 = []
    df_fail.apply(lambda x: failing_from_df(x, fail_table4, fail_table6), axis=1)

    # check the tables
    #check_table(bgp_table4, fail_table4)
    check_table(bgp_table6, fail_table6)

    # compute the results 

    matched = 0
    
    fail_file = os.path.join(outdir, "still_failing_v6.csv")
    with open(fail_file,"w") as F:
        F.write("Subnet, uids, matching\n")

        #matched = 0
        #for r in fail_table4:
        #    match_str = ""
        #    if r.is_matched:
        #        matched += 1
        #        match_str = str(r.matching)
        #    F.write(str(r.subnet) + "," + str(r.uids) + "," + match_str + "\n")
        #print("IPv4, " + str(len(fail_table4)) + ", matched: " + str(matched))
    
        matched = 0
        for r in fail_table6:
            match_str = ""
            if r.is_matched:
                matched += 1
                match_str = str(r.matching)
            F.write(str(r.subnet) + "," + str(r.uids) + "," + match_str + "\n")
        print("IPv6, " + str(len(fail_table6)) + ", matched: " + str(matched))




