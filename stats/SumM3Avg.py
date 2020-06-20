
#!/usr/bin/env python
# coding=utf-8
#
# ITHI Kafka prototype, consume M3 summaries as they are produced, creates M3Sum message

import sys
import codecs
from enum import Enum
import copy
import traceback
import datetime
import math
import m3name
import m3summary
from confluent_kafka import Consumer
import pandas
import numpy
import plotly.graph_objects as go
from plotly.offline import plot
threshold=100000000
tdrop=30

nodenames=[]
usefulMax=[]
usefulMin=[]
usefulAvg2=[]
usefulMAvg=[]
uselessMax=[]
uselessMin=[]
uselessAvg2=[]
uselessMAvg=[]
dgaMax=[]
dgaMin=[]
dgaAvg2=[]
jumboMax=[]
jumboMin=[]
jumboAvg2=[]
othersMax=[]
othersMin=[]
othersAvg2=[]
queriesMax=[]
queriesMin=[]
queriesAvg2=[]

arguments=sys.argv
arguments.pop(0)
dateCollect=arguments.pop(0)
basefile='/data/ITHI/html/'+dateCollect+'/'
label=arguments.pop(0)

for arg in arguments:
	print(arg)
	nodename=arg.replace('results-','').replace('.sum3','')
	df=pandas.read_csv(arg,  header=0, skipinitialspace=True)
	indexnull=df[df['queries'] == 0].index
	df.drop(indexnull, inplace=True)
	indexdrop=df[df['queries']/df['duration'] < tdrop].index
	df.drop(indexdrop, inplace=True)

	if df['queries'].sum()> threshold:

		nodenames.append(nodename)

		df['usefulRatio']=df['useful']/df['queries']*100
		df['uselessRatio']=df['useless']/df['queries']*100
		df['dgaRatio']=df['dga']/df['queries']*100
		df['jumboRatio']=df['jumbo']/df['queries']*100
		df['othersRatio']=df['others']/df['queries']*100

		usefulMin.append(df['usefulRatio'].min())
		usefulAvg2.append(df['useful'].sum()/df['queries'].sum()*100)
		usefulMax.append(df['usefulRatio'].max())

		uselessMin.append(df['uselessRatio'].min())
		uselessAvg2.append(df['useless'].sum()/df['queries'].sum()*100)
		uselessMax.append(df['uselessRatio'].max())

		dgaMin.append(df['dgaRatio'].min())
		dgaAvg2.append(df['dga'].sum()/df['queries'].sum()*100)
		dgaMax.append(df['dgaRatio'].max())

		jumboMin.append(df['jumboRatio'].min())
		jumboAvg2.append(df['jumbo'].sum()/df['queries'].sum()*100)
		jumboMax.append(df['jumboRatio'].max())

		othersMin.append(df['othersRatio'].min())
		othersAvg2.append(df['others'].sum()/df['queries'].sum()*100)
		othersMax.append(df['othersRatio'].max())

outputUseful=pandas.DataFrame()
outputUseful['node']=nodenames
outputUseful['max']=usefulMax
outputUseful['min']=usefulMin
outputUseful['average']=usefulAvg2

outputUseless=pandas.DataFrame()
outputUseless['node']=nodenames
outputUseless['max']=uselessMax
outputUseless['min']=uselessMin
outputUseless['average']=uselessAvg2

outputDga=pandas.DataFrame()
outputDga['node']=nodenames
outputDga['max']=dgaMax
outputDga['min']=dgaMin
outputDga['average']=dgaAvg2

outputJumbo=pandas.DataFrame()
outputJumbo['node']=nodenames
outputJumbo['max']=jumboMax
outputJumbo['min']=jumboMin
outputJumbo['average']=jumboAvg2

outputOthers=pandas.DataFrame()
outputOthers['node']=nodenames
outputOthers['max']=othersMax
outputOthers['min']=othersMin
outputOthers['average']=othersAvg2


outputUsefulSorted=outputUseful.sort_values('average', inplace=False, ascending=False)
usefulfig=go.Figure(data=[
	go.Bar(x=outputUsefulSorted['node'], y=outputUsefulSorted['min'], name='Min 5min Useful Ratio', marker_color='lightblue', opacity=0.5),
	go.Scatter(x=outputUsefulSorted['node'], y=outputUsefulSorted['average'], name='Series Average Useful', line=dict(color='cornflowerblue', width=4)),
	go.Bar(x=outputUsefulSorted['node'], y=outputUsefulSorted['max'], name='Max 5min Useful Ratio', marker_color='darkblue', opacity=0.5)
])
usefulfig.update_layout(title='Min, Node Average and Max 5min Percentage of Useful Queries: '+label)
#usefulfig.show()
writefile=basefile+'avgminmax-'+label+'-useful.html'
plot(usefulfig, filename=writefile, auto_open=False)

outputUselessSorted=outputUseless.sort_values('average', inplace=False, ascending=False)
uselessfig=go.Figure(data=[
	go.Bar(x=outputUselessSorted['node'], y=outputUselessSorted['min'], name='Min 5min Useless Ratio', marker_color='yellow', opacity=0.5),
	go.Scatter(x=outputUselessSorted['node'], y=outputUselessSorted['average'], name='Series Average Useless', line=dict(color='gold', width=4)),
	go.Bar(x=outputUselessSorted['node'], y=outputUselessSorted['max'], name='Max 5min Useless Ratio', marker_color='goldenrod', opacity=0.5)
])
uselessfig.update_layout(title='Min, Average and Max 5min Percentage of Non-Cached Queries: '+label)
#uselessfig.show()
writefile=basefile+'avgminmax-'+label+'-useless.html'
plot(uselessfig, filename=writefile, auto_open=False)

outputDgaSorted=outputDga.sort_values('average', inplace=False, ascending=False)
dgafig=go.Figure(data=[
	go.Bar(x=outputDgaSorted['node'], y=outputDgaSorted['min'], name='Min 5min DGA Ratio', marker_color='lightgreen',opacity=0.5),
	go.Scatter(x=outputDgaSorted['node'], y=outputDgaSorted['average'], name='Series Average DGA', line=dict(color='seagreen', width=4)),
	go.Bar(x=outputDgaSorted['node'], y=outputDgaSorted['max'], name='Max 5min DGA Ratio', marker_color='green',opacity=0.5)
])
dgafig.update_layout(title='Min, Average and Max 5min Percentage of DGA Queries: '+label)
#dgafig.show()
writefile=basefile+'avgminmax-'+label+'-dga.html'
plot(dgafig, filename=writefile, auto_open=False)

outputJumboSorted=outputJumbo.sort_values('average', inplace=False, ascending=False)
jumbofig=go.Figure(data=[
	go.Bar(x=outputJumboSorted['node'], y=outputJumboSorted['min'], name='Min 5min Jumbo Ratio', marker_color='lightsteelblue',opacity=0.5),
	go.Scatter(x=outputJumboSorted['node'], y=outputJumboSorted['average'], name='Series Average Jumbo', line=dict(color='purple', width=4)),
	go.Bar(x=outputJumboSorted['node'], y=outputJumboSorted['max'], name='Max 5min Jumbo Ratio', marker_color='midnightblue',opacity=0.5)
])
jumbofig.update_layout(title='Min, Average and Max 5min Percentage of Jumbo Queries: '+label)
#jumbofig.show()
writefile=basefile+'avgminmax-'+label+'-jumbo.html'
plot(jumbofig, filename=writefile, auto_open=False)

outputOthersSorted=outputOthers.sort_values('average', inplace=False, ascending=False)
othersfig=go.Figure(data=[
	go.Bar(x=outputOthersSorted['node'], y=outputOthersSorted['min'], name='Min 5min Others Ratio', marker_color='lightsalmon', opacity=0.5),
	go.Scatter(x=outputOthersSorted['node'], y=outputOthersSorted['average'], name='Series Average Others', line=dict(color='darkorange', width=4)),
	go.Bar(x=outputOthersSorted['node'], y=outputOthersSorted['max'], name='Max 5min Others Ratio', marker_color='tomato', opacity=0.5)
])
othersfig.update_layout(title='Min, Average and Max 5min Percentage of Other Queries: '+label)
#othersfig.show()
writefile=basefile+'avgminmax-'+label+'-others.html'
plot(othersfig, filename=writefile, auto_open=False)

