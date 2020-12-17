#!/bin/bash
# stage3 <date>
# calculate the html
mkdir -p /data/ITHI/html/$1
cd /data/ITHI/sum3/$1 ; ls -1 | parallel "/usr/local/ITHI/dev/graph.py $1 {}"
