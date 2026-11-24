#!/usr/bin/env python3
"""plan-55-C: merge reader packets into specs.tsv.

usage: python3 -I merge-specs.py <packet.tsv>...

A packet is a TSV with the columns of specs.tsv (common.SPEC_COLS; extra columns are ignored, missing ones empty)
that a reader filled for the spec rows (company, series, pdf, package, supplier_package) assigned to it. For every
key in the packet, the key's rows of specs.tsv are replaced by the packet's rows for it (a key may have several
rows: two pin assignments or two enable polarities, told apart by `ordering`); `rows` and `example` are
recomputed from candidates.tsv by specs-skeleton.py, so a packet may leave them empty. Keys not in specs.tsv are
refused (print and exit 1). specs.tsv keeps the row order of its keys.
"""
import csv
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

specs = common.read_tsv(common.SPECS)
order = []
by_key = {}
for r in specs:
    k = common.spec_key(r)
    if k not in by_key:
        order.append(k)
    by_key.setdefault(k, []).append(r)
new = {}
bad = 0
for path in sys.argv[1:]:
    for r in csv.DictReader(open(path), delimiter='\t'):
        row = {c: r.get(c, '') or '' for c in common.SPEC_COLS}
        k = common.spec_key(row)
        if k not in by_key:
            print(f'{path}: no such spec row {k}')
            bad += 1
            continue
        new.setdefault(k, []).append(row)
if bad:
    sys.exit(1)
out = []
for k in order:
    for row in (new.get(k) or by_key[k]):
        base = by_key[k][0]
        row['rows'], row['example'] = base['rows'], base['example']
        out.append(row)
common.write_tsv(common.SPECS, common.SPEC_COLS, out)
print(f'{len(new)} keys replaced; {len(out)} rows in specs.tsv')
