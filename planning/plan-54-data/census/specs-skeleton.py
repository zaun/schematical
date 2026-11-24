#!/usr/bin/env python3
"""plan-54-A Phase 3: the specs.tsv skeleton.

usage: python3 -I specs-skeleton.py candidates.tsv specs.tsv

One row per distinct (family, maker, series, pdf) of the `candidate` rows that have a PDF (`pdf`, the path place.py
wrote): `rows` the number of candidate rows in the group, `example` its most-stocked part number. The columns
`scheme`, `label`, `card`, `pinout`, `positions`, `pitch`, `status` are empty (plan-54-B, C and D fill them). Run again over an
existing specs.tsv it merges: a group's existing scheme columns are kept, a new group gets empty ones, and a row
whose group is gone is dropped and named.
"""
import collections
import csv
import os
import sys

COLS = ['family', 'maker', 'series', 'pdf', 'rows', 'example', 'scheme', 'label', 'card', 'pinout', 'positions', 'pitch', 'status']
KEEP = COLS[6:]


def main(src, dst):
    old = {}
    if os.path.exists(dst):
        for r in csv.DictReader(open(dst), delimiter='\t'):
            old[(r['family'], r['maker'], r['series'], r['pdf'])] = r
    groups = collections.defaultdict(list)
    for r in csv.DictReader(open(src), delimiter='\t'):
        if r['state'] in ('candidate', 'add') and r['pdf']:
            groups[(r['family'], r['maker'], r['series'], r['pdf'])].append(r)
    with open(dst, 'w') as out:
        out.write('\t'.join(COLS) + '\n')
        for g in sorted(groups):
            rs = groups[g]
            example = max(rs, key=lambda r: (int(r['stock'] or 0), r['mpn']))['mpn']
            keep = [old[g].get(c, '') for c in KEEP] if g in old else [''] * len(KEEP)
            out.write('\t'.join(list(g) + [str(len(rs)), example] + keep) + '\n')
    for g in sorted(set(old) - set(groups)):
        print('dropped', '|'.join(g))
    per = collections.Counter(g[0] for g in groups)
    print(len(groups), 'rows;', sum(1 for g in groups if g in old), 'kept from before;', dict(per))


main(*sys.argv[1:3])
