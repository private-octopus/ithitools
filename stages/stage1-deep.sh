#!/bin/bash
# stage1-prallel.sh <month>
ls|grep "cbor.xz" | parallel "test ! -f /data/ITHI/results-std/$1/results-$2/{}-results.csv && /usr/bin/xzcat {} | /usr/local/ITHI/dev/ithitools/ithitools -o /data/ITHI/results-std/$1/results-$2/{}-results-std.csv -E /data/ITHI/results-name/$1/results-$2/{}-results-name.csv -A /data/ITHI/results-addr/$1/results-$2/{}-results-addr.csv -e -Y /dev/stdin; gzip  /data/ITHI/results-name/$1/results-$2/{}-results-name.csv; gzip /data/ITHI/results-addr/$1/results-$2/{}-results-addr.csv"
