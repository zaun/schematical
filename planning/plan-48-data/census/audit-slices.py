#!/usr/bin/env python3
"""plan-48-B: a blind re-read sample of the filled spec rows.
usage: python3 -I audit-slices.py <specs.tsv> <out dir> <n rows> <rows per slice> [seed]
Writes `audit-NN.tsv` slices (specs.tsv columns plus `note`) with the fill
columns emptied, for a second reader; `answers.tsv` keeps the first reading."""
import csv
import os
import random
import sys

src, out, n, per = sys.argv[1], sys.argv[2], int(sys.argv[3]), int(sys.argv[4])
seed = sys.argv[5] if len(sys.argv) > 5 else '48'
os.makedirs(out, exist_ok=True)
rows = list(csv.DictReader(open(src, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE))
cols = list(rows[0].keys()) + ['note']
rnd = random.Random(seed)
pick = rnd.sample(range(len(rows)), n)
pick.sort(key=lambda i: (rows[i]['company'], rows[i]['pdf']))
with open(os.path.join(out, 'answers.tsv'), 'w', encoding='utf-8', newline='') as f:
    f.write('\t'.join(cols) + '\n')
    for i in pick:
        f.write('\t'.join(dict(rows[i], note='')[c] for c in cols) + '\n')
for k in range(0, n, per):
    path = os.path.join(out, 'audit-%02d.tsv' % (k // per + 1))
    with open(path, 'w', encoding='utf-8', newline='') as f:
        f.write('\t'.join(cols) + '\n')
        for i in pick[k:k + per]:
            r = dict(rows[i], scheme='', label='', pinout='', facts='', note='')
            f.write('\t'.join(r[c] for c in cols) + '\n')
    print(path)
