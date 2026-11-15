#!/usr/bin/env python3
"""plan-40-A section 5 rule 2: the picked rows that came from a cut piece.

usage: python3 -I cut-picked.py <raw dir> <candidates.tsv> [<stock-check.tsv>]

Picked means reason `candidate`; from a cut piece means `cut` = 1
(select.py). Prints `<maker id> TAB <mpn>` per such row (the maker id is the
row's own DigiKey id, from the raw listing: an onsemi row may be Fairchild's
600039), skipping part numbers already in <stock-check.tsv> when given; the
counts go to stderr.
"""
import csv
import glob
import json
import os
import sys


def main():
    raw, cand = sys.argv[1], sys.argv[2]
    done = set()
    if len(sys.argv) > 3 and os.path.exists(sys.argv[3]):
        done = {r['mpn'] for r in csv.DictReader(open(sys.argv[3]), delimiter='\t')}
    ids = {}
    for path in sorted(glob.glob(os.path.join(raw, '*.jsonl'))):
        for line in open(path):
            d = json.loads(line)
            ids.setdefault(d['mpn'], d['makerId'])
    n = todo = 0
    for r in csv.DictReader(open(cand), delimiter='\t'):
        if r['reason'] != 'candidate' or r['cut'] != '1':
            continue
        n += 1
        if r['mpn'] in done:
            continue
        todo += 1
        print(f"{ids[r['mpn']]}\t{r['mpn']}")
    print(f'cut-picked: {n} picked rows from cut pieces, {todo} unchecked', file=sys.stderr)


if __name__ == '__main__':
    main()
