#!/usr/bin/env python3
"""plan-55-B Phase 3: the URLs fetch.js downloads.

usage: python3 -I dl-urls.py candidates.tsv > urls.tsv      (url, company: the distinct `maker_url` of the candidate rows)
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

seen = {}
for r in common.read_tsv(sys.argv[1]):
    if common.eligible(r) and r['maker_url']:
        seen.setdefault(r['maker_url'], r['company'])
for u, c in sorted(seen.items()):
    print(f'{u}\t{c}')
