# ip2as_test
# Check that addresses can be correctly resolver.

import sys
import os
from pathlib import Path
import ip2as
import ipaddress
import time


# main
if __name__ == "__main__":
    time_start = time.time()
    if len(sys.argv) < 2:
        usage()
        exit(-1)

    test_ip = sys.argv[1:]

    # get the as name tables
    source_path = Path(__file__).resolve()
    resolver_dir = source_path.parent
    auto_source_dir = resolver_dir.parent
    print("Auto source path is: " + str(auto_source_dir) + " (source: " + str(source_path) + ")")
    source_dir = os.path.join(auto_source_dir, "data") 
    ip2a4_file = os.path.join(source_dir, "ip2as.csv") 
    ip2a6_file = os.path.join(source_dir, "ip2asv6.csv")
    as_names_file = os.path.join(source_dir, "as_names.csv")   
    ip2a4 = ip2as.ip2as_table()
    ip2a4.load(ip2a4_file)
    ip2a6 = ip2as.ip2as_table()
    ip2a6.load(ip2a6_file)
    as_names = ip2as.asname()
    as_names.load(as_names_file)
    time_loaded = time.time()

    # try to resolve test IP

    for ip_text in test_ip:
        if ":" in ip_text:
            print("Try v6")
            asn = ip2a6.get_asn(ip_text, dbg=True)
        else:
            print("Try v4")
            asn = ip2a4.get_asn(ip_text, dbg=True)
        print(ip_text + ": " + str(asn))

