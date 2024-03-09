
HOME=/home/christian.huitema@DS.ICANN.ORG
ITHITOOLS="$HOME/ithitools/ithitools"
IPSTATS_DIR="$HOME/ipstats"
IMRS_CBOR=/data/ITHI/cbor/IMRS/
MONTH=202403
SCRIPT="$HOME/ithitools/src/imrsrsv.py"

echo "Home: $HOME"
echo "ithitools: $ITHITOOLS"
echo "ipstats: $IPSTATS_DIR"
echo "imrs: $IMRS_CBOR"
echo "Month: $MONTH"
echo "Script: $SCRIPT"

python3.6 $SCRIPT $IMRS_CBOR $IPSTATS_DIR $MONTH $ITHITOOLS
