#!/usr/bin/python
# coding=utf-8
#
# Usage: SumM3Producer.py <summary_file_path>
#
# This script is launched when a summary file has been produced by ithitools.
# The file name is expected to be formated as expected by M3Name.
#
# TODO: import the configuration from a configuration file describing the
# topology for this kafka pipeline.
#

from confluent_kafka import Producer
import sys
import codecs
from enum import Enum
import copy
import traceback
import datetime
import math
import m3name
from SumM3Lib import sumM3Message

# callback function that will be used after sending a message.
# TODO: move a Kafka Common library.
def captureAcked(err, msg):
    if err is not None:
        print("Failed to deliver M3 summary message: %s: %s" % (str(msg), str(err)))
        exit(1)
    else:
        print("Message Acked")
    return

# check the calling argument
if len(sys.argv) != 3:
    print("Usage: " + sys.argv[0] + " <bootstrap.servers> <m3-capture-file>\n")
    exit(1)
file_name = sys.argv[2]

print("bootstrap.servers: " + sys.argv[1])
print("Capture file: " + file_name)

# parse the name
m3n = m3name.m3name()
if m3n.parse_file_id(file_name) != 0:
    print("Invalid name: " + file_name);
    exit(1)
capMsg = sumM3Message()
capMsg.topic="m3Capture"
capMsg.copy_m3name_values(m3n)
capMsg.fpath = file_name
captureMessage = capMsg.to_string()
print("Message: " + captureMessage)

# Send the message on the "m3Summary" topic
try:
    #define the producer configuration
    p = Producer({'bootstrap.servers': sys.argv[1]})
    print("Producer started")
    # Produce a message
    p.produce(capMsg.topic, captureMessage.encode(encoding='utf-8', errors='strict'), callback=captureAcked)
    print("Message produced")
    # Wait up to 1 second for events. Callbacks will be invoked during
    # this method call if the message is acknowledged.
    p.poll(1)
    print("Poll done")
except Exception:
    traceback.print_exc()
    print("Failed to produce the M3 summary message!")
    exit(1)

# flush and exit after the message is normally sent.
print("Flushing")
p.flush()
exit(0)
