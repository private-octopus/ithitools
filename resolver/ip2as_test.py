# ip2as_test
# Check that addresses can be correctly resolver.

import sys
import os 
from pathlib import Path
import ip2as
import ipaddress
import time
import pandas as pd

def usage():
    print("Usage: ip2as_test outdir test_ip42as test_ip62as 1*as0")

def try_map(ip_text, dbg=False):
    if ":" in ip_text:
        asn = ip2a6.get_asn(ip_text, dbg=dbg)
    else:
        asn = ip2a4.get_asn(ip_text, dbg=dbg)
    return asn

def try_row(x, totals, F):
    subnet = x['subnet']
    sub_part = subnet.split('/')
    ip_text = sub_part[0]
    ix = 0
    if ":" in ip_text:
        ix = 2
    totals[ix] += 1
    asn = try_map(ip_text)
    if asn == 0 :
        totals[ix+1] += 1
        F.write(ip_text + "," + str(asn) + "," + str(x['uids']) + "\n")

# main
if __name__ == "__main__":
    time_start = time.time()
    if len(sys.argv) < 4:
        usage()
        exit(-1)

    outdir = sys.argv[1]
    test_ip42as = sys.argv[2]
    test_ip62as = sys.argv[3]
    as0_reports = sys.argv[4:]

    # get the as to ip tables
    ip2a4 = ip2as.ip2as_table()
    ip2a4.load(test_ip42as)
    ip2a6 = ip2as.ip2as_table()
    ip2a6.load(test_ip62as)
    time_loaded = time.time()

    # try to resolve test IP
    fail_file = os.path.join(outdir, "failing.csv")
    with open(fail_file,"w") as F:
        F.write("IP, ASN, oids\n")
        totals = [ 0, 0, 0, 0 ]
        for as0_report in as0_reports:
            df = pd.read_csv(as0_report)
            df.apply(lambda row: try_row(row, totals, F),axis=1)
    print("Tried(v4): " + str(totals[0]) + ", as0: " + str(totals[1]))
    print("Tried(v6): " + str(totals[2]) + ", as0: " + str(totals[3]))