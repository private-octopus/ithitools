import gzip
import shutil
import sys

#with gzip.open(sys.argv[1], 'rt') as f_gz:

iline = 0
for line in gzip.open(sys.argv[1], 'rt'):
    iline += 1
    if iline > 10:
        break
    print("-->" + line)
    