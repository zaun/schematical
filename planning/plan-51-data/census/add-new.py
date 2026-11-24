#!/usr/bin/env python3
"""plan-51-B phase 3: adds the parts a fresh listing shows in scope that candidates.tsv lacks (a part that came
into stock since the first listing), as state 'candidate', checked <date>, citing the maker's catalog PDF.
usage: python3 -I add-new.py <date> <maker>=<id>:<dir> <pdf of the maker>   e.g.
  add-new.py 2026-11-24 Mill-Max=54:raw-recheck datasheets/ic-socket/mill-max/088-145.pdf
Also bumps the `parts` count of the PDF in its folder's datasheets.tsv."""
import os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import common

DATA = os.path.join(HERE, '..')
ROOT = os.path.join(DATA, '..', '..')
TSV = os.path.join(DATA, 'candidates.tsv')
date, spec, pdf = sys.argv[1:4]
maker, rest = spec.split('=')
mid, folder = rest.split(':')
common.RAW = os.path.join(DATA, folder)
lines = [l.rstrip('\n').split('\t') for l in open(TSV)]
header, rows = lines[0], lines[1:]
have = {r[1] for r in rows}
added = []
for r in common.rows(int(mid)):
    s = common.scope(r)
    if s and r['mpn'] not in have:
        rows.append([maker, r['mpn'], r['series'], s[0], str(s[1]), s[2], s[3], str(r['stock']), r['status'],
                     r['datasheet'], pdf, 'candidate', '', date])
        added.append(r['mpn'])
rows.sort(key=lambda r: (r[0], r[1]))
with open(TSV, 'w') as f:
    f.write('\t'.join(header) + '\n')
    for r in rows:
        f.write('\t'.join(r) + '\n')
sheets = os.path.join(ROOT, os.path.dirname(pdf), 'datasheets.tsv')
text = [l.rstrip('\n').split('\t') for l in open(sheets)]
for t in text[1:]:
    if t[0] == os.path.basename(pdf):
        t[3] = str(int(t[3]) + len(added))
with open(sheets, 'w') as f:
    f.write('\n'.join('\t'.join(t) for t in text) + '\n')
print('added', added)
