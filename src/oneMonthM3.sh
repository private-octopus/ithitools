# !/bin/bash
pwd
cd /home/ubuntu
pwd
YEAR=$1
MM=$2
FIRST_DAY=$(date -d $YEAR-$MM-01 +%Y-%m-%d)
DATE=$(date -d $FIRST_DAY +%Y%m)
DAY_AFTER_MONTH=$(date -d "$FIRST_DAY +1 months" +%Y-%m-01)
LAST_DAY=$(date -d "$DAY_AFTER_MONTH -1 day" +%Y-%m-%d)
echo "First day: $FIRST_DAY"
echo "Day after month: $DAY_AFTER_MONTH"
echo "This month selector: $DATE"
echo "Last day of this month: $LAST_DAY"

find /home/rarends/data/$DATE* | grep ".csv" > m3_this_month.txt
echo "Found $(wc -l m3_this_month.txt) files in /home/rarends/data/$DATE*"
M3F1=/home/ubuntu/ithi/input/M3/M3-$LAST_DAY-summary.csv
echo "Creating summary file in $M3F1"
./ithitools/ithitools -S m3_this_month.txt -o $M3F1
echo "Computing metrics for $LAST_DAY"
./ithitools/ithitools -i /home/ubuntu/ithi -d $LAST_DAY -m
