#!/usr/bin/env python3
"""plan-55 census: the distinct maker-URL chains of the candidate rows (plan-55-B Phase 2).

usage: python3 -I urls.py candidates.tsv > urls-needed.tsv
Columns: chain (the row's maker URLs, best first, joined by a space: common.alternatives; a Texas Instruments
part has up to three names to try), host (of the first), company, parts (the candidate rows with this chain).
A row with no maker URL has no chain (plan-55-B Phase 3 looks for its part number in a same-company PDF).
"""
import collections
import os
import sys
from urllib.parse import urlparse

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

chains = collections.OrderedDict()
for r in common.read_tsv(sys.argv[1]):
    if not common.eligible(r):
        continue
    alts = common.alternatives(r['company'], r['mpn'], r['datasheet_url'])
    if not alts:
        continue
    e = chains.setdefault((' '.join(alts), r['company']), [0])
    e[0] += 1
print('chain\thost\tcompany\tparts')
for (chain, company), (n,) in chains.items():
    print(f'{chain}\t{urlparse(chain.split()[0]).netloc}\t{company}\t{n}')
