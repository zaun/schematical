#!/usr/bin/env python3
"""plan-48-A phase 1: the rows of one state that have no stock-check row yet.
usage: python3 -I to-check.py <raw dir> <candidates.tsv> <stock-check.tsv> <state> [verdicts.tsv]
Prints `<DigiKey maker id> TAB <mpn>` per row of that state (`candidate` or
`reserve`), most-stocked first, skipping part numbers already in
<stock-check.tsv> and, when <verdicts.tsv> is given, the rows whose verdict
there is not `add` (a part its datasheet does not list needs no stock call:
plan-48-A Corrections 1); the counts go to stderr."""
import csv
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

raw, cand, done_path, state = sys.argv[1:5]
skip = set()
if len(sys.argv) > 5 and os.path.exists(sys.argv[5]):
    skip = {r['mpn'] for r in csv.DictReader(open(sys.argv[5], encoding='utf-8'), delimiter='\t') if r['verdict'] != 'add'}
done = set()
if os.path.exists(done_path):
    done = {r['mpn'] for r in csv.DictReader(open(done_path, encoding='utf-8'), delimiter='\t')}
ids = {r['mpn']: r['makerId'] for r in common.read_raw(raw)}
n = todo = 0
for r in csv.DictReader(open(cand, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE):
    if r['state'] != state:
        continue
    n += 1
    if r['mpn'] in done or r['mpn'] in skip:
        continue
    todo += 1
    print('%s\t%s' % (ids[r['mpn']], r['mpn']))
print('to-check: %d %s rows, %d unchecked' % (n, state, todo), file=sys.stderr)
