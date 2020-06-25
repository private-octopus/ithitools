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
from SumM3Lib import sumM3FileSeparator, sumM3EnsureEndInSep, sumM3Message
import os

def m3AnalysisAcked(err, msg):
    if err is not None:
        print("Failed to deliver sumM3 analysis message: %s: %s" % (str(msg), str(err)))
        exit(1)

# check the calling argument
if len(sys.argv) != 3:
    print("Usage: " + sys.argv[0] + " <bootstrap.servers>  <ithi_dir>\n")
    exit(1)
base_dir = sys.argv[2]
sep = sumM3FileSeparator(base_dir)
base_dir = sumM3EnsureEndInSep(base_dir, sep)

print("bootstrap.servers: " + sys.argv[1])
print("ithi directory: " + base_dir)

# Create Consumer instance
c = Consumer({
    'bootstrap.servers': sys.argv[1],
    'group.id': 'sumM3Consumer'
})

# Subscribe to topic 'm3Capture'
c.subscribe(['m3Capture'])

# Create a provider instance.
# TODO: move to a common create produce function.
try:
    p = Producer({'bootstrap.servers': sys.argv[1]})
except:
    print("Failed to create producer : " + str(p))
    exit(1)

# Process messages
total_count = 0
exit_code = 0
try:
    while True:
        try:
            s3msg_in = sumM3Message()
            s3msg_in.poll_kafka(c, 300.0)
            if s3msg_in.topic == "":
                print("No good message for 300 sec.")
            else:
                #add a line to the selected sumM3 file. Return the associated M3 Name
                s3msg_out = s3msg_in.sumM3AppendLine(base_dir, sep)
                # if all went well, produce the message for the next stage 
                if s3msg_out.topic != "":
                    msg = s3msg_out.to_string()
                    print("Sending: " + msg)
                    p.produce(s3msg_out.topic, msg.encode(encoding='utf-8', errors='strict'), callback=m3AnalysisAcked)
                elif s3msg_out.fpath == "":
                    print("Cannot process m3 capture message: " + s3msg_in.to_string())
                    print("Got: " + msg)
                else:
                    print("Cannot append line to file: " + s3msg_out.fpath)
        except KeyboardInterrupt:
            break
        except Exception:
            traceback.print_exc()
            print("Exception processing m3 capture message: " + s3msg_in.to_string())
            exit_code = 1
            break
except KeyboardInterrupt:
    pass
except Exception:
    traceback.print_exc()
    exit_code = 1
finally:
    # Leave group and commit final offsets
    p.flush()
    c.close()
    exit(exit_code)



