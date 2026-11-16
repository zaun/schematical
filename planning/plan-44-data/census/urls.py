#!/usr/bin/env python3
"""plan-44 census: urls-needed.tsv, the maker-site datasheet URLs urlcheck.js must test.

usage: python3 -I urls.py <raw dir> > urls-needed.tsv

One line per distinct resolved URL (common.resolve_urls) of the rows that are in
scope and not duplicates: url, via (direct | archive), maker, rows, example part.
"""
import collections
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

rows = common.read_raw(sys.argv[1])
for r in rows:
    r['reason'] = common.scope(r)
common.dedupe(rows)
common.resolve_urls(rows)
by = collections.OrderedDict()
for r in sorted(rows, key=lambda r: (-r['stock'], r['mpn'])):
    if r['reason'] or not r['url']:
        continue
    by.setdefault(r['url'], []).append(r)
print('url\tvia\tmaker\trows\texample')
for url, g in sorted(by.items()):
    print('\t'.join([url, g[0]['via'], g[0]['maker'], str(len(g)), g[0]['mpn']]))
