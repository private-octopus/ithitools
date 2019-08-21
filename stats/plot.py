#!/usr/bin/python
# coding=utf-8
#

import plotly.graph_objects as go
from urllib.request import urlopen
import json
import pandas as pd
import plotly.express as px

#fig = go.Figure(
#   data=[go.Bar(y=[2, 1, 3])],
#    layout_title_text="A Figure Displaying Itself"
#)
#fig.show()


#with urlopen('https://raw.githubusercontent.com/plotly/datasets/master/geojson-counties-fips.json') as response:
#    counties = json.load(response)
#
#
#unemp = pd.read_csv("https://raw.githubusercontent.com/plotly/datasets/master/fips-unemp-16.csv",
#                   dtype={"fips": str})
#
#
#fig = go.Figure(go.Choroplethmapbox(geojson=counties, locations=unemp.fips, z=unemp.unemp,
#                                    colorscale="Viridis", zmin=0, zmax=12, marker_line_width=0))
#
#token = "pk.eyJ1IjoiaHVpdGVtYSIsImEiOiJjanoxbHAxajUwOHZnM2JvOGphd21tMmpxIn0.UYKv2IZEdPw8rb-P-cagnA"
#
#fig.update_layout(mapbox_style="light", mapbox_accesstoken=token,
#                  mapbox_zoom=3, mapbox_center = {"lat": 37.0902, "lon": -95.7129})
#fig.update_layout(margin={"r":0,"t":0,"l":0,"b":0})
#fig.show()

#gapminder = px.data.gapminder().query("year==2007")
#
#fig = px.choropleth(gapminder, locations="iso_alpha",
#                    color="lifeExp", # lifeExp is a column of gapminder
#                    hover_name="country", # column to add to hover information
#                    color_continuous_scale=px.colors.sequential.Plasma)
#fig.show()

df = pd.read_csv('https://raw.githubusercontent.com/plotly/datasets/master/2011_us_ag_exports.csv')

for col in df.columns:
    df[col] = df[col].astype(str)

df['text'] = df['state'] + '<br>' + \
    'Beef ' + df['beef'] + ' Dairy ' + df['dairy'] + '<br>' + \
    'Fruits ' + df['total fruits'] + ' Veggies ' + df['total veggies'] + '<br>' + \
    'Wheat ' + df['wheat'] + ' Corn ' + df['corn']

fig = go.Figure(data=go.Choropleth(
    locations=df['code'],
    z=df['total exports'].astype(float),
    locationmode='USA-states',
    colorscale='Reds',
    autocolorscale=False,
    text=df['text'], # hover text
    marker_line_color='white', # line markers between states
    colorbar_title="Millions USD"
))

fig.show()