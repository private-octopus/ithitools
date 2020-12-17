#!/bin/bash
# stage2b <date> <yyyy-mm-dd>
# calculate the M9 metrics
# TODO: install latest ithitools at: cd /usr/local/ITHI/dev/ithitools/
#       and update script location
cd ~/ithitools/stats
COUNT_FOLDER=/data/ITHI/count-addr/$1
M9_FOLDER=/data/ITHI/addr-m9
mkdir -p $M9_FOLDER
FREQUENT=../data/frequent-resolvers.csv
M9_CSV="$M9_FOLDER/M9-$2.csv"
echo "Preparing $M9_CSV from $COUNT_FOLDER"
python3 ./compute_m9x.py $2 $M9_CSV $FREQUENT $COUNT_FOLDER

