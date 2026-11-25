#!/usr/bin/env python3
"""plan-48-C/D: collect the slice outputs of the footprint readers.
usage: python3 -I fp-collect.py <fpout dir>
Reads <dir>/fp-<L>-NN.json and fp-<L>-NN-map.tsv; writes
planning/plan-48-data/footprints-<L>.json (one object per line, ids unique;
an id given twice must be the same object) and footprint-map-<L>.tsv.
Prints the counts and every duplicate."""
import csv
import glob
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
src = sys.argv[1]
COLS = ['letter', 'company', 'package', 'supplier', 'footprint', 'note', 'pdf']
bad = 0
for L in 'CD':
    objs, seen = [], {}
    for p in sorted(glob.glob(os.path.join(src, 'fp-%s-[0-9][0-9].json' % L))):
        for o in json.load(open(p))['footprints']:
            if o['id'] in seen:
                if seen[o['id']] != o:
                    print('duplicate id with a different object:', o['id'], p)
                    bad += 1
                continue
            if o['style'] == 'dual' and o['pins'] % 2 and 'perSide' not in o:
                # an odd pin count needs perSide: 0 in slots is the exposed pad (a pin on no slot)
                o['perSide'] = (o['pins'] - 1) // 2 if 0 in o.get('slots', []) else (o['pins'] + 1) // 2
            seen[o['id']] = o
            objs.append(o)
    with open(os.path.join(DATA, 'footprints-%s.json' % L), 'w', encoding='utf-8') as f:
        f.write('{"format": "schematical-footprints", "version": 1, "footprints": [\n')
        f.write(',\n'.join('  ' + json.dumps(o, ensure_ascii=False, separators=(',', ':')) for o in objs))
        f.write('\n]}\n')
    rows = []
    for p in sorted(glob.glob(os.path.join(src, 'fp-%s-[0-9][0-9]-map.tsv' % L))):
        for r in csv.DictReader(open(p, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE):
            rows.append([r[c] for c in COLS])
    with open(os.path.join(DATA, 'footprint-map-%s.tsv' % L), 'w', encoding='utf-8') as f:
        f.write('\t'.join(COLS) + '\n')
        for r in rows:
            f.write('\t'.join(r) + '\n')
    print('%s: %d new footprints, %d map rows' % (L, len(objs), len(rows)))
sys.exit(1 if bad else 0)
