# APNIC test.
# verify that the parse library does what we expect

import rsv_log_parse
import traceback

test_lines = [
    "1730423099.986047 client 2001:558:fe13:d:76:96:22:147#20318: query: 0di-u19783a8a-c233-a1ef2-s1730423099-i00000000-0.am.dotnxdomain.net. IN DNSKEY -ED () 1914810962 0",
    "1730423099.986954 client 184.178.229.12#40985: query: 0du-results-u4d03cba4-c233-a58f5-s1730423089-i00000000-0.am.dotnxdomain.net. IN AAAA -E () 1914810962 0",
    "1730423099.987429 client 2400:cb00:72:1024::a29e:8965#48184: query: 0ds-u7a16847d-c233-a58f5-s1730423099-i00000000-0.am.dotnxdomain.net. IN DS -ED () 1914810962 0",
    "1730423099.987980 client 2001:558:fe13:d:76:96:22:129#32766: query: 0di-u19783a8a-c233-a1ef2-s1730423099-i00000000-0.am.dotnxdomain.net. IN DNSKEY -ED () 1914810962 0",
    "1730423099.988488 client 76.96.15.79#40367: query: 0du-results-u255c79d1-c233-a1ef2-s1730423089-i00000000-0.am.dotnxdomain.net. IN DS -ED () 1914810962 0",
    "1730423099.989316 client 141.101.75.101#62737: query: firmware.am.dotnxdomain.net. IN DNSKEY -ED () 1914810962 0",
    "1730422800.028111 client 173.194.97.25#53973: query: 0di-u5cbd23ce-c238-a409b4-s1730422799-ibe78f991-0.la.dotnxdomain.net. IN DS -ED (C) -1293065646 0",
    "1730422800.029338 client 172.217.37.19#50074: query: 0du-uf8c0aa51-c185-a40625-s1730422799-iaa54ac12-0.la.dotnxdomain.net. IN A -EDS4/24/0|170.84.172.0 (C) -1293065646 0",
    "1730422800.029452 client 172.217.35.147#52166: query: 06u-uf8c0aa51-c185-a40625-s1730422799-iaa54ac12-0.la.dotnxdomain.net. IN DS -ED (C) -1293065646 0",
    "1730422800.030165 client 172.253.234.24#50729: query: 04u-uf8c0aa51-c185-a40625-s1730422799-iaa54ac12-0.la.dotnxdomain.net. IN DS -ED (C) -1293065646 0",
    "1736378116.030165 client 172.253.234.24#50729: query: 000-000-000a-0000-0006-e7b5bab7-233-a55A8-1736378116-ac380eb6-0.am2.dotnxdomain.net. IN AAAA -E () 1914810962 0",
    "1730423099.989316 client 141.101.75.101#62737: query: valid.starnxdomain.net. IN A -ED () 1914810962 0",
    "1730423099.989316 client 141.101.75.101#62737: query: invalid4.starnxdomain.net. IN A -ED () 1914810962 0",
    "1730423099.989316 client 141.101.75.101#62737: query: invalid6.starnxdomain.net. IN A -ED () 1914810962 0",
]

test_results = [
    "1730423099.986047, 2001:558:fe13:d:76:96:22:147, 20318, 0di, u19783a8a, US, AS7922, 1730423099, 0.0.0.0, IN, DNSKEY, am.dotnxdomain.net, , , , , \"-ED () 1914810962 0\", ",
    "1730423099.986954, 184.178.229.12, 40985, 0du, u4d03cba4, US, AS22773, 1730423089, 0.0.0.0, IN, AAAA, am.dotnxdomain.net, R, , , , \"-E () 1914810962 0\", ",
    "1730423099.987429, 2400:cb00:72:1024::a29e:8965, 48184, 0ds, u7a16847d, US, AS22773, 1730423099, 0.0.0.0, IN, DS, am.dotnxdomain.net, , , , , \"-ED () 1914810962 0\", ",
    "1730423099.98798, 2001:558:fe13:d:76:96:22:129, 32766, 0di, u19783a8a, US, AS7922, 1730423099, 0.0.0.0, IN, DNSKEY, am.dotnxdomain.net, , , , , \"-ED () 1914810962 0\", ",
    "1730423099.988488, 76.96.15.79, 40367, 0du, u255c79d1, US, AS7922, 1730423089, 0.0.0.0, IN, DS, am.dotnxdomain.net, R, , , , \"-ED () 1914810962 0\", ",
    "1730423099.989316, 141.101.75.101, 62737, , , , , 0, , IN, DNSKEY, am.dotnxdomain.net, , , C, , \"-ED () 1914810962 0\", firmware",
    "1730422800.028111, 173.194.97.25, 53973, 0di, u5cbd23ce, VE, AS264628, 1730422799, 190.120.249.145, IN, DS, la.dotnxdomain.net, , , , , \"-ED (C) -1293065646 0\", ",
    "1730422800.029338, 172.217.37.19, 50074, 0du, uf8c0aa51, PY, AS263717, 1730422799, 170.84.172.18, IN, A, la.dotnxdomain.net, , , , , \"-EDS4/24/0|170.84.172.0 (C) -1293065646 0\", ",
    "1730422800.029452, 172.217.35.147, 52166, 06u, uf8c0aa51, PY, AS263717, 1730422799, 170.84.172.18, IN, DS, la.dotnxdomain.net, , , , , \"-ED (C) -1293065646 0\", ",
    "1730422800.030165, 172.253.234.24, 50729, 04u, uf8c0aa51, PY, AS263717, 1730422799, 170.84.172.18, IN, DS, la.dotnxdomain.net, , , , , \"-ED (C) -1293065646 0\", ",
    "1736378116.030165, 172.253.234.24, 50729, 000-000-000a-0000-0006, e7b5bab7, US, AS21928, 1736378116, 172.56.14.182, IN, AAAA, am2.dotnxdomain.net, , A, , , \"-E () 1914810962 0\", ",
    "1730423099.989316, 141.101.75.101, 62737, , , , , 0, , IN, A, valid.starnxdomain.net, , , , S, \"-ED () 1914810962 0\", ",
    "1730423099.989316, 141.101.75.101, 62737, , , , , 0, , IN, A, invalid4.starnxdomain.net, , , , S, \"-ED () 1914810962 0\", ",
    "1730423099.989316, 141.101.75.101, 62737, , , , , 0, , IN, A, invalid6.starnxdomain.net, , , , S, \"-ED () 1914810962 0\", ",
]

def parse_test():
    passing = True
    for i in range(0,len(test_lines)):
        line = test_lines[i]
        try:
            x = rsv_log_parse.rsv_log_line()
            if not x.parse_line(line):
                print("Cannot parse: " + line)
                passing = False
                break;
            else:
                s = x.pretty_string()
                print(s)
                if not s == test_results[i]:
                    print("Expected:\n"+test_results[i])
                    print("for:\n" + line)
                    passing = False
                    if len(s) != len(test_results[i]):
                        print("Got " + str(len(s)) + " chars instead of " + str(len(test_results[i])))
                    else:
                        t = test_results[i]
                        for j in range(0, len(s)):
                            if not s[j] == t[j]:
                                print("differ at s[" + str(j) + "]")
                                print(s[j:] + "\nvs.:\n" + t[j:])
                    break;
        except Exception as exc:
            traceback.print_exc()
            print('\nCode generated an exception: %s' % (exc))
            print("Cannot print or parse:\n" + line + "\n")
            passing = False
            break
    return passing



# Main program

if not parse_test():
    print("test fails.")
    exit(-1)
exit(0)


