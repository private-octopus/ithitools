#!/bin/bash
# stage4 <date>
# calculate the html
mkdir -p /data/ITHI/html/$1
cd /data/ITHI/sum3/$1
/usr/local/ITHI/dev/avg.py $1 ALL results-a*
/usr/local/ITHI/dev/avg.py $1 US results-a*-us-*
/usr/local/ITHI/dev/avg.py $1 CA results-a*-ca-*
/usr/local/ITHI/dev/avg.py $1 BR results-a*-br-*
/usr/local/ITHI/dev/avg.py $1 RU results-a*-ru-*
/usr/local/ITHI/dev/avg.py $1 CZ results-a*-cz-*
/usr/local/ITHI/dev/avg.py $1 FR results-a*-fr-*
/usr/local/ITHI/dev/avg.py $1 DE results-a*-de-*
/usr/local/ITHI/dev/avg.py $1 AU results-a*-au-*
/usr/local/ITHI/dev/avg.py $1 LAX results-a*-us-lax.*
/usr/local/ITHI/dev/avg.py $1 RTV results-a*-us-rtv.*
/usr/local/ITHI/dev/avg.py $1 PDX results-a*-us-pdx*
