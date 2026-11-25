#!/usr/bin/env python3
"""plan-47-D section 3.3: the footprint-map.tsv skeleton.

usage: python3 -I map-skeleton.py > footprint-map.tsv

One row per line of footprints-needed.txt (every letter): the first nine columns of the
line, then `footprint`, `label`, `note` empty, and `series_split` empty (a row that serves
only one series of a line carries that series there). Run it again only into a new file:
it does not merge. To follow a changed footprints-needed.txt, fp-check.py names the lines
that have no row.
"""
import os
import sys

DATA = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
KEY = ['letter', 'type', 'company', 'series', 'mounting', 'package', 'supplier_package', 'size', 'detail']

print('\t'.join(KEY + ['footprint', 'label', 'note', 'series_split']))
for line in open(os.path.join(DATA, 'footprints-needed.txt'), encoding='utf-8'):
    if line.startswith('#') or not line.strip():
        continue
    f = line.rstrip('\n').split('\t')
    print('\t'.join(f[:9] + ['', '', '', '']))
