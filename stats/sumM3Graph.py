#!/usr/bin/env python
# coding=utf-8
#
# ITHI Kafka prototype
#
# SumM3Graph listens to Kafka message from the "thresholder", 
# and for each message creates an HTML file for visualizing the data
#
# The command line arguments are:
# - bootstrap_servers: the bootstrap servers for the Kafka deployment
# - basefile: directory in which daily output directories will be created
#
# The main program listens to the "m3Thresholder" topic.
# The client's group-id is set to "sumM3Graph". This should allow load sharing
# between several processes

import sys
import traceback
import datetime
import math
# import m3name
# import m3summary
from confluent_kafka import Consumer
import pandas
import numpy
import plotly.graph_objects as go
from plotly.offline import plot
from SumM3lib import sumM3FileSeparator, sumM3Message

#
# This is based on Alain's script "avg.py", encapsulated as a Python
# function so it can be called as a function. The script used a number
# of arguments:
# - dateCollect: the day of the collection, in YYYY-MM-DD format.
# - file: the file to be graphed.
# The script uses the intermediate variable:
# - basefile: the root directory for the HTML files, set by convention
#   '/data/ITHI/html/'+dateCollect+'/'
# - node_id: obtained by parsing the file name
# - outfile: named after the node being graphed
#
# The script assumed that the input file was located in the current directory,
# read them directly through pandas' read_csv function, and that the names of
# the nodes were directly derived from the file names. In the function file, we
# move these name conventions outside of the function itself. Instead, the function
# will take the names as input arguments:
#
# - writefile: the file that shall be written
# - file: the location of the files that should be read
# - node_dns: the name of the node where the data was collected
#
# The calling application sets the names of these files as it sees fit
# 

def sumM3DrawGraph(file, writefile, node_dns)
	# dateCollect=sys.argv[1]
	# file=sys.argv[2]
	# outfile=file.replace('results-','').replace('.sum3','')

	df=pandas.read_csv(file,  header=0, skipinitialspace=True).sort_values(['date', 'hour'], inplace=False).head(9000)

	indexnull=df[df['queries'] == 0].index
	df.drop(indexnull, inplace=True)

	df['Combined']=df['date']+'--'+df['hour']
	df['qps']=df['queries']/df['duration']
	df['usefulqps']=df['useful']/df['duration']
	df['uselessqps']=df['useless']/df['duration']
	df['dgaqps']=df['dga']/df['duration']
	df['jumboqps']=df['jumbo']/df['duration']
	df['othersqps']=df['others']/df['duration']

	usefulAvg2=int(df['usefulqps'].sum()/df['qps'].sum()*100+.5)
	uselessAvg2=int(df['uselessqps'].sum()/df['qps'].sum()*100+.5)
	jumboAvg2=int(df['jumboqps'].sum()/df['qps'].sum()*100+.5)
	dgaAvg2=int(df['dgaqps'].sum()/df['qps'].sum()*100+.5)
	othersAvg2=int(df['othersqps'].sum()/df['qps'].sum()*100+.5)

	usefulP=str(usefulAvg2)+'%'
	uselessP=str(uselessAvg2)+'%'
	dgaP=str(dgaAvg2)+'%'
	jumboP=str(jumboAvg2)+'%'
	othersP=str(othersAvg2)+'%'

	fig=go.Figure(data=[
		go.Scatter(x=df['Combined'], y=df['usefulqps'], name='Useful: '+usefulP, mode='lines', stackgroup='g', line = dict( width = 2, color='cornflowerblue'), fillcolor='cornflowerblue' ),
		go.Scatter(x=df['Combined'], y=df['uselessqps'], name='Non-Cached: '+uselessP, mode='lines', stackgroup='g', line = dict( width = 2,color='gold'),fillcolor='gold' ),
		go.Scatter(x=df['Combined'], y=df['dgaqps'], name='DGA: '+dgaP, mode='lines', stackgroup='g', line = dict( width = 2, color='seagreen'), fillcolor='seagreen' ),
		go.Scatter(x=df['Combined'], y=df['jumboqps'], name='Jumbo: '+jumboP, mode='lines', stackgroup='g', line = dict( width = 2, color='purple'), fillcolor='purple' ),
		go.Scatter(x=df['Combined'], y=df['othersqps'], name='others: '+othersP, mode='lines', stackgroup='g', line = dict( width = 1, color='darkorange', simplify=True), fillcolor='darkorange' )
	])
	# fig.update_layout(title=outfile, yaxis_title="Queries per Second")
	fig.update_layout(title=node_dns, yaxis_title="Queries per Second")
	#fig.show()
	# writefile='/data/ITHI/html/'+dateCollect+'/'+outfile+'.html'
	#writefile=outfile+'.html'
	print(writefile)
	plot(fig,filename=writefile, auto_open=False)

#
# Grapher: read and verify the command line arguments,
# listen to the thresholder
# channel, and then process the incoming messages.
#
bootstrap_servers = ""
basefile = ""
good_arguments = True
if len(sys.argv != 3):
	good_arguments = False
else:
	bootstrap_servers = argv[1]
	basefile = argv[2]

if not good_arguments:
	print("Usage: " + sys.argv[0] + " basefile nb_hours label country_code city_code")
	print(" - bootstrap_servers: the bootstrap servers for the Kafka deployment")
	print(" - basefile: directory in which daily output directories will be created")
	exit(1)

# initialize the filesepator to deal with both WIndows and Unix
sep = sumM3FileSeparator(basefile)

# Create Kafka Consumer instance
c = Consumer({
    'bootstrap.servers': sys.argv[1],
    'group.id': 'sumM3Graph'
})
# Subscribe to topic 'm3Thresholder'
c.subscribe(['m3Thresholder'])

# Process messages
try:
    while True:
        msg = c.poll(3600.0)
        if msg is None:
            # print a log message on time out.
            # maybe should also send a kafka message to 
            print("Waiting for m3Thresholder message or event/error in poll()")
            continue
        elif msg.error():
            print('error: {}'.format(msg.error()))
        else:
			pattern_in = sumM3Message()
			if not pattern_in.parse(str(msg.value())):
				print("Cannot parse m3Thresholder message <" + str(msg.value()) + ">")
        else:
			writefile = basefile + pattern_in.m3_date + sep
			sumM3DrawGraph(pattern_in.fpath, writefile, pattern_in.node_dns)

except KeyboardInterrupt:
    pass
finally:
    # Leave group 
    c.close()