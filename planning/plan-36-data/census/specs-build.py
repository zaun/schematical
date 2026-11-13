#!/usr/bin/env python3
"""plan-36-A Phase 3: specs.tsv = specs-draft.py's rows (read against each figure) + one row per ESP32 / RP2040 PDF + specs-hand.tsv.

usage: python3 -I specs-build.py   (reads /tmp/p36/specs-draft.tsv, /tmp/p36/pdf/index.tsv, /tmp/p36/cand.tsv)
"""
import csv, sys
import os
root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..') + '/'
idx = {r['md5']: r['url'] for r in csv.DictReader(open('/tmp/p36/pdf/index.tsv'), delimiter='\t')}
def name(h): return idx.get(h, '').rstrip('/').split('/')[-1]
rows = []
for r in csv.DictReader(open('/tmp/p36/specs-draft.tsv'), delimiter='\t'):
    rows.append((r['pdf'], r['mpn_regex'], r['package_field'], f"{name(r['pdf'])}: {r['note']}"))
cand = list(csv.DictReader(open('/tmp/p36/cand.tsv'), delimiter='\t'))
seen = set()
for r in cand:
    if r['scope'] == 'in' and r['family'] == 'ESP32' and r['pdf'] and r['pdf'] not in seen:
        seen.add(r['pdf'])
        rows.append((r['pdf'], '-', 'device', f"{name(r['pdf'])}: module datasheet; the package is the module itself (B § 3.1: code = the module device), verbatim hits only"))
    if r['scope'] == 'in' and r['family'] == 'RP2040' and r['pdf'] and r['pdf'] not in seen:
        seen.add(r['pdf'])
        rows.append((r['pdf'], '-', '=QFN-56', f"{name(r['pdf'])}: one package, \"RP2040 7×7 mm QFN-56\" (§ 5.1, text line 45858); SC0914(7)/(13) are its reels (text lines 47731, 47734)"))

try:
    for r in csv.DictReader(open(root + 'specs-hand.tsv'), delimiter='\t'):
        rows.append((r['pdf'], r['mpn_regex'], r['package_field'], f"{name(r['pdf'])}: {r['note']}"))
except FileNotFoundError:
    pass
with open(root + 'specs.tsv', 'w') as out:
    out.write('pdf\tmpn_regex\tpackage_field\tnote\n')
    for x in rows:
        out.write('\t'.join(x) + '\n')
print(len(rows), 'specs rows')
