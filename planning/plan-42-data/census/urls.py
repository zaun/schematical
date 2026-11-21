#!/usr/bin/env python3
"""plan-42-A Phase 3: the distinct datasheet URLs fetch.js downloads.

usage: python3 -I urls.py <candidates.tsv> > urls.txt

One line per distinct common.sources() URL of the download-set rows, sorted
(`page:<url>` lines are Microchip product pages, section 4 rule 4). The md5 of
the line is the cache name fetch.js writes it to.
"""
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

rows = common.load(sys.argv[1])
common.init(rows)
rows = [r for r in rows if common.eligible(r)]
urls = sorted({u for r in rows for u in common.sources(r)})
for u in urls:
    print(u)
print(len(rows), "download-set rows;", len(urls), "distinct URLs;",
      sum(1 for r in rows if not common.sources(r)), "rows with no URL to fetch", file=sys.stderr)
