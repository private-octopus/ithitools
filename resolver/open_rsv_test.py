import sys
import open_rsv

open_rsv_test_table = [
    # start with a series of tests by AS, using an IP address below the first range
    ["1.2.3.4", "AS12007", "" ],
    ["1.2.3.4", "AS12008","neustar"],
    ["1.2.3.4", "AS12009", "" ],
    ["1.2.3.4", "AS131400","dnswatch"],
    ["1.2.3.4", "AS131620",""],
    ["1.2.3.4", "AS131621","twnic"],
    ["1.2.3.4", "AS131622",""],
    ["1.2.3.4", "AS132203","dnspod"],
    ["1.2.3.4", "AS13334",""],
    ["1.2.3.4", "AS13335","cloudflare"],
    ["1.2.3.4", "AS13336",""],
    ["1.2.3.4", "AS15169","googlepdns"],
    ["1.2.3.4", "AS1679",""],
    ["1.2.3.4", "AS1680","greenteamdns"],
    ["1.2.3.4", "AS1681",""],
    ["1.2.3.4", "AS204136","opennic"],
    ["1.2.3.4", "AS23274","dnspai"],
    ["1.2.3.4", "AS23393","comodo"],
    ["1.2.3.4", "AS23724","onedns"],
    ["1.2.3.4", "AS31400","dnswatch"],
    ["1.2.3.4", "AS33517","dyn"],
    ["1.2.3.4", "AS4808","onedns"],
    ["1.2.3.4", "AS4812","dnspai"],
    ["1.2.3.4", "AS51453","freedns"],
    ["1.2.3.4", "AS57926","safedns"],
    ["1.2.3.4", "AS60679","freedomworld"],
    ["1.2.3.4", "AS8551","greenteamdns"],
    ["1.2.3.4", "AS8552",""],
    # follow by a set of IPv4 address tests, using an AS larger than max or lower than min
    ["7.7.7.7", "AS1111", "" ],
    ["8.0.0.1", "AS9999","level3"],
    ["8.0.255.255", "AS9999","level3"],
    ["8.1.1.1", "AS1111", "" ],
    ["23.236.73.15", "AS9999", "cleanbrowsing"],
    ["23.253.163.53", "AS9999", "alternatedns"],
    ["66.102.36.192", "AS9999", "quad9"],
    ["66.102.36.193", "AS9999", "quad9"],
    ["66.102.36.194", "AS9999", "quad9"],
    ["216.218.128.1", "AS1111","he"],
    ["217.30.80.0", "AS1111","vrsgn"],
    ["218.99.143.0", "AS1111",""],
    ["219.99.143.0", "AS1111","freenom"],
    ["221.99.143.0", "AS1111",""],
    # follow by a set of IPv6 address tests, using an AS larger than max or lower than min
    ["1001::1", "AS1111",""],
    ["2001:470::1", "AS1111","he"],
    ["2001:500:15:200::1", "AS1111","quad9"],
    ["2001:500:15:300::1", "AS1111",""],
    ["2001:500:15:400::1", "AS1111","quad9"],
    ["2404:6800:4000::1", "AS1111","googlepdns"],
    ["2404:6800:4001::1", "AS1111",""],
    ["2404:6800:4003::f;f", "AS1111",""],
    ["2a04:e4c0:31::2", "AS1111","opendns"],
    ["2a04:e4c0:40::abc", "AS1111","opendns"],
    ["2a0c:e080:0:3::1", "AS1111","quad9"],
    ["3456::1", "AS1111",""],
    # Then a couple of malformed addresses
    ["7.7.a.7", "AS1111", "" ],
    ["1001:1;2;3", "AS1111",""],
]


if len(sys.argv) == 4 and sys.argv[1] == "print": 
    open_rsv.table_print(sys.argv[1])
else:
    # check that a series of test cases provides the expected value
    passing = True
    for test in open_rsv_test_table:
        rsv = open_rsv.get_open_rsv(test[0], test[1])
        if rsv != test[2]:
            print("get_open_rsv(" + test[0] + "," + test[1] + ") returns \"" + rsv + "\", expected \"" + test[2] + "\"")
            passing = False
            rsv_as = open_rsv.get_open_rsv_from_AS(test[1])
            if test[0] == "1.2.3.4":
                if rsv_as != test[2]:
                    print("get_open_rsv_from_AS("  + test[1] + ") returns \"" + rsv_as + "\", expected \"" + test[2] + "\"")
                    break
            elif rsv_as != "":
                print("get_open_rsv_from_AS("  + test[1] + ") returns \"" + rsv_as + "\", expected \"" + test[2] + "\"")
            if rsv_as == "":
                rsv_ip = open_rsv.get_open_rsv_from_IP(test[0])
                print("get_open_rsv_from_IP("  + test[0] + ") returns \"" + rsv_ip + "\", expected \"" + test[2] + "\"")
            break
    if not passing:
        print("Fail.")
        exit(-1)
    else:
        print("Success.")
        exit(0)






