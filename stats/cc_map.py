#!/usr/bin/python
# coding=utf-8
#
import sys
import plotly.graph_objects as go
from urllib.request import urlopen
import json
import pandas as pd
import plotly.express as px
import m3summary

df = pd.read_csv(sys.argv[1])

df['qps'] = 0.0
df['nxd'] = 0.0
df['hom'] = 0.0
df['cor'] = 0.0
df['mai'] = 0.0
df['cc3'] = "???"
df['text'] = ""

countries = df.index.tolist()

for i in countries:
    if df['duration'][i] > 0:
        df['qps'][i] = float(df['queries'][i]) / float(df['duration'][i])
    if df['queries'][i] > 0:
        df['nxd'][i] = float(df['nx_domain'][i])/float(df['queries'][i])
        df['hom'][i] = float(df['home'][i])/float(df['queries'][i])
        df['cor'][i] = float(df['corp'][i])/float(df['queries'][i])
        df['mai'][i] = float(df['mail'][i])/float(df['queries'][i])
    df['cc3'][i] = m3summary.cc_to_iso3(df['cc'][i])
    df['text'][i] = df['cc'][i] + '<br>' + 'duration ' + str(df['duration'][i]) + \
        '<br>' + 'queries ' + str(df['queries'][i]) + \
        '<br>' + 'nx ' + str(df['nx_domain'][i]) + \
        '<br>' + 'home ' + str(df['home'][i]) + \
        '<br>' + 'corp ' + str(df['corp'][i]) + \
        '<br>' + 'mail ' + str(df['mail'][i])


print(df.head())

fig = go.Figure(data=go.Choropleth(
    locations=df['cc3'],
    z=df['mail'].astype(float),
    text=df['text'], # hover text
    marker_line_color='black', 
    colorbar_title="Total queries"
))

fig.show()