# !/bin/bash
pwd
cd /home/ubuntu
pwd

YEAR=$1
MM=$2
DATE_CURRENT=$(date -d $YEAR-$MM-15 +%Y%m%d)
echo "Current: $DATE_CURRENT"
DATE=$(date -d $DATE_CURRENT +%Y%m)
echo "Date: $DATE"
DATE_DASH=$(date -d $DATE_CURRENT +%Y-%m)
FIRST_DAY=$(date -d $YEAR-$MM-01 +%Y-%m-%d)
DAY_AFTER_MONTH=$(date -d "$FIRST_DAY +1 months" +%Y-%m-01)
LAST_DAY=$(date -d "$DAY_AFTER_MONTH -1 day" +%Y-%m-%d)
echo "First day: $FIRST_DAY"
echo "Day after month: $DAY_AFTER_MONTH"
echo "This month selector: $DATE (or $DATE_DASH), Year: $YEAR, Month: $MM"
echo "Last day of this month: $LAST_DAY"

>m46_one_month.txt
python ithitools/src/tlsaInput.py tlsa-data-$DATE_DASH.csv /home/viktor/data/tlsa-$DATE_DASH
echo tlsa-data-$DATE_DASH.csv >> m46_one_month.txt
for M4P in `cat /home/ubuntu/data/m4list`; do
find /home/$M4P/data/* | grep $DATE_DASH | grep ".csv" >> m46_one_month.txt
done
echo "Found $(wc -l m46_this_month.txt) recursive resolver reports for $DATE*"
M46F1=/home/ubuntu/ithi/input/M46/M46-$LAST_DAY-summary.csv
echo "Creating summary file in $M46F1"
./ithitools/ithitools -S m46_one_month.txt -o $M46F1

echo "Computing metrics for $LAST_DAY"
./ithitools/ithitools -i /home/ubuntu/ithi -d $LAST_DAY -m

# Partner data
./ithitools/src/oneMonthM4Partner.sh nawala $LAST_DAY
./ithitools/src/oneMonthM4Partner.sh uccgh $LAST_DAY
./ithitools/src/oneMonthM4Partner.sh unlp $LAST_DAY

