#!/usr/bin/env python3
import os

vals = [{'Q': [], 'R': []}] * 6
for fp in sorted(os.listdir("./")):
    if fp.endswith(".txt"):
        n = int(fp.split('_')[0]) - 1
        with open(fp) as fh:
            for line in fh:
                vals[n][line[0]].append(line[3:-1].split())

for i, x in enumerate(vals):
    i += 1
    
