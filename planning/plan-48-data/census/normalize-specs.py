#!/usr/bin/env python3
"""plan-48-B: an `explicit` pinout equal, pin for pin, to a named scheme of
the same type and circuits becomes that scheme (pinout empty).
usage: python3 -I normalize-specs.py [<specs.tsv>]"""
import csv
import importlib.util
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location('pc', os.path.join(HERE, 'pinout-check.py'))
pc = importlib.util.module_from_spec(spec)
spec.loader.exec_module(pc)
path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, '..', 'specs.tsv')
rows = list(csv.DictReader(open(path, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE))
cols = list(rows[0].keys())
n = 0
for r in rows:
    if r['scheme'] != 'explicit' or r['pinout'] in ('', '-'):
        continue
    pairs = pc.parse_pinout(r['pinout'])
    names = [x for _, x in pairs]
    for name, pins in pc.SCHEMES.items():
        if pc.SCHEME_FOR[name] == (r['type'], r['circuits']) and pins == names \
                and [d for d, _ in pairs] == [str(i) for i in range(1, len(pins) + 1)]:
            r['scheme'], r['pinout'] = name, ''
            n += 1
            break
with open(path, 'w', encoding='utf-8', newline='') as f:
    f.write('\t'.join(cols) + '\n')
    for r in rows:
        f.write('\t'.join(r[c] for c in cols) + '\n')
print('%d explicit rows became a named scheme' % n)
