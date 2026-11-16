#!/usr/bin/env python3
"""plan-45 census: footprints-needed.txt, one line per variant (footprint family) of the candidate rows.

usage: python3 -I footprint-lines.py candidates.tsv > footprints-needed.txt
Columns: letter (C fixed terminal blocks, D pluggable headers, E JST), variant, maker, series, pitch,
orientation, mounting, positions (the distinct position counts of the candidate rows), parts (rows),
existing (the `PINHDR-` / `TBS-` ids in schematical/schematical/Parts/footprints.json whose pitch
equals the variant's: only 2.54 mm pin headers exist, so the column is empty for every variant),
label (the id stem a footprint of the variant gets: <MAKER>-<SERIES>-<PITCH>-<V|H|A><T|S>; the
position count is appended: -02 … -12).
"""
import collections
import csv
import json
import re
import os
import sys

rows = [r for r in csv.DictReader(open(sys.argv[1]), delimiter='\t') if r['state'] == 'candidate']
fps = [f for f in json.load(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', 'schematical', 'schematical', 'Parts', 'footprints.json')))['footprints']]
by = collections.OrderedDict()
for r in rows:
    by.setdefault(r['variant'], []).append(r)
print('# letter\tvariant\tmaker\tseries\tpitch\torientation\tmounting\tpositions\tparts\texisting\tlabel')
for v, rs in by.items():
    r0 = rs[0]
    letter = 'E' if r0['group'] == 'jst' else ('D' if r0['class'] == 'pluggable' else 'C')
    pos = sorted({int(r['positions']) for r in rs})
    pitch = float(r0['pitch'])
    existing = [f['id'] for f in fps if abs(f.get('pitch', 0) - pitch) < 0.001 and f.get('style') in ('header', 'dual')
                and f['id'].startswith(('PINHDR-', 'TBS-'))]
    stem = re.sub(r'[^A-Z0-9.]+', '-', (r0['maker'].split()[0][:3] + '-' + r0['series']).upper()).strip('-')
    label = f"{stem}-{pitch:.2f}-{r0['orientation'][:1].upper()}{'S' if r0['mounting'] == 'smd' else 'T'}"
    print('\t'.join([letter, v, r0['maker'], r0['series'], r0['pitch'], r0['orientation'], r0['mounting'],
                     ','.join(map(str, pos)), str(len(rs)), ','.join(existing), label]))
