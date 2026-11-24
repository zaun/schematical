#!/usr/bin/env python3
"""plan-55-F/G: combine the footprint readers' packets.

usage: python3 -I fp-combine.py <letter> <packet dir>...      e.g. fp-combine.py F /tmp/p55/fpackets/f0[1-8]

Writes planning/plan-55-data/footprints-<letter>.json (the new objects of the packets; the same id twice must be the
same object: the second is dropped; an id already in Parts/footprints.json is refused) and appends the packets' rows to
planning/plan-55-data/footprint-map.tsv (columns letter, company, package, supplier_package, pins, footprint, source,
drawing, pdf, note, mpn). Prints the counts.
"""
import csv
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

COLS = ['letter', 'company', 'package', 'supplier_package', 'pins', 'footprint', 'source', 'drawing', 'pdf', 'note', 'mpn']
letter = sys.argv[1]
dirs = sys.argv[2:]
library = {o['id'] for o in json.load(open(os.path.join(common.ROOT, 'schematical', 'schematical', 'Parts', 'footprints.json')))['footprints']}
objects = {}
rows = []
for d in dirs:
    fp = os.path.join(d, 'footprints.json')
    if os.path.exists(fp):
        for o in json.load(open(fp))['footprints']:
            if o['id'] in library:
                sys.exit(f"{d}: {o['id']} is already in Parts/footprints.json")
            if o['id'] in objects and objects[o['id']] != o:
                sys.exit(f"{d}: {o['id']} differs from the packet that wrote it first")
            objects.setdefault(o['id'], o)
    mp = os.path.join(d, 'map.tsv')
    if os.path.exists(mp):
        for r in csv.DictReader(open(mp), delimiter='\t'):
            if r['letter'] != letter:
                sys.exit(f"{d}: letter {r['letter']} is not {letter}")
            rows.append({c: r.get(c, '') or '' for c in COLS})
out = os.path.join(common.DATA, f'footprints-{letter}.json')
with open(out, 'w') as fh:
    fh.write('{"format":"schematical-footprints","version":1,"footprints":[\n')
    fh.write(',\n'.join('    ' + json.dumps(o, ensure_ascii=False, separators=(',', ':')) for o in objects.values()))
    fh.write('\n]}\n')
mapfile = os.path.join(common.DATA, 'footprint-map.tsv')
old = common.read_tsv(mapfile) if os.path.exists(mapfile) else []
keep = [r for r in old if r['letter'] != letter]
common.write_tsv(mapfile, COLS, keep + rows)
print(f'{len(objects)} new footprints in {os.path.basename(out)}; {len(rows)} map rows for {letter} ({len(keep)} of the other letter kept)')
