#!/usr/bin/env python3
"""plan-51-B phase 3: the stock re-check of candidates.tsv.
usage: python3 -I recheck.py <date> <maker=listing> ...   e.g.
  recheck.py 2026-11-24 Mill-Max=54:raw-recheck Preci-Dip=1212:raw Assmann=123:raw
A listing is <maker id>:<dir under planning/plan-51-data>, a fresh census/list.js run (<id>.jsonl[.gz]).
A candidate still listed there and still passing common.scope (in stock, Active or Last Time Buy,
the same kind of socket) gets stock_checked = <date> and its stock and status refreshed; any other
becomes state 'out', reason 'no stock', stock_checked = <date>. It refuses a file that is already checked."""
import os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import common

DATA = os.path.join(HERE, '..')
TSV = os.path.join(DATA, 'candidates.tsv')
date = sys.argv[1]
fresh = {}
for spec in sys.argv[2:]:
    maker, rest = spec.split('=')
    mid, folder = rest.split(':')
    common.RAW = os.path.join(DATA, folder)
    fresh[maker] = {r['mpn']: r for r in common.rows(int(mid))}
lines = [l.rstrip('\n').split('\t') for l in open(TSV)]
header, rows = lines[0], lines[1:]
if any(r[13] for r in rows):
    raise SystemExit('stock_checked is already filled')
kept = out = 0
for r in rows:
    new = fresh[r[0]].get(r[1])
    scope = common.scope(new) if new else None
    if scope and scope[0] == r[3] and str(scope[1]) == r[4]:
        r[7], r[8], r[13] = str(new['stock']), new['status'], date
        kept += 1
    else:
        r[11], r[12], r[13] = 'out', 'no stock', date
        out += 1
with open(TSV, 'w') as f:
    f.write('\t'.join(header) + '\n')
    for r in rows:
        f.write('\t'.join(r) + '\n')
print(f'{kept} still in stock, {out} out')
for r in rows:
    if r[11] == 'out':
        print('out:', r[0], r[1], r[2], r[3], r[4], r[6])
