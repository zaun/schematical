#!/usr/bin/env python3
"""plan-47-A Phase 3: the specs.tsv skeleton.

usage: python3 -I specs-skeleton.py <candidates.tsv> <specs.tsv>

One row per distinct (type, maker, series, pdf, DigiKey Package / Case) of the
download-set rows with a PDF (`pdf`, the path place.py wrote): `rows` the number
of such rows in the group, `example` its most-stocked part number. The columns
`scheme`, `label`, `page`, `pinout`, `terminals` and plan-47-B's `pads` and fact columns
(specs47.py) are empty (plan-47-B and C fill them). Run again over an existing specs.tsv it merges: a group's existing
columns are kept, a new group gets empty ones, and a row whose group is gone is
dropped and named.
"""
import collections
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import scancommon as sc  # noqa: E402

FACTS = ['pads', 'values', 'tols', 'amps_max', 'isat_max', 'dcr_max', 'shield', 'speed', 'vac', 'vdc', 'break_max', 'vmax', 'imax',
         'rtype', 'coils', 'polarized', 'latching', 'contacts', 'coilv', 'coil_ohms', 'amps', 'op', 'rel', 'ron',
         'driver', 'technology', 'hz', 'volts', 'vrange', 'ma_max', 'db']
COLS = ['type', 'maker', 'series', 'pdf', 'package', 'rows', 'example', 'scheme', 'label', 'page', 'pinout', 'terminals'] + FACTS
KEEP = ['scheme', 'label', 'page', 'pinout', 'terminals'] + FACTS
KEY = ['type', 'maker', 'series', 'pdf', 'package']


def main(src, dst):
    old = {}
    if os.path.exists(dst):
        for r in sc.read(dst):
            old[(r['type'], r['maker'], r['series'], r['pdf'], r['package'])] = r
    groups = collections.defaultdict(list)
    for r in sc.read(src):
        if sc.in_set(r) and r['pdf']:
            pkg = (f"{r['package']} / {r['form']} / {r['detail'].split(';')[0]}"
                   if r['type'] == 'relay' else r['package'])
            groups[(r['type'], r['company'], r['series'], r['pdf'], pkg)].append(r)
    rows = []
    for g in sorted(groups):
        rs = groups[g]
        example = max(rs, key=lambda r: (int(r['stock'] or 0), r['mpn']))['mpn']
        row = dict(zip(KEY, g))
        row['rows'], row['example'] = str(len(rs)), example
        for c in KEEP:
            row[c] = old[g].get(c, '') if g in old else ''
        rows.append(row)
    sc.write(dst, rows, COLS)
    for g in sorted(set(old) - set(groups)):
        print('dropped', '|'.join(g))
    print(len(groups), 'rows;', sum(1 for g in groups if g in old), 'kept from before')


if __name__ == '__main__':
    main(*sys.argv[1:3])
