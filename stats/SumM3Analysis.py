#!/usr/bin/env python
# coding=utf-8
#
# ITHI Kafka prototype, consume M3 captures as they are produced, 
# store a capture line in the table file per day and node, 
# and propagate a message mentioning the update to the "m3Analysis" topic

import sys
import codecs
from enum import Enum
import copy
import traceback
import datetime
import math
import m3name
import m3summary
from confluent_kafka import Consumer, Producer
from SumM3Lib import sumM3FileSeparator, sumM3FileName, sumM3AppendLine, sumM3Message
import os

def m3AnalysisAcked(err, msg):
    if err is not None:
        print("Failed to deliver sumM3 analysis message: %s: %s" % (str(msg), str(err)))
        exit(1)

# check the calling argument
if len(sys.argv) != 3:
    print("Usage: " + sys.argv[0] + "<bootstrap.servers>  <ithi_dir>\n")
    exit(1)
base_dir = sys.argv[2]
sep = sumM3FileSeparator(base_dir)

# Create Consumer instance
c = Consumer({
    'bootstrap.servers': sys.argv[1],
    'group.id': 'sumM3Consumer'
})

# Subscribe to topic 'test'
c.subscribe(['m3Summary'])

# Create a provider instance.
# TODO: move to a common create produce function.
p = Producer({'bootstrap.servers': sys.argv[1]})
if err is not None:
    print("Failed to create producer: %s: %s" % (str(p), str(err)))
    exit(1)

# Process messages
total_count = 0
try:
    while True:
        msg = c.poll(300.0)
        if msg is None:
            # print a log message on time out.
            # maybe should also send a kafka message to 
            print("Waiting for m3Summary message or event/error in poll()")
            continue
        elif msg.error():
            print('error: {}'.format(msg.error()))
        else:
            try:
                # The Kafka message should provide keys of this summary: location and date,
                # and the name of the file.
                record_key = msg.key()
                record_value = msg.value()
                # Parse the message value, retrieve the capture file name, 
                s3msg_in = sumM3Message()
                if s3msg.parse(str(record_value)):
                    #add a line to the selected sumM3 file. Return the associated M3 Name
                    s3msg_out = s3msg_in.sumM3AppendLine(base_dir, sep)
                    # if all went well, produce the message for the next stage 
                    if s3msg_out.topics != "":
                        msg = s3msg_out.to_string()
                        p.produce(s3msg_out.topics, msg.encode(encoding='utf-8', errors='strict'), callback=m3AnalysisAcked)
                    elif s3msg_out.fpath == "":
                        print("Cannot process m3 capture message: " + s3msg_in.to_string())
                    else:
                        print("Cannot append line to file: " + s3msg_out.fpath)
            except:
                print("Cannot process m3 capture message: " + s3msg_in.to_string())
except KeyboardInterrupt:
    pass
finally:
    # Leave group and commit final offsets
    p.flush()
    p.close()
    c.close()



