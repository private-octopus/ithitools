# !/bin/bash
pwd
cd /home/ubuntu
pwd
DATE_CURRENT=$1
echo "Current: $DATE_CURRENT"
DATE=$(date -d $DATE_CURRENT +%Y%m)
YEAR=$(date -d $DATE_CURRENT +%Y)
MM=$(date -d $DATE_CURRENT +%m)
DATE_DASH=$(date -d $DATE_CURRENT +%Y-%m-%d)
FIRST_DAY=$(date -d $YEAR-$MM-01 +%Y-%m-%d)
DAY_AFTER_MONTH=$(date -d "$FIRST_DAY +1 months" +%Y-%m-01)
LAST_DAY=$(date -d "$DAY_AFTER_MONTH -1 day" +%Y-%m-%d)
echo "First day: $FIRST_DAY"
echo "Day after month: $DAY_AFTER_MONTH"
echo "This month selector: $DATE (or $DATE_DASH), Year: $YEAR, Month: $MM"
echo "Last day of this month: $LAST_DAY"

find /home/rarends/data/$DATE* | grep ".csv" > m3_catch_month.txt
echo "Found $(wc -l m3_catch_month.txt) files in /home/rarends/data/$DATE*"
M3F1=/home/ubuntu/ithi/input/M3/M3-$LAST_DAY-summary.csv
echo "Creating summary file in $M3F1"
./ithitools/ithitools -S m3_catch_month.txt -o $M3F1

>m46_catch_month.txt
python ithitools/src/tlsaInput.py tlsa-data-$DATE_DASH.csv /home/viktor/data/tlsa-$DATE_DASH
echo tlsa-data-$DATE_DASH.csv >> m46_catch_month.txt
find /home/matiasf/* | grep $DATE | grep ".csv" >> m46_catch_month.txt
find /home/uccgh/data/* | grep $DATE_DASH | grep ".csv" >> m46_catch_month.txt
find /home/nawala/data/* | grep $DATE_DASH | grep ".csv" >> m46_catch_month.txt
echo "Found $(wc -l m46_catch_month.txt) recursive resolver reports for $DATE*"
M46F1=/home/ubuntu/ithi/input/M46/M46-$LAST_DAY-summary.csv
echo "Creating summary file in $M46F1"
./ithitools/ithitools -S m46_catch_month.txt -o $M46F1

>m8_catch_month.txt
find /home/kaznic/* | grep $DATE_DASH | grep ".csv" >> m8_catch_month.txt
find /home/twnic/data/* | grep $DATE_DASH | grep ".csv" >> m8_catch_month.txt
echo "Found $(wc -l m8_catch_month.txt) authoritative resolver reports for $DATE_DASH*"
M8F1=/home/ubuntu/ithi/input/M8/M8-$LAST_DAY-summary.csv
echo "Creating summary file in $M8F1"
./ithitools/ithitools -S m8_catch_month.txt -o $M8F1

M7F1=/home/ubuntu/ithi/input/M7/M7-$LAST_DAY.zone
echo "Copying root zone file to $M7F1"
wget https://www.internic.net/domain/root.zone -O $M7F1

echo "Computing metrics for $LAST_DAY"
./ithitools/ithitools -i /home/ubuntu/ithi -d $LAST_DAY -m

echo "Ingesting M5 for $LAST_DAY"
python ithitools/src/m5ingest.py /home/gih/data/$YEAR/$MM/ /home/ubuntu/ithi/M5/M5-$LAST_DAY.csv $LAST_DAY



