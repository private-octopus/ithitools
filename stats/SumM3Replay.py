#!/usr/bin/env python
# coding=utf-8
#
# ITHI Kafka prototype, replay of summary folder
# Usage: 
#     SumM3Replay.py <folder>
# This will find all the summaries in the folder, and create
# an M3 summary message

import sys
import codecs
from enum import Enum
import copy
import traceback
import datetime
import math
import m3name
import os
from os.path import isfile, join
from confluent_kafka import Producer

def load_m3(file_name, sum_m3):
    m3sl = m3summary.m3summary_line()
    if m3sl.load_m3(file_name) != 0:
        return -1
    sum_m3.write(m3sl.to_string() +  "\n")
    return 0


def load_folder(mypath, name_sum_f3):
    sum_m3 = codecs.open(name_sum_f3, "w", "UTF-8")
    sum_m3.write(m3summary.summary_title_line() + "\n")

    file_list = []
    m3summary.ithiwalk(file_list,mypath)
    nb_loaded = 0
    for file in file_list:
        # If this is an M3 capture file, add it.
        if load_m3(file, sum_m3) == 0:
            nb_loaded += 1
            if (nb_loaded%1000) == 0:
                print(str(nb_loaded))
    sum_m3.close()
    print("In " + mypath + " found " + str(len(file_list)) + " files, loaded " + str(nb_loaded) + " summaries.")


# Create a provider instance.
# TODO: move to a common create produce function.
p = Producer({'bootstrap.servers': "localhost:9092"})
if err is not None:
    print("Failed to create producer: %s: %s" % (str(p), str(err)))
    exit(1)

# Process messages
total_count = 0
try:
    while True:
        msg = c.poll(60.0)
        if msg is None:
            # print a log message on time out.
            # maybe should also send a kafka message to 
            print("Waiting for m3Summary message or event/error in poll()")
            continue
        elif msg.error():
            print('error: {}'.format(msg.error()))
        else:
            # The Kafka message should provide keys of this summary: location and date,
            # and the name of the file.
            record_key = msg.key()
            record_value = msg.value()
            # Parse the message value as M3Name data and file name

            # Read the M3 summary file

            # Compute the Sum M3 line

            # Forward the Sum M3 line.
            total_count += 1
            print("Consumed record with key {} and value {}, count {}"
                   .format(record_key, record_value, str(total_count)))
except KeyboardInterrupt:
    pass
finally:
    # Leave group and commit final offsets
    c.close()



