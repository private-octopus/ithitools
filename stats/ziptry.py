import gzip
import shutil
import sys

with gzip.open(sys.argv[1], 'rb') as f_gz:
    with open(sys.argv[2], 'wb') as f_tx:
        shutil.copyfileobj(f_gz, f_tx)