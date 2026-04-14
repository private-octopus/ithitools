# Get the top ISP, with UIDs > limit

import sys
import os
from pathlib import Path
import ip2as
import rsv_log_parse
import pandas as pd
import traceback

# main
df = pd.read_csv(sys.argv[1])
print("Df has " + str(df.shape) + " entries.")
limit = int(sys.argv[2])
s_df = df.sort_values(by='uids', axis=0, ascending=False)
df_top = s_df[s_df['uids'] >= limit ]
print("Df_top has " + str(df_top.shape) + " entries.")
df_top.to_csv(sys.argv[3])
print("Saved to " + sys.argv[3])




