#!/usr/bin/env python3
"""plan-51-C phase 2: marks candidates out.  usage: python3 -I mark-out.py <maker> <series> <family> <mounting> <reason>
Every candidate row of that group gets state `out` and the reason (column 13). Prints the number of rows."""
import os, sys

TSV = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'candidates.tsv')
maker, series, family, mounting, reason = sys.argv[1:6]
lines = [l.rstrip('\n').split('\t') for l in open(TSV)]
n = 0
for r in lines[1:]:
    if (r[0], r[2], r[3], r[6]) == (maker, series, family, mounting):
        r[11], r[12] = 'out', reason
        n += 1
with open(TSV, 'w') as f:
    f.write('\n'.join('\t'.join(r) for r in lines) + '\n')
print(n, 'rows out:', maker, series, family, mounting)
