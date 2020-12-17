#!/bin/bash
# stage1.sh <month>
cd /data/ITHI/cbor/$1
pwd
for i in a*
do
mkdir -p /data/ITHI/results-std/$1
mkdir -p /data/ITHI/results-addr/$1
mkdir -p /data/ITHI/results-name/$1
mkdir -p /data/ITHI/results-std/$1/results-$i
mkdir -p /data/ITHI/results-addr/$1/results-$i
mkdir -p /data/ITHI/results-name/$1/results-$i
(cd /data/ITHI/cbor/$1/$i ; sh /usr/local/ITHI/dev/stage1-deep.sh $1 $i)
done
