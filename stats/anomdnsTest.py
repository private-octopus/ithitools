#!/usr/bin/python
# coding=utf-8
#
# This script reformats a list of leaked names

import anomdns
from anomdns import anonymizer

key = "blahh"
anom = anonymizer()
anom.set_key(key)

test = [
    "WWW.GOOGLEAPIS.COM.DAVOLINK", "IB.TIKTOKV.COM.DAVOLINK", "YOUTUBE.COM.DAVOLINK", 
    "MTALK.GOOGLE.COM.DAVOLINK", "US.PERF.GLBDNS.MICROSOFT.COM.IPTIME",
    "ANDROID.PROD.CLOUD.NETFLIX.COM.DAVOLINK", "YT3.GGPHT.COM.DAVOLINK",
    "YT3.GGPHT.8.8.8.8.DAVOLINK", "YT3.GGPHT.128.18.28.38.DAVOLINK",
    "KASPERSKY-AP.VUITTON.LVMH", "PAC.VUITTON.LVMH", "LVAPKRN060001C7.NA.VUITTON.LVMH",
    "CWDLPPAPP3.SSG20", "GRAPH.FACEBOOK.COM.SSG20", "API.FACEBOOK.COM.SSG20",
    "2019-01-09.SSG20", "NTPS1-0.UNI-ERLANGEN.DE.219.50.36.130",
    "13.228.35.130", "LB._DNS-SD._UDP.192.9.8.130"]

addr_test = [
    "0.0.0.0", "8.8.8.8", "1.1.1.1", "8.8.4.4", "255.255.255.255",
    "127.0.0.1", "192.168.0.1", "172.16.13.23", "169.254.123.45",
    "321.1.2.3", "1.234.5.6", "a.b.c.d", "-1.2.3.4", "1.2.3.4.5", "1.2.3",
    "12.34.56.78", "123.45.67.89", 
    "::", "::1", 
    "2620:fe::fe",
    "::ffff:255.255.255.255", "::ffff:1.2.3.4",
    "::ffff:ffff:ffff", "::ffff:102:304",
    "64::ffff:ffff:ffff", "64::ffff:102:304",
    "2001:123:45:67::abc:900d",
    "2001:123:45:67::abc:FFFF",
    "2001:123:45:67:0:0:abc:def",
    "2001:123:45:67:0:0:bad",
    "2001:123::67::bad:bad",
    "2001:0290:2002:0004:0000:0000:0000:0211"]

for line in test:
    print (line + ", " + anom.anonymizeName(line, 2))

for addr in addr_test:
    print (addr + ", " + anom.anonymizeAddress(addr))