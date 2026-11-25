#!/usr/bin/env python3
"""plan-48-C/D: the work list of the footprint letters.
usage: python3 -I fp-work.py <letter> [<n per slice> <out dir>]
Prints, per line of footprints-needed.txt (the final pick) of the letter:
company, DigiKey package, supplier package, pins, the existing candidate
footprint ids, and the spec rows (PDF, circuits, example part, scheme, ball
pinout when explicit) that use it. With <n per slice> and <out dir> it
writes slice-<letter>-NN.tsv files (one line of work per row) instead."""
import csv
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
letter = sys.argv[1]
specs = list(csv.DictReader(open(os.path.join(DATA, 'specs.tsv'), encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE))
out = []
for line in open(os.path.join(DATA, 'footprints-needed.txt'), encoding='utf-8'):
    if line.startswith('#') or not line.strip():
        continue
    f = line.rstrip('\n').split('\t')
    if f[0] != letter:
        continue
    items = []
    for s in specs:
        if s['company'] == f[1] and s['package'] == f[3] and s['supplier_package'] == f[4]:
            pin = s['pinout'] if s['scheme'] == 'explicit' else s['scheme']
            items.append('%s [%s circuits, %s, pins: %s]' % (s['pdf'], s['circuits'], s['example'], pin))
    out.append([f[1], f[3], f[4], f[5], f[11], ' ;; '.join(items)])
if len(sys.argv) > 3:
    n, d = int(sys.argv[2]), sys.argv[3]
    os.makedirs(d, exist_ok=True)
    for i in range(0, len(out), n):
        path = os.path.join(d, 'fp-%s-%02d.tsv' % (letter, i // n + 1))
        with open(path, 'w', encoding='utf-8') as fh:
            fh.write('company\tpackage\tsupplier_package\tpins\texisting_candidates\tspec_rows\n')
            for r in out[i:i + n]:
                fh.write('\t'.join(r) + '\n')
        print(path, len(out[i:i + n]))
else:
    for r in out:
        print('\t'.join(r))
