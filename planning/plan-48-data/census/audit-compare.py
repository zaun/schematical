#!/usr/bin/env python3
"""plan-48-B: compare a blind re-read with the first reading.
usage: python3 -I audit-compare.py <answers.tsv> <audit-NN.tsv>...
A named scheme is expanded to its pin list (pin-checker's table) so
`dual-8` and an `explicit` pinout with the same pins compare equal. Prints
the rows whose pin list differs, then the rows whose label, process,
maxHz, vcc or inputMax differ, then the counts."""
import csv
import importlib.util
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location('pc', os.path.join(HERE, 'pinout-check.py'))
pc = importlib.util.module_from_spec(spec)
spec.loader.exec_module(pc)
KEY = ['pdf', 'package', 'supplier_package', 'circuits']


def pins(r):
    if r['scheme'] in pc.SCHEMES:
        return list(pc.SCHEMES[r['scheme']])
    if r['pinout'] in ('', '-'):
        return ['-']
    return [n for _, n in pc.parse_pinout(r['pinout'])]


def read(p):
    return {tuple(r[c] for c in KEY): r for r in csv.DictReader(open(p, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE)}


first = read(sys.argv[1])
rows = {}
for p in sys.argv[2:]:
    rows.update(read(p))
pin_diff = fact_diff = 0
for k, b in rows.items():
    a = first[k]
    if pins(a) != pins(b):
        pin_diff += 1
        print('PINS  ', a['company'], k[0].split('/')[-1], k[1], a['example'])
        print('   first:', ','.join(pins(a)))
        print('   blind:', ','.join(pins(b)))
    fa, fb = pc.facts_of(a['facts']), pc.facts_of(b['facts'])
    d = [(x, fa.get(x), fb.get(x)) for x in sorted(set(fa) | set(fb)) if fa.get(x) != fb.get(x)]
    if a['label'] != b['label']:
        d.append(('label', a['label'], b['label']))
    if d:
        fact_diff += 1
        print('FACTS ', a['company'], k[0].split('/')[-1], k[1], a['example'], d)
print('%d rows compared, %d pin differences, %d fact/label differences' % (len(rows), pin_diff, fact_diff))
