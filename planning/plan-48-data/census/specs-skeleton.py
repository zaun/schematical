#!/usr/bin/env python3
"""plan-48-A phase 3: the specs.tsv skeleton.
usage: python3 -I specs-skeleton.py <candidates.tsv> <specs.tsv>

One row per distinct (type, company, pdf, package, supplier_package,
circuits) of the rows of state `candidate` (after phase 3: the `add` rows;
`pdf` is the path place.py wrote): `rows` the number of such rows, `example`
the most-stocked part number. The columns `pinout`, `scheme`, `facts` are
empty (plan-48-B fills them). Run again over an existing specs.tsv it merges:
a group's existing `pinout`, `scheme`, `facts` are kept, a new group gets
empty ones, a group that is gone is dropped and named."""
import collections
import csv
import os
import sys

COLS = ['type', 'company', 'pdf', 'package', 'supplier_package', 'circuits', 'rows', 'example',
        'pinout', 'scheme', 'facts']
KEEP = ['pinout', 'scheme', 'facts']
KEY = COLS[:6]


def main(src, dst):
    old = {}
    if os.path.exists(dst):
        for r in csv.DictReader(open(dst, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE):
            old[tuple(r[c] for c in KEY)] = r
    groups = collections.defaultdict(list)
    for r in csv.DictReader(open(src, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE):
        if r['state'] == 'candidate' and r['pdf']:
            groups[tuple(r[c] for c in KEY)].append(r)
    with open(dst, 'w', encoding='utf-8', newline='') as out:
        out.write('\t'.join(COLS) + '\n')
        for g in sorted(groups):
            rs = groups[g]
            example = max(rs, key=lambda r: (int(r['stock'] or 0), r['mpn']))['mpn']
            keep = [old[g][c] for c in KEEP] if g in old else [''] * len(KEEP)
            out.write('\t'.join(list(g) + [str(len(rs)), example] + keep) + '\n')
    for g in sorted(set(old) - set(groups)):
        print('dropped', '|'.join(g))
    print('%d rows; %d kept from before' % (len(groups), sum(1 for g in groups if g in old)))


if __name__ == '__main__':
    main(*sys.argv[1:3])
