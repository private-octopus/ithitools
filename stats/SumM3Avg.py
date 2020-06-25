#!/usr/bin/env python
# coding=utf-8
#
# ITHI Kafka prototype
#
# SumM3Avg listens to Kafka message from the "thresholder", 
# checks whether these messages fit a Coumtry/city pattern, and
# if they do determines whether it is time to update the pattern's
# graph.
#
# The thresholder will send messages when a data file has collected
# at least M hours of input. The averager will update the graph
# when all files in the pattern have been updated in the last N hours.
# This works best if N > M, for example N = 2*M. If N is not greater than M,
# the first update in the day will happen when the first file is updated,
# and only the update after that will have data from all the files in the
# pattern.
#
# The command line arguments are:
# - bootstrap_servers: the bootstrap servers for the Kafka deployment
# - basefile: directory in which daily output directories will be created
# - nb_hours: the number N of hours between updates
# - label: the name of the pattern
# - country_code: the code of the selected country, or "" if not tested
# - city_code: the name of the selected city, or "" if not tested
#
# Each message from the Thresholder provides information about one node,
# which may or may not fit the pattern. For nodes in pattern, the 
# class sumM3Pattern maintains a list of the files updates for either the
# current day or the previous day. The program feeds the incoming messages
# to the sumM3Pattern class, and after updates chck whether either the 
# current day value or the previous day value needs to be updated. In that
# case, the sumM3Pattern class provides the updated day, the list of files
# to be averaged, and the corresponding list of nodes.
#
# The main program listens to the "m3Thresholder" topic.
# The client's group-id is set to the pattern's label. The program assumes
# that there is exactly one instance listening for each label -- otherwise,
# each instance would consume just a fraction of the messages, and the
# results would be wrong. 

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
from SumM3Lib import sumM3FileSeparator, sumM3EnsureEndInSep, sumM3EnsureDir, \
    sumM3Message, sumM3Pattern, sumM3DayPattern

#
# This is based on Alain's script "avg.py", encapsulated as a Python
# function so it can be called as a function. The script used a number
# of arguments:
# - dateCollect: the day of the collection, in YYYY-MM-DD format.
# - label: name of the pattern, combined with basefile to compute the
#   names of output files
# - arguments: the list of files that have to be averaged.
# The script uses the intermediary variable:
# - basefile: the root directory for the HTML files, set by convention
#   '/data/ITHI/html/'+dateCollect+'/'
#
# The script assumed that the files were located in the current directory,
# reading them directly through pandas' read_csv function, and that the names of
# the nodes were directly derived from the file names. In the function file, we
# move these name conventions outside of the function itself. Instead, the function
# will take the names as input arguments:
#
# - basefile: the root directory for the HTML files,
# - label: name of the pattern, 
# - file_paths: the location of the files that should be averaged
# - node_names: the name of the node for each file in the list.
# 
def sumM3DrawAverages(label, basefile, file_paths, nodenames):
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
    
    threshold=100000000
    tdrop=30

    # arguments=sys.argv
    # arguments.pop(0)
    # dateCollect=arguments.pop(0)
    # basefile='/data/ITHI/html/'+dateCollect+'/'
    # label=arguments.pop(0)

    for arg in file_paths:
        print("Loading: " + arg)
        # nodename=arg.replace('results-','').replace('.sum3','')
        df=pandas.read_csv(arg,  header=0, skipinitialspace=True)
        indexnull=df[df['queries'] == 0].index
        df.drop(indexnull, inplace=True)
        indexdrop=df[df['queries']/df['duration'] < tdrop].index
        df.drop(indexdrop, inplace=True)

        if df['queries'].sum()> threshold:
            # nodenames.append(nodename)

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

def publish_day_report(label, basefile, day_record, sep):
    file_paths = day_record.publish()
    if len(file_paths) > 0:
        day_report_base = basefile + day_record.bin_date.isoformat() + sep
        sumM3EnsureDir(day_report_base)
        nodenames = day_record.node_list()
        sumM3DrawAverages(label, day_report_base, file_paths, nodenames)
        print("Updated the daily report for " + label + " in " + day_report_base)

#
# Averager: read and verify the command line arguments,
# initialize the sumM3Pattern class, listen to the thresholder
# channel, and then process the incoming messages.
#
bootstrap_servers = ""
basefile = ""
nb_hours = 24
label = ""
country_code = ""
city_code = ""
good_arguments = True
if len(sys.argv) != 7:
    good_arguments = False
    print("Expected 7 arguments, got " + str(len(sys.argv)))
else:
    bootstrap_servers = sys.argv[1]
    basefile = sys.argv[2]
    try:
        nb_hours = int(sys.argv[3], 10)
    except:
        print("Unexpected value for nb_hours: " + sys.argv[3])
        good_arguments = False
    label = sys.argv[4]
    country_code = sys.argv[5]
    city_code = sys.argv[6]

if not good_arguments:
    print("Usage: " + sys.argv[0] + " <bootstrap_servers> basefile nb_hours label country_code city_code")
    print(" - bootstrap_servers: the bootstrap servers for the Kafka deployment")
    print(" - basefile: directory in which daily output directories will be created")
    print(" - nb_hours: the number N of hours between updates")
    print(" - label: the name of the pattern")
    print(" - country_code: the code of the selected country, or \"\" if not tested")
    print(" - city_code: the name of the selected city, or \"\" if not tested")
    exit(1)

# initialize the filesepator to deal with both Windows and Unix
sep = sumM3FileSeparator(basefile)
basefile = sumM3EnsureEndInSep(basefile, sep)

print("bootstrap.servers: " + sys.argv[1])
print("basefile: " + basefile)
print("nb_hours: " + str(nb_hours))
print("Pattern label: " + label)
print("Pattern country code: " + country_code)
print("Pattern city code: " + city_code)


# create the pattern manager
s3p = sumM3Pattern(label, country_code, city_code, 1, 4)

# Create Kafka Consumer instance
c = Consumer({
    'bootstrap.servers': bootstrap_servers,
    'group.id': 'sumM3Avg' + '-' + label
})

# Subscribe to topic 'm3Thresholder'
c.subscribe(['m3Thresholder'])

# Process messages
try:
    while True:
        try:
            pattern_in = sumM3Message()
            pattern_in.poll_kafka(c, 300.0)
            if pattern_in.topic == "":
                print("No good message for 300 sec.")
            elif s3p.pattern_match(pattern_in):
                print("Processing: " + pattern_in.to_string())
                for day_record in s3p.days:
                    if day_record.is_too_old(pattern_in.bin_date, 1):
                        # The date has changed, the first message is now too old, will be flushed next
                        publish_day_report(label, basefile, day_record, sep)
                # Flush the old days
                s3p.flush_old(pattern_in)
                # Add the newly received message
                s3p.add_element(pattern_in)
                # Process the day records that are ready
                for day_record in s3p.days:
                    publish_day_report(label, basefile, day_record, sep)
        except KeyboardInterrupt:
            break
        except Exception:
            traceback.print_exc()
            print("Exception processing m3 thresholding message: " + pattern_in.to_string())
            exit_code = 1
            break

except KeyboardInterrupt:
    pass
finally:
    # Leave group 
    c.close()
