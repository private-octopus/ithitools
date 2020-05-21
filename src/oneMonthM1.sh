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

echo "Computing metrics for $LAST_DAY"
./ithitools/ithitools -i /home/ubuntu/ithi -d $LAST_DAY -m


