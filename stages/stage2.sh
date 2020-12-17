#!/bin/bash
# stage2 <date>
# clean-up: remove empty directories
cd /data/ITHI/results-std/$1 ; for i in *; do echo -n $i; echo -n " "; ls $i| wc -l; done | grep " 0" | /usr/bin/rmdir  `awk '{ print $1}'`
cd /data/ITHI/results-std/$1 ; /bin/rm -f progress
# calculate the stats
cd /usr/local/ITHI/dev/ithitools/stats
mkdir -p /data/ITHI/sum3/$1
ls -1 /data/ITHI/results-std/$1 | parallel "python3 ./load_l_root_data.py /data/ITHI/results-std/$1/{} /data/ITHI/sum3/$1/{}.sum3"
