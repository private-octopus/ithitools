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
        try:
            s3msg_in = sumM3Message()
            s3msg_in.poll_kafka(c, 300.0)
            if s3msg_in.topic == "":
                print("No good message for 300 sec.")
            else:
                # Check whether this message triggers a threshold
                if thr.checkList(s3msg_in):
                    # this message needs re-broadcasting
                    msg = thr.node_list[s3msg_in.node_dns].to_string()
                    print("Sending: " + msg)
                    p.produce("m3Thresholder", msg.encode(encoding='utf-8', errors='strict'), callback=m3ThresholderAcked)
                    thr.update(s3msg_in)
        except KeyboardInterrupt:
            break;
        except Exception:
            traceback.print_exc()
            print("Cannot process m3analysis message: " + s3msg_in.to_string())
            break

except KeyboardInterrupt:
    pass
finally:
    # Leave group and commit final offsets
    p.flush()
    c.close()



