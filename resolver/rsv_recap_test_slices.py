# test file. Verify that recap2 is computed correctly
#
# Two tests:
# - the sum of columns 'nb_A_u300ms_ISP', 'nb_A_u1s_ISP', 'nb_A_u3s_ISP', 'nb_A_u10s_ISP', and 'nb_A_u30s_ISP' should be equal to 'nb_A_ISP'
# - the column 'nb_A_u300ms_ISP' should be larger than the sum of columns 'A_ISP_only',  'A_ISP_PDNS', ' A_ISP_others', 'A_all3'
import sys
import os
import pandas as pd

#import rsv_both_graphs
import pandas as pd
isp_A_slices = [
            'nb_A_0ms_ISP',
            'nb_A_u10ms_ISP',
            'nb_A_u30ms_ISP',
            'nb_A_u100ms_ISP',
            'nb_A_u300ms_ISP', 'nb_A_u1s_ISP', 'nb_A_u3s_ISP',
            'nb_A_u10s_ISP', 'nb_A_u30s_ISP']
isp_A_uids =  ['A_ISP_only', 'A_ISP_PDNS', 'A_ISP_others', 'A_all3']

nb_A_0ms = [ 'nb_A_0ms_ISP', 'nb_A_0ms_PDNS', 'nb_A_0ms_others']



delay_columns = [
    ['sum_deltas_A_PDNS_ISP', 'uids_A_PDNS_ISP', 'average_A_PDNS_ISP'],
    ['sum_deltas_A_others_ISP', 'uids_A_others_ISP', 'average_A_others_ISP'],
    ['sum_deltas_AAAA_PDNS_ISP', 'uids_AAAA_PDNS_ISP', 'average_AAAA_PDNS_ISP'],
    ['sum_deltas_AAAA_others_ISP', 'uids_AAAA_others_ISP', 'average_AAAA_others_ISP'],
    ['sum_deltas_HTTPS_PDNS_ISP', 'uids_HTTPS_PDNS_ISP', 'average_HTTPS_PDNS_ISP'],
    ['sum_deltas_HTTPS_others_ISP', 'uids_HTTPS_others_ISP', 'average_HTTPS_others_ISP']]

uids_columns = [
    [ 'A_ISP_PDNS', 'A_all3', 'uids_A_PDNS_ISP' ],
    [ 'A_ISP_others', 'A_all3', 'uids_A_others_ISP' ]]

def check_average(r, headers):
    if r[headers[1]] > 0:
        average = r[headers[0]] / r[headers[1]]
        if abs(average - r[headers[2]]) > 0.000001:
            print("average " + headers[2] + " = " + str(r[headers[2]]) +
                  " != sum_deltas/uids = " + str(average))
            print(r)
            exit()
    elif r[headers[2]] != 0:
        print("average " + headers[2] + " = " + str(r[headers[2]]) +
              " != 0 when uids is 0.")
        print(r)
        exit()

def check_uids(r, headers):
    uids = r[headers[0]] + r[headers[1]]
    if uids != r[headers[2]]:
        print("uids " + headers[2] + " = " + str(r[headers[2]]) +
              " != sum of columns " + headers[0] + " and " + headers[1] +
              " = " + str(uids))
        print(r)
        exit()

def check_row(r):
    sum_slices = 0
    sum_uids = 0
    sum_0ms = 0
    for key in isp_A_slices:
        sum_slices += r[key]
    if sum_slices != r['nb_A_ISP']:
        print("r[nb_A_ISP] = " + str(r['nb_A_ISP']) + " != sum of slices " + str(sum_slices))
        print(r)
        exit()
    for key in isp_A_uids:
        sum_uids += r[key]
    if sum_uids > sum_slices:
        print("sum of slices = " + str(r['sum_slices']) + " < sum of A uids " + str(sum_uids))
        print(r)
        exit()
    for key in isp_A_uids:
        sum_0ms += r[key]
    if sum_0ms > sum_slices:
        print("nb_A = " + str(r['nb_A']) + " != sum of 0ms slices " + str(sum_0ms))
        print(r)
        exit()
    for headers in uids_columns:
        check_uids(r, headers)
    for headers in delay_columns:
        check_average(r, headers)


#
print("Testing: " + sys.argv[1])
df = pd.read_csv(sys.argv[1],skipinitialspace=True)
#print(df.columns)
df.apply(lambda row: check_row(row),axis=1)
print("test pass.")
