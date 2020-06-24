#!/usr/bin/env python
# coding=utf-8
#
# ITHI Kafka prototype, consume M3 analysis as they are produced, creates and updates SumM3 files
import sys
import codecs
import datetime
from enum import Enum
import copy
import traceback
import datetime
import math
import m3name
import m3summary
from confluent_kafka import Consumer, Producer
from SumM3Lib import sumM3Message, sumM3Thresholder

# Ack function to detect whether Kafka is still running.
def m3ThresholderAcked(err, msg):
    if err is not None:
        print("Failed to deliver sumM3 threshold message: %s: %s" % (str(msg), str(err)))
        exit(1)

# check the calling arguments
if len(sys.argv) != 3:
    print("Usage: " + sys.argv[0] + " <bootstrap.servers> <nb_hours>\n")
    exit(1)
try:
    nb_hours = int(sys.argv[2], 10)
except:
    print("Cannot parse the number of hours: " + sys.argv[2]);
    exit(1)

   
print("bootstrap.servers: " + sys.argv[1])
print("nb hours: " + str(nb_hours))

# create a table of node instances
thr = sumM3Thresholder(nb_hours)

# Create Kafka Consumer instance
c = Consumer({
    'bootstrap.servers': sys.argv[1],
    'group.id': 'sumM3Consumer'
})

# Subscribe to topic 'm3Analysis'
c.subscribe(['m3Analysis'])

# Create a provider instance.
p = Producer({'bootstrap.servers': sys.argv[1]})

# Process messages
try:
    while True:
        msg = c.poll(60.0)
        if msg is None:
            # print a log message on time out.
            # maybe should also send a kafka message to 
            print("Waiting for m3Analysis message or event/error in poll()")
            continue
        elif msg.error():
            print('error: {}'.format(msg.error()))
        else:
            try:
                # The Kafka message should provide keys of this summary: location and date,
                # and the name of the file.
                # Parse the message value as sum M3 message
                s3msg_in = sumM3Message("")
                if not s3msg.parse(str(msg.value())):
                    print("unexpected m3analysis message: " + str(record_value))
                else:
                    # Check whether this message triggers a threshold
                    if thr.checkList(s3msg):
                        # this message needs re-broadcasting
                        msg = thr.node_list[s3msg.node_dns].to_string()
                        p.produce(s3msg_out.topics, msg.encode(encoding='utf-8', errors='strict'), callback=m3ThresholderAcked)
                        thr.update(s3msg)
            except:
                print("Cannot process m3analysis message: " + str(msg.value()))

except KeyboardInterrupt:
    pass
finally:
    # Leave group and commit final offsets
    p.flush()
    p.close()
    c.close()



