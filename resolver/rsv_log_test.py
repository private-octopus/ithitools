# APNIC test.
# verify that the parse library does what we expect
import sys
import ip2as
import rsv_log_parse
import rsv_both_graphs
import pandas as pd
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

def frame_test():
    passing = True
    m = []
    for i in range(0,len(test_lines)):
        line = test_lines[i]
        try:
            x = rsv_log_parse.rsv_log_line()
            line = test_lines[i]
            if x.parse_line(line):
                m.append(x.row())
        except Exception as exc:
            traceback.print_exc()
            print('\nCode generated an exception: %s' % (exc))
            print("Cannot parse:\n" + line + "\n")
            passing = False
            break
    print("Matrix has " + str(len(m)) + " lines.")
    df = pd.DataFrame(m,columns=rsv_log_parse.rsv_log_line.header())
    print(df)

def get_name(as_names, x):
    y = as_names.name(x)
    if len(y) == 0:
        y = "???"
    return y;

def create_as_frame(numpy_as, as_names):
    header = [ \
        'resolver_AS', \
        'resolver_name', \
        'count']
    m = []
    as_counts = dict()
    bad_names = 0

    for r in numpy_as:
        as_v = r[0]
        if not as_v in as_counts:
            as_counts[as_v] = 1
        else:
            as_counts[as_v] += 1
    for as_v in as_counts:
        n = 0
        name = as_names.name(as_v)
        if name == "":
            name = as_v
        r = [ as_v, name, as_counts[as_v] ]
        m.append(r)
    df = pd.DataFrame(m,columns=header)
    return df

            

# Main program
if __name__ == "__main__":
    if len(sys.argv) < 2:
        if not parse_test():
            print("test fails.")
            exit(-1)
        frame_test()

    if len(sys.argv) >= 6:
        ip2a4 = ip2as.ip2as_table()
        ip2a4.load(sys.argv[1])
        ip2a6 = ip2as.ip2as_table()
        ip2a6.load(sys.argv[2])
        as_names = ip2as.asname()
        as_names.load(sys.argv[3])
        rsv_table = rsv_log_parse.rsv_log_file()
        rsv_table.load(sys.argv[4], ip2a4, ip2a6, as_names, experiment=['0du'], rr_types = [])
        df = rsv_table.get_frame()
        print("DF:")
        print(df.head(10))
        # list the top ISPs
        isps = df['query_AS']
        ispdf = isps.to_frame()
        ispvc = ispdf['query_AS'].value_counts()
        print("ISPVC:")
        print(ispvc)
        # list the top open DNS
        odns = df['resolver_tag']
        odnsdf = odns.to_frame()
        odnsc = odnsdf['resolver_tag'].value_counts()
        print("ODNSC:")
        print(odnsc)

        print("ODNSU:")
        odnsu = odns.unique()
        print(odnsu)
        
        
        # list the top experiments
        expt = df['experiment_id']
        expdf = expt.to_frame()
        exptc = expdf['experiment_id'].value_counts()
        print("EXPTC:")
        print(exptc)
        print("EXPTU:")
        exptu = expt.unique()
        print(exptu)
        
        ppq = rsv_log_parse.pivoted_per_query()
        ppq.process_log(df)
        dfdt = ppq.get_frame_delta_t()
        print("DFDT:")
        print(dfdt)
        # dfdt.to_csv(sys.argv[5], sep=",")

        as_list = dfdt['query_AS'].unique()
        as_res = []
        for asn in as_list:
            rbg = rsv_both_graphs.per_as_analysis(asn, dfdt)
            rbg.compute_both()
            l = rbg.to_list()
            #print(str(l))
            as_res.append(l)
            if rbg.n_both > 50:
                #image_file = "..\\tmp\\delays_log_" + asn
                #rbg.do_graph(image_file, x_delay=True, log_y=True)
                image_file = "..\\tmp\\delays_log_hist_" + asn
                rbg.do_hist(image_file=image_file)

        as_df = pd.DataFrame(as_res,columns=rsv_both_graphs.per_as_analysis.list_headers())
        as_df.to_csv(sys.argv[5], sep=",")

        # list the top RR types
        qt = df['rr_type']
        qtdf = qt.to_frame()
        qtsc = qtdf['rr_type'].value_counts()
        print("QTSC:")
        print(qtsc)

        #print the number of user ids:
        #print("nb_uids: " + str(len(ppq.user_ids)))
        #qui = df['query_user_id']
        #quiu = qui.unique()
        #print("nb_uids: " + str(quiu.shape[0]))

        

    exit(0)


