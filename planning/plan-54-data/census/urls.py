#!/usr/bin/env python3
"""plan-54 census: the distinct (maker URL, download source) pairs of the candidate rows.

usage: python3 -I urls.py candidates.tsv > urls-needed.tsv
Columns: url (what datasheets.json will name), src (what is downloaded), host (of src), maker, parts.
"""
import collections
import csv
import sys
from urllib.parse import urlparse

pairs = collections.OrderedDict()
for r in csv.DictReader(open(sys.argv[1]), delimiter='\t'):
    if r['state'] != 'candidate' or not r['src']:
        continue
    e = pairs.setdefault((r['url'], r['src']), [r['maker'], 0])
    e[1] += 1
print('url\tsrc\thost\tmaker\tparts')
for (u, s), (m, n) in pairs.items():
    print(f'{u}\t{s}\t{urlparse(s).netloc}\t{m}\t{n}')
