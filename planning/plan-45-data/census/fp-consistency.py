#!/usr/bin/env python3
"""plan-45-C/D/E: does every row of a variant agree with the numbers its footprint is drawn from?

usage: python3 -I fp-consistency.py <letter>

A variant is one footprint family (A's key: maker, series, pitch, orientation, mounting, clamp class); a maker can draw
different housings under one series (Weidmuller `LM` and `LM1N`), and then the rows do not share a land pattern.
For the rows of the letter's variants this prints, per variant, the distinct values of the facts the sheets print:
  Phoenix      Length (depth), Hole diameter, Pin spacing, Pitch
  Weidmuller   Depth, Solder eyelet hole diameter (D), Number of solder pins per pole, Pitch, Conductor outlet direction
  Wurth        (per-item sheets) the hole diameter of the Recommended Hole Pattern is a drawing: not read here
A variant with more than one value of a fact is listed with the items for each value.
"""
import collections
import csv
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))

FACTS = {
    'Phoenix Contact': [('Length', r'^\s*Length\s+([\d.,]+)\s*mm'), ('Hole diameter', r'^\s*Hole diameter\s+([\d.,]+)\s*mm'),
                        ('Pin spacing', r'^\s*Pin spacing\s+([\d.,]+)\s*mm'), ('Pitch', r'^\s*Pitch\s+([\d.,]+)\s*mm')],
    'Weidmuller': [('Depth', r'^\s*Depth\s+([\d.,]+)\s*mm'), ('Eyelet D', r'Solder eyelet hole diameter \(D\)\s+([\d.,]+)\s*mm'),
                   ('Pins per pole', r'Number of solder pins per pole\s+(\d+)'), ('Pitch', r'Pitch in mm \(P\)\s+([\d.,]+)\s*mm'),
                   ('Outlet', r'Conductor outlet direction\s+(\d+)')],
}


def main():
    letter = sys.argv[1]
    labels = {}
    for line in open(os.path.join(DATA, 'footprints-needed.txt')):
        if line.startswith('#') or not line.strip():
            continue
        f = line.rstrip('\n').split('\t')
        if f[0] == letter:
            labels[f[1]] = f[10]
    seen = collections.defaultdict(lambda: collections.defaultdict(lambda: collections.defaultdict(list)))
    for r in csv.DictReader(open(os.path.join(DATA, 'candidates.tsv')), delimiter='\t'):
        if r['state'] != 'add' or r['variant'] not in labels or r['maker'] not in FACTS:
            continue
        t = subprocess.run(['pdftotext', '-q', '-layout', os.path.join(ROOT, r['pdf']), '-'], capture_output=True, text=True).stdout
        for name, rx in FACTS[r['maker']]:
            m = re.search(rx, t, re.M)
            seen[r['variant']][name][m.group(1) if m else '(none)'].append(r['mpn'])
    odd = 0
    for v in labels:
        if v not in seen:
            continue
        for name, vals in seen[v].items():
            if len(vals) > 1:
                odd += 1
                print(f'{v} | {name}: ' + '; '.join(f"{k} ({len(m)}: {','.join(m[:4])}{'…' if len(m) > 4 else ''})" for k, m in sorted(vals.items())))
    print(f'{odd} facts with more than one value within a variant')


main()
