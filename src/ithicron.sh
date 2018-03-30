# !/bin/bash
pwd
cd /home/ubuntu
pwd
DATE=$(date +%Y%m)
echo "This month selector: $DATE"
PREVIOUS_DATE=$(date -d "-1 months" +%Y%m)
echo "Previous month selector: $PREVIOUS_DATE"

DATE_DASH=$(date +%Y-%m)
echo "This month selector: $DATE"
PREVIOUS_DASH=$(date -d "-1 months" +%Y-%m)
echo "Previous month selector: $PREVIOUS_DATE"

LAST_DAY=$(date -d "$(date -d "+1 months" +%Y-%m-01) -1 days" +%Y-%m-%d)
echo "Last day of this month: $LAST_DAY"
LAST_LAST_DAY=$(date -d"$(date +%Y-%m-01) -1 days" +%Y-%m-%d)
echo "Last day of previous month: $LAST_LAST_DAY"

find /home/rarends/data/$DATE* | grep ".csv" > m3_this_month.txt
echo "Found $(wc -l m3_this_month.txt) files in /home/rarends/data/$DATE*"
M3F1=/home/ubuntu/ithi/input/M3/M3-$LAST_DAY-summary.csv
echo "Creating summary file in $M3F1"
./ithitools/ithitools -S m3_this_month.txt -o $M3F1

find /home/rarends/data/$PREVIOUS_DATE* | grep ".csv" > m3_previous_month.txt
echo "Found $(wc -l m3_previous_month.txt) files in /home/rarends/data/$PREVIOUS_DATE*"
M3F2=/home/ubuntu/ithi/input/M3/M3-$LAST_LAST_DAY-summary.csv
echo "Creating summary file in $M3F2"
./ithitools/ithitools -S m3_previous_month.txt -o $M3F2

>m46_this_month.txt
py ithitools/src/tlsaInput.py tlsa-data-$DATE_DASH.csv /home/viktor/data/tlsa-$DATE_DASH-*.csv
echo tlsa-data-$DATE_DASH.csv >> m46_this_month.txt
find /home/matiasf/* | grep $DATE | grep ".csv" >> m46_this_month.txt
find /home/uccgh/* | grep $DATE_DASH | grep ".csv" >> m46_this_month.txt
find /home/nawala/data/* | grep $DATE_DASH | grep ".csv" >> m46_this_month.txt
echo "Found $(wc -l m46_this_month.txt) recursive resolver reports for $DATE*"
M46F1=/home/ubuntu/ithi/input/M46/M46-$LAST_DAY-summary.csv
echo "Creating summary file in $M46F1"
./ithitools/ithitools -S m46_this_month.txt -o $M46F1

>m46_previous_month.txt
py ithitools/src/tlsaInput.py tlsa-data-$PREVIOUS_DASH.csv /home/viktor/data/tlsa-$PREVIOUS_DASH-*.csv
echo tlsa-data-$PREVIOUS_DASH.csv >> m46_previous_month.txt
find /home/matiasf/* | grep $PREVIOUS_DATE | grep ".csv" >> m46_previous_month.txt
find /home/uccgh/* | grep $PREVIOUS_DASH | grep ".csv" >> m46_previous_month.txt
find /home/nawala/data/* | grep $PREVIOUS_DASH | grep ".csv" >> m46_previous_month.txt
echo "Found $(wc -l m46_previous_month.txt) recursive resolver reports for $PREVIOUS_DATE*"
M46F2=/home/ubuntu/ithi/input/M46/M46-$LAST_LAST_DAY-summary.csv
echo "Creating summary file in $M46F2"
./ithitools/ithitools -S m46_previous_month.txt -o $M46F2

M7F1=/home/ubuntu/ithi/input/M7/M7-$LAST_DAY.zone
echo "Copying root zone file to $M7F1"
wget https://www.internic.net/domain/root.zone -O $M7F1

echo "Computing metrics for $LAST_LAST_DAY"
./ithitools/ithitools -i /home/ubuntu/ithi -d $LAST_LAST_DAY -m
echo "Computing metrics for $LAST_DAY"
./ithitools/ithitools -i /home/ubuntu/ithi -d $LAST_DAY -m

echo "Computing JSON Data for publication"
./ithitools/ithitools -i /home/ubuntu/ithi -w /var/www/html/ -p
