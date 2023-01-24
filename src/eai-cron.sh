# !/bin/bash
# Check whether there are new EAI files published, and copy them
# to the eai_results folder in the html directory.
# Update the link to the latest file in the EAI folder.
eai_input_dir="/home/eai/eai_results"
ithi_www_dir="/var/www/html"
eai_www_dir="/var/www/html/eai_results"

rsync -av "$eai_input_dir/" "$eai_www_dir/"

eai_first=""
link_target="$ithi_www_dir/eai_results/eai-latest.csv"
for i in `ls -c $eai_www_dir`; do
    if [ "$eai_first" = "" ]; then
        eai_first="$eai_www_dir/$i"
        ln -f $eai_first $link_target
        break;
    fi
    echo $i;
done
echo "The latest file is $eai_first"
echo "The linked file starts as: "
head -3 $link_target
