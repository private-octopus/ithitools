LAST_DAY=$2

>m46_$1_one_month.txt
grep $1 < m46_one_month.txt >> m46_$1_one_month.txt
echo "Found $(wc -l m46_$1_one_month.txt) recursive resolver reports for $1 at $2"
M46F1=ithi/$1/input/M46/M46-$LAST_DAY-summary.csv
echo "Creating summary file in $M46F1"
./ithitools/ithitools -S m46_$1_one_month.txt -o $M46F1

echo "Computing metrics for $LAST_DAY"
./ithitools/ithitools -i ithi/$1 -d $LAST_DAY -m
