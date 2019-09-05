
#!/usr/bin/python
# coding=utf-8
#
# Produce summaries of the data per specific column list

import sys
import pandas as pd
import numpy as np


# load the CSV file

df = pd.read_csv(sys.argv[1])

print(df.index)

print(df.dtypes)

df_cc = df.sort_values(by="CC")
print(df_cc.head())

i = 0

