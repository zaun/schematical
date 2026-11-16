#!/usr/bin/env python3
"""plan-45 census: the distinct datasheet URLs of the candidate rows.

usage: python3 -I urls.py <candidates.tsv> > urls-needed.tsv
Columns: url (the maker-site URL the datasheets.json entry names), src (where the file is
downloaded from: the same, or DigiKey's copy for Phoenix Contact), maker, rows (how many
candidate rows cite it).
"""
import collections
import csv
import sys

rows = list(csv.DictReader(open(sys.argv[1]), delimiter='\t'))
seen = collections.OrderedDict()
for r in rows:
    if r['state'] != 'candidate':
        continue
    k = (r['url'], r['src'], r['maker'])
    seen[k] = seen.get(k, 0) + 1
print('url\tsrc\tmaker\trows')
for (url, src, maker), n in seen.items():
    print(f'{url}\t{src}\t{maker}\t{n}')
