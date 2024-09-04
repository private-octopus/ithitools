
HOME=/home/christian.huitema@DS.ICANN.ORG
ITHITOOLS="$HOME/ithitools/ithitools"
IPSTATS_DIR="/data/huitema/ipstats"
IMRS_CBOR=/data/ITHI/cbor/IMRS/
MONTH=202409
SCRIPT="$HOME/ithitools/src/imrsrsv.py"
RUN_DATE=$(date '+%Y-%m-%d')
RUN_LOG="$HOME/tmp/$RUN_DATE.$$.txt"

echo "Home: $HOME"
echo "ithitools: $ITHITOOLS"
echo "ipstats: $IPSTATS_DIR"
echo "imrs: $IMRS_CBOR"
echo "Month: $MONTH"
echo "Script: $SCRIPT"
echo "Log: $RUN_LOG"

python $SCRIPT $IMRS_CBOR $IPSTATS_DIR $MONTH $ITHITOOLS > $RUN_LOG
