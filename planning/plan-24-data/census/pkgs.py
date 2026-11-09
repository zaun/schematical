#!/usr/bin/env python3
"""Packages the matched rows need: TI type, code, pins -> row count, and the footprint id
footprint-ids.tsv gives it (or '?').

usage: pkgs.py rows-*.tsv
"""
import collections, csv, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
IDS = os.path.join(os.path.dirname(HERE), 'footprint-ids.tsv')
ids = {}
if os.path.exists(IDS):
    for l in open(IDS):
        if l.startswith('#') or not l.strip():
            continue
        p = l.rstrip('\n').split('\t')
        ids[p[0]] = p[1]
c = collections.Counter()
for f in sys.argv[1:]:
    for r in csv.DictReader(open(f), delimiter='\t'):
        if r['match'] in ('own', 'other'):
            c[f"{r['code']}-{r['pins']}\t{r['pkgtype']}"] += 1
for k, n in sorted(c.items(), key=lambda x: -x[1]):
    key = k.split('\t')[0]
    print(f'{n}\t{k}\t{ids.get(key, "?")}')
print(len(c), 'packages;', sum(1 for k in c if k.split("\t")[0] not in ids), 'without an id', file=sys.stderr)
