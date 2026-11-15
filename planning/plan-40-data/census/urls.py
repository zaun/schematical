#!/usr/bin/env python3
"""plan-40-A Phase 3: the distinct datasheet URLs fetch.js downloads.

usage: python3 -I urls.py <candidates.tsv> > urls.txt

One line per distinct common.source() URL of the eligible rows, sorted.
The md5 of the line is the cache name fetch.js writes it to.
"""
import csv
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

rows = [r for r in csv.DictReader(open(sys.argv[1]), delimiter="\t") if common.eligible(r)]
urls = sorted({common.source(r) for r in rows} - {None})
for u in urls:
    print(u)
print(len(rows), "eligible rows;", len(urls), "distinct URLs;",
      sum(1 for r in rows if common.source(r) is None), "rows with no URL to fetch", file=sys.stderr)
