#!/usr/bin/env python3
"""plan-45-A Phase 3: the specs.tsv skeleton.

usage: python3 -I specs-skeleton.py <candidates.tsv> <specs.tsv>

One row per distinct (maker, variant, pdf) of the `candidate` rows with a placed PDF: `rows` the number
of candidate rows in the group, `example` its most-stocked part number. The columns plan-45-B fills
(`scheme` the ordering scheme as a regular expression over the part number, `volts` and `amps` the
rating the datasheet prints, `pinout` the pin rule, `note` the reading) are empty. Run again over an
existing specs.tsv it merges: a group's existing B columns are kept, a new group gets empty ones, and a
row whose group is gone is dropped and named.
"""
import collections
import csv
import os
import sys

COLS = ['maker', 'variant', 'pdf', 'rows', 'example', 'scheme', 'volts', 'amps', 'pinout', 'note']
KEEP = COLS[5:]


def main(src, dst):
    old = {}
    if os.path.exists(dst):
        for r in csv.DictReader(open(dst), delimiter='\t'):
            old[(r['maker'], r['variant'], r['pdf'])] = r
    groups = collections.defaultdict(list)
    for r in csv.DictReader(open(src), delimiter='\t'):
        if r['state'] in ('candidate', 'add') and r['pdf']:
            groups[(r['maker'], r['variant'], r['pdf'])].append(r)
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


main(*sys.argv[1:3])
