#!/usr/bin/env python3
"""plan-34-A § 5 rule 2: the picked rows that came from a cut piece.

usage: python3 -I cut-picked.py <raw dir> <candidates.tsv> [<stock-check.tsv>]

A row came from a cut piece when its piece label, or a piece label it
starts with (the base piece a recheck.sh re-listing split), has a
`piece <label>: 300 of N (cut at 300: S)` line in an .err file of its
(category, maker). Picked means decision candidate or add. Prints
`<maker id> TAB <mpn>` per such row, skipping part numbers already in
<stock-check.tsv> when given; the count goes to stderr.
"""
import csv
import glob
import os
import re
import sys

MAKER = {'viking': ('Viking Tech', 2577), 'yageo': ('YAGEO', 13), 'dale': ('Vishay Dale', 541)}
CAT = {'52': 'chip', '53': 'through-hole', '50': 'array'}


def cut_pieces(raw):
    cut = set()
    for err in glob.glob(os.path.join(raw, '5[023]-*.err')):
        stem = os.path.basename(err)[:-4]
        cat, slug = stem.split('-')[:2]
        for line in open(err):
            m = re.match(r'piece (.*): 300 of \d+ \(cut at 300: \d+\)', line)
            if m:
                cut.add((CAT[cat], MAKER[slug][0], m.group(1)))
    return cut


def from_cut(row, cut):
    label = row['piece']
    parts = label.split('/')
    return any((row['category'], row['maker'], '/'.join(parts[:i])) in cut
               for i in range(1, len(parts) + 1))


def main():
    raw, cand = sys.argv[1], sys.argv[2]
    done = set()
    if len(sys.argv) > 3 and os.path.exists(sys.argv[3]):
        done = {r['mpn'] for r in csv.DictReader(open(sys.argv[3]), delimiter='\t')}
    cut = cut_pieces(raw)
    ids = {name: i for name, i in MAKER.values()}
    n = todo = 0
    for r in csv.DictReader(open(cand), delimiter='\t'):
        if r['decision'] not in ('candidate', 'add') or not from_cut(r, cut):
            continue
        n += 1
        if r['mpn'] in done:
            continue
        todo += 1
        print(f"{ids[r['maker']]}\t{r['mpn']}")
    print(f'cut-picked: {n} picked rows from cut pieces, {todo} unchecked', file=sys.stderr)


if __name__ == '__main__':
    main()
