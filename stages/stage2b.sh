#!/bin/bash
# stage2b <date>
# calculate the stats
# TODO: install latest ithitools at: cd /usr/local/ITHI/dev/ithitools/
#       and update script location
cd ~/ithitools/stats
LOC_DIR=/data/ITHI/results-addr/$1
COUNT_FOLDER=/data/ITHI/count-addr/$1
mkdir -p $COUNT_FOLDER
LOCATION_FILE=~/stages/imrs-locations.txt
TEMP_PREFIX=~/ithi-stage2b-tmp
FREQUENT=../data/frequent-resolvers.csv
IP2AS=../data/ip2as.csv
IP2AS6=../data/ip2asv6.csv
COUNT_PARAMS="$TEMP_PREFIX $FREQUENT $IP2AS $IP2AS6"
for location in `cat $LOCATION_FILE`;
do
    rm $TEMP_PREFIX*
    COUNT_CSV="$COUNT_FOLDER/count-$location.csv"
    echo "Preparing count file for $location at $COUNT_CSV"
    LOC_FOLDERS=""
    echo $LOC_DIR
    for LOC_FILE in `ls $LOC_DIR | grep $location`;
    do
        echo $LOC_FILE
        LOC_FOLDERS="$LOC_FOLDERS $LOC_DIR/$LOC_FILE"
        echo $LOC_FOLDERS
    done
    echo "From: $LOC_FOLDERS"
    python3 ./count_ip.py $COUNT_CSV $COUNT_PARAMS $LOC_FOLDERS
done
rm $TEMP_PREFIX*

