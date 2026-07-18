# Go get the first N line of a compressed file, and save them as encrypted copy

import bz2
import sys
import traceback

def usage():
    print("Usage: bz2head.py <nb_lines> <source> <dest>")
    exit(-1)

# main
if len(sys.argv) != 4:
    usage()
try:
    nb_lines = int(sys.argv[1])
except Exception as exc:
    print('\nCode generated an exception: %s' % (exc))
    print("Cannot parse:\n" + sys.argv[1] + " as integer.")
    usage()

try:
    with bz2.open(sys.argv[2], "rt") as F:
        print("Opened " + sys.argv[2] + " for copying.")
        with bz2.open(sys.argv[3], "wt") as G:
            print("Opened " + sys.argv[3] + " for writing.")
            nb_copied = 1
            for line in F:
                if nb_copied > nb_lines:
                    break
                G.write(line)
                nb_copied += 1
            nb_copied -= 1
except Exception as exc:
    print('\nCode generated an exception: %s' % (exc))
    print("Cannot open:\n" + sys.argv[2] + " or " + sys.argv[3])
    usage()

print ("Copied " + str(nb_copied) + " lines from " + sys.argv[2] + " to " + sys.argv[3])
