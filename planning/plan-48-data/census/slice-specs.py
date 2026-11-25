#!/usr/bin/env python3
"""plan-48-B: cut the unfilled rows of specs.tsv into reading slices.
usage: python3 -I slice-specs.py <specs.tsv> <out dir> <type> <rows per slice>
Each slice is a TSV with specs.tsv's columns plus `note`, holding whole PDFs
(all rows of one PDF in one slice), ordered by (company, pdf). Only rows with
no `scheme` yet are cut. Prints each slice's name, rows and PDFs."""
import collections
import csv
import os
import sys

src, out, typ, size = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4])
os.makedirs(out, exist_ok=True)
rows = list(csv.DictReader(open(src, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE))
cols = list(rows[0].keys()) + ['note']
by_pdf = collections.OrderedDict()
for r in sorted(rows, key=lambda r: (r['company'], r['pdf'], r['package'], r['circuits'])):
    if r['type'] == typ and not r['scheme']:
        by_pdf.setdefault(r['pdf'], []).append(r)
slices, cur = [], []
for pdf, rs in by_pdf.items():
    if cur and len(cur) + len(rs) > size:
        slices.append(cur)
        cur = []
    cur += rs
if cur:
    slices.append(cur)
for i, sl in enumerate(slices, 1):
    path = os.path.join(out, '%s-%02d.tsv' % (typ, i))
    with open(path, 'w', encoding='utf-8', newline='') as f:
        f.write('\t'.join(cols) + '\n')
        for r in sl:
            r = dict(r, note='')
            f.write('\t'.join(r[c] for c in cols) + '\n')
    print(path, len(sl), 'rows', len({r['pdf'] for r in sl}), 'PDFs', sl[0]['company'])
