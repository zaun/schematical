#!/usr/bin/env python3
"""plan-34: one line per land pattern the kept rows need.

usage: python3 -I footprint-lines.py <candidates.tsv>  > footprints-needed.txt

A line is (category, maker, datasheet, package, body): a chip's Package /
Case; a through-hole part's body from Size / Dimension (the datasheet gives
its lead spacing); an array's Package / Case and pin count. Count = rows.
"""
import csv
import collections
import re
import sys

rows = [r for r in csv.DictReader(open(sys.argv[1]), delimiter='\t')
        if r['decision'] in ('candidate', 'add', 'pending')]
lines = collections.Counter()
for r in rows:
    url = r['datasheet_url']
    name = re.sub(r'[?].*$', '', url.rstrip('/').split('/')[-1])
    if r['category'] == 'through-hole':
        m = re.search(r'\(([^)]*)\)', r['size_dim'])
        body = m.group(1) if m else r['size_dim']
    elif r['category'] == 'array':
        body = f"{r['pins']} pins, {r['circuit']}"
    else:
        body = ''
    lines[(r['category'], r['maker'], name, r['package'], body)] += 1
print('# category\tmaker\tdatasheet\tpackage\tbody\trows')
for k in sorted(lines):
    print('\t'.join(k) + f'\t{lines[k]}')
