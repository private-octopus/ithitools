# !/bin/bash
pwd
cd /home/ubuntu
pwd
DATE_CURRENT=$(date +%Y-%m-%d)
YEAR=$(date -d $DATE_CURRENT +%Y)
MM=$(date -d $DATE_CURRENT +%m)
FIRST_DAY=$(date -d $YEAR-$MM-01 +%Y-%m-%d)
LAST_LAST_DAY=$(date -d "$FIRST_DAY -1 day"  +%Y-%m-%d)
LAST_LAST_M3=/home/ubuntu/ithi/M3/M3-$LAST_LAST_DAY.csv
echo "Current: $DATE_CURRENT"
echo "First day: $FIRST_DAY"
echo "Last day of previous month: $LAST_LAST_DAY"
echo "File to copy: $LAST_LAST_M3"
cp $LAST_LAST_M3 /srv/ftp/M3/
ls -l /srv/ftp/M3/






