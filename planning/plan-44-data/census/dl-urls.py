#!/usr/bin/env python3
"""plan-44-A Phase 3 (section 4): the distinct URLs the download set cites.

usage: python3 -I dl-urls.py <candidates.tsv> <url-check.tsv>

Prints `url TAB via TAB snapshot` per distinct `url` of the download-set rows
(`state` is not `out`), sorted; `snapshot` is the Internet Archive timestamp
of an `archive` (Suntsu) URL from url-check.tsv, empty for `direct`. The
count goes to stderr.
"""
import csv
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402


def main():
    snap = {}
    for r in csv.DictReader(open(sys.argv[2]), delimiter='\t'):
        snap[r['url']] = r['bytes'] if r['via'] == 'archive' and r['pdf'] == '1' else ''
    urls = {}
    for r in csv.DictReader(open(sys.argv[1]), delimiter='\t'):
        if common.eligible(r) and r['url']:
            urls[r['url']] = r['via']
    for u in sorted(urls):
        print(f'{u}\t{urls[u]}\t{snap.get(u, "")}')
    print(f'dl-urls: {len(urls)} distinct URLs', file=sys.stderr)


main()
