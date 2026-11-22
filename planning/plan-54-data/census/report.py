#!/usr/bin/env python3
"""plan-54 census: the numbers plan-54 quotes, from the listing and candidates.tsv.

usage: python3 -I report.py raw candidates.tsv
"""
import collections
import csv
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

listing = common.read_listing(sys.argv[1])
print(f'listed: {len(listing)} parts')
stocked = collections.Counter()
nostock = collections.Counter()
for r in listing:
    key = (common.FAMILY[r['category']], common.MAKER[r['maker']])
    (stocked if (r.get('stock') or 0) > 0 else nostock)[key] += 1
for key in sorted(set(stocked) | set(nostock)):
    print(f'  {key[0]:6} {key[1]:18} with stock {stocked[key]:4}  without {nostock[key]:3}')
print('rows whose datasheet link resolves to a maker URL (common.src_for), of the stocked rows:')
res = collections.Counter()
for r in listing:
    if (r.get('stock') or 0) > 0:
        m = common.MAKER[r['maker']]
        if common.src_for(m, common.norm_url(r['datasheet']))[0]:
            res[(common.FAMILY[r['category']], m)] += 1
for key in sorted(stocked):
    print(f'  {key[0]:6} {key[1]:18} {res[key]:4} of {stocked[key]:4}')
rows = list(csv.DictReader(open(sys.argv[2]), delimiter='\t'))
cand = [r for r in rows if r['state'] == 'candidate']
print(f'candidates: {len(cand)}; out: ' + ', '.join(f'{k} {v}' for k, v in collections.Counter(r['reason'] for r in rows if r['state'] == 'out').most_common()))
for fam in ('card', 'module', 'edge'):
    c = [r for r in cand if r['family'] == fam]
    print(f'{fam}: {len(c)} parts; makers {dict(collections.Counter(r["maker"] for r in c))}; variants {len({r["variant"] for r in c})}; '
          f'footprint keys (variant, positions) {len({(r["variant"], r["positions"]) for r in c})}; '
          f'download sources {len({r["src"] for r in c})}; specs rows (maker, series, src) {len({(r["maker"], r["series"], r["src"]) for r in c})}')
print(f'download sources: {len({r["src"] for r in cand})}')
