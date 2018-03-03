# !/bin/bash
DATE=$(date)
echo $DATE
MONTH=$(date +%Y)
YEAR=$(date +%m)
echo $MONTH
echo $YEAR
PREVIOUS_DATE=$(date -d "-1 months" +%Y%m)
echo $PREVIOUS_DATE
LAST_DAY=$(date -d "$(date -d "+1 months" +%Y-%m-01) -1 days" +%Y-%m-%d)
echo $LAST_DAY
LAST_LAST_DAY=$(date -d"$(date +%Y-%m-01) -1 days" +%Y-%m-%d)
echo $LAST_LAST_DAY
