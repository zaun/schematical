#!/usr/bin/env python3
"""plan-47 census: the distinct maker-site datasheet URLs of the in-scope
rows. usage: urls.py <raw dir> > urls-needed.tsv
Columns: company, url, rows (in-scope rows that cite it), via (link,
sibling or guess, the commonest)."""
import collections
import sys

sys.path.insert(0, __import__('os').path.dirname(__file__))
import common

rows = common.in_scope(common.read_raw(sys.argv[1]))
common.resolve_urls(rows)
n = collections.Counter()
via = collections.defaultdict(collections.Counter)
for r in rows:
    if r['state'] or not r['url']:
        continue
    n[(r['company'], r['url'])] += 1
    via[(r['company'], r['url'])][r['via']] += 1
print('company\turl\trows\tvia')
for (c, u), k in sorted(n.items()):
    print(c, u, k, via[(c, u)].most_common(1)[0][0], sep='\t')
