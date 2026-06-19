import sys
import json
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import bz2


def json_pdns_parse(filepath):
    traces = []
    try:
        if filepath.endswith("bz2"):
            print(f"Open '{filepath}' as bz2")
            F = bz2.open(filepath,"rt")
        else:
            print(f"Open '{filepath}' as text")
            F = open(filepath,"rt")
        jpdns = json.load(F)
        if 'asns' in jpdns:
            print("ASNS vector has length: " + str(len(jpdns['asns'])))
        F.close()
    except FileNotFoundError:
        print(f"Error: File '{filepath}' not found")
        return None
    except PermissionError:
        print(f"Error: Permission denied reading '{filepath}'")
        return None
    except json.JSONDecodeError as e:
        print(f"Error: Invalid JSON in '{filepath}'")
        print(f"  {e.msg} at line {e.lineno}")
        return None

# test part of the program

print(sys.argv[1])
json_pdns_parse(sys.argv[1])
