
#!/usr/bin/python
# coding=utf-8
#
# Produce summaries of the data per specific column list

import sys
import pandas as pd
import numpy as np


# load the CSV file

df = pd.read_csv(argv[1])
df
