#!/usr/bin/env python3
"""Pick one name per distinct datasheet (same bytes = same document).

usage: canon.py <pdfdir> <pdfkeys.tsv> > canon.tsv
canon.tsv: <file name>  <canonical file name>
The canonical name of a group is the one the most in-scope rows cite
(pdfkeys.tsv: key  parts...), ties broken by the higher name (ads1115 over ads1113).
"""
import collections, hashlib, os, sys

d, keys = sys.argv[1], sys.argv[2]
cites = collections.Counter()
for l in open(keys):
    k, parts = l.rstrip('\n').split('\t')
    cites[k + '.pdf'] = len(parts.split())
groups = collections.defaultdict(list)
for f in sorted(os.listdir(d)):
    if f.endswith('.pdf'):
        h = hashlib.md5(open(os.path.join(d, f), 'rb').read()).hexdigest()
        groups[h].append(f)
for h, fs in groups.items():
    c = max(fs, key=lambda f: (cites[f], f))
    for f in fs:
        print(f + '\t' + c)
