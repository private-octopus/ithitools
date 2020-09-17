#!/usr/bin/python
# coding=utf-8
#
# Automatic parser for the ASN source at https://bgp.potaroo.net/cidr/autnums.html
# That page contains lines of the form "<a href="/cgi-bin/as-report?as=AS0&view=2.0">AS0    </a> -Reserved AS-, ZZ"

import sys
import traceback
import ipaddress

if len(sys.argv) != 3:
    print("Usage: " + sys.argv[0] + " <asnames.html> <asnames.csv>")


with open(sys.argv[2],"wt") as w_out:
    w_out.write("as_number,as_name,as_country,\n")
    for html_line in open(sys.argv[1]):
        parts = html_line.split(">")
        if len(parts) == 3:
            as_parts = parts[1].split("<")
            name_parts = parts[2].split(",")
            if len(as_parts)==2 and len(name_parts) >= 2:
                as_id = as_parts[0].strip()
                as_country = name_parts.pop().strip()
                as_name = ""
                for n in name_parts:
                    as_name += n
                    as_name += " "
                as_name = as_name.strip()
                if as_id[0:2] == "AS":
                    try:
                        as_number = int(as_id[2:])
                        w_out.write(as_id + "," + as_name + "," + as_country + ",\n")
                    except:
                        traceback.print_exc()
                        print("Cannot parse: " + line.strip())




