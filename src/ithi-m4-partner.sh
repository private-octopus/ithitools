# !/bin/bash
pwd
cd /home/ubuntu
pwd
TODAY=$(date +%d)

if [ $TODAY -lt 7 ]
    then
	    DATE_CURRENT=$(date -d "-7 days" +%Y-%m-%d)
		DATE_PREVIOUS=$(date -d "-38 days" +%Y-%m-%d)
	else
	    DATE_CURRENT=$(date +%Y-%m-%d)
        if [ $TODAY -gt 15 ]
		then
		    DATE_PREVIOUS=$(date -d "-31 days" +%Y-%m-%d)
		else
		    DATE_PREVIOUS=$(date -d "-1 months" +%Y-%m-%d)
		fi
	fi

DATE=$(date -d $DATE_CURRENT +%Y%m)
PREVIOUS_DATE=$(date -d $DATE_PREVIOUS +%Y%m)
YEAR=$(date -d $DATE_CURRENT +%Y)
MM=$(date -d $DATE_CURRENT +%m)
DATE_DASH=$(date -d $DATE_CURRENT +%Y-%m)
PREVIOUS_DASH=$(date -d $DATE_PREVIOUS +%Y-%m)
PREVIOUS_YEAR=$(date -d $DATE_PREVIOUS +%Y)
PREVIOUS_MM=$(date -d $DATE_PREVIOUS +%m)


echo "This month selector: $DATE (or $DATE_DASH), Year: $YEAR, Month: $MM"
echo "Previous month selector: $PREVIOUS_DATE (or $PREVIOUS_DASH), Year: $PREVIOUS_YEAR, Month: $PREVIOUS_MM"

LAST_DAY=$(date -d "$(date -d "$DATE_CURRENT +1 months" +%Y-%m-01) -1 days" +%Y-%m-%d)
echo "Last day of this month: $LAST_DAY"
LAST_LAST_DAY=$(date -d "$(date -d $DATE_CURRENT +%Y-%m-01) -1 days" +%Y-%m-%d)
echo "Last day of previous month: $LAST_LAST_DAY"

>m46_$1_this_month.txt
find $2/* | grep $DATE | grep ".csv" >> m46_$1_this_month.txt
find $2/* | grep $DATE_DASH | grep ".csv" >> m46_$1_this_month.txt
echo "Found $(wc -l m46_$1_this_month.txt) recursive resolver reports for $DATE*"
M46F1=/home/$1/ithi/input/M46/M46-$LAST_DAY-summary.csv
echo "Creating summary file in $M46F1"
./ithitools/ithitools -S m46_$1this_month.txt -o $M46F1

>m46_$1previous_month.txt
find $2/* | grep $PREVIOUS_DASH | grep ".csv" >> m46_$1_previous_month.txt
find $2/* | grep $PREVIOUS_DASH | grep ".csv" >> m46_$1_previous_month.txt
find /home/nawala/data/* | grep $PREVIOUS_DASH | grep ".csv" >> m46_previous_month.txt
echo "Found $(wc -l m46_previous_month.txt) recursive resolver reports for $PREVIOUS_DATE*"
M46F2=/home/$1/ithi/input/M46/M46-$LAST_LAST_DAY-summary.csv
echo "Creating summary file in $M46F2"
./ithitools/ithitools -S m46_previous_month.txt -o $M46F2

echo "Computing $1 metrics for $LAST_LAST_DAY"
./ithitools/ithitools -i /home/$1/ithi -d $LAST_LAST_DAY -m
echo "Computing metrics for $LAST_DAY"
./ithitools/ithitools -i /home$1/ithi -d $LAST_DAY -m

echo "Computing JSON Data for publication"
./ithitools/ithitools -i /home/$1/ithi -w /var/www/html/$1 -p
