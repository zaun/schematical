#!/usr/bin/env python3
"""plan-55-B Phase 3: the specs.tsv skeleton (the columns of plan-55-C § 3.1).

usage: python3 -I specs-skeleton.py <candidates.tsv> <specs.tsv>

One row per distinct (company, series, pdf, package, supplier_package) of the candidate rows with a PDF (`pdf`, the
path place.py wrote): `package` and `supplier_package` are DigiKey's `Package / Case` and `Supplier Device
Package`; `rows` the number of candidate rows in the group and `example` its most-stocked part number (both
beside the C columns, for the reader). The columns `ordering`, `pinout`, `type`, `switches`, `analog`, `vin_min`,
`vin_max`, `iout`, `ilimit`, `ron`, `iq`, `flags` and `note` are empty (C, D and E fill them, plan-55-C § 3.1; a
series with two enable polarities or two pin assignments in one package becomes two rows then). Run again over
an existing specs.tsv it merges: a group's existing columns are kept, a new group gets empty ones, and a row
whose group is gone is dropped and named.
"""
import collections
import csv
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

COLS = ['company', 'series', 'pdf', 'package', 'supplier_package', 'rows', 'example', 'ordering', 'pinout', 'type',
        'switches', 'analog', 'vin_min', 'vin_max', 'iout', 'ilimit', 'ron', 'iq', 'flags', 'note']
KEEP = COLS[7:]


def main(src, dst):
    old = {}
    if os.path.exists(dst):
        for r in csv.DictReader(open(dst), delimiter='\t'):
            old[(r['company'], r['series'], r['pdf'], r['package'], r['supplier_package'])] = r
    groups = collections.defaultdict(list)
    for r in common.read_tsv(src):
        if common.eligible(r) and r['pdf']:
            groups[(r['company'], r['series'], r['pdf'], r['package'], r['supplier_package'])].append(r)
    with open(dst, 'w') as out:
        out.write('\t'.join(COLS) + '\n')
        for g in sorted(groups):
            rs = groups[g]
            example = max(rs, key=lambda r: (int(r['stock'] or 0), r['mpn']))['mpn']
            keep = [old[g].get(c, '') for c in KEEP] if g in old else [''] * len(KEEP)
            out.write('\t'.join(list(g) + [str(len(rs)), example] + keep) + '\n')
    for g in sorted(set(old) - set(groups)):
        print('dropped', '|'.join(g))
    print(len(groups), 'rows;', sum(1 for g in groups if g in old), 'kept from before')
    return 0


if __name__ == '__main__':
    sys.exit(main(*sys.argv[1:3]))
