LAST_DAY=$2

grep $1 < m8_this_month.txt > m8_$1_one_month.txt
echo "Found $(wc -l m8_$1_one_month.txt) authoritative resolver reports for $1 at $2"
M8F1=ithi/$1/input/M8/M8-$LAST_DAY-summary.csv
echo "Creating summary file in $M8F1"
./ithitools/ithitools -S m8_$1_one_month.txt -o $M8F1

echo "Computing metrics for $LAST_DAY"
./ithitools/ithitools -i ithi/$1 -d $LAST_DAY -m
