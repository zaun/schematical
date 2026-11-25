#!/usr/bin/env python3
"""plan-45-D/E: do the rows of a variant share one housing?

usage: python3 -I fp-groups.py <letter> [--only <label-substring>]

For every `add` row of the letter's variants whose datasheet prints the overall width, group the rows of a
variant by (width - n x pitch, depth): rows with the same pair have the same housing outline (a flange, a
latch or a shroud adds a constant to the width). A variant with more than one group mixes housings that do not
share a footprint. Width and depth are read as fp-widths.py / fp-consistency.py read them:
  Weidmuller   `Width`, `Depth` of the Dimensions table
  Wurth        `Length L` (per-item sheets); the depth is a drawing and is not read
  Phoenix      `Length` is the depth; the width is not printed (only the hole span `Dimension a`): grouped by depth
Prints, per variant with more than one group, the groups with their rows' position counts and items.
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


def num(m):
    return float(m.group(1)) if m else None


def read(maker, t):
    if maker == 'Weidmuller':
        return (num(re.search(r'(?<![A-Za-z(])Width\s+([\d.]+)\s*mm', t)), num(re.search(r'^\s*Depth\s+([\d.]+)\s*mm', t, re.M)))
    if maker == 'Wurth Elektronik':
        m = re.search(r'Length\s+L\s+([\d.]+)\s*mm', t)
        if not m:
            m2 = re.search(r'^\s*\d+\s+\S+\s*±\s*[\d.]+\s+(\S+)\s*±\s*[\d.]+\s+mm\s+\d{12,}', t, re.M)
            if m2:
                parts = m2.group(1).replace(',', '.').split('.')
                return (float(parts[0] + '.' + parts[1]) if len(parts) >= 2 else float(parts[0]), None)
            return (None, None)
        return (float(m.group(1)), None)
    return (None, num(re.search(r'^\s*Length\s+([\d.]+)\s*mm', t, re.M)))


def main():
    letter = sys.argv[1]
    only = sys.argv[sys.argv.index('--only') + 1] if '--only' in sys.argv else ''
    labels = {}
    for line in open(os.path.join(DATA, 'footprints-needed.txt')):
        if line.startswith('#') or not line.strip():
            continue
        f = line.rstrip('\n').split('\t')
        if f[0] == letter:
            labels[f[1]] = f[10]
    groups = collections.defaultdict(lambda: collections.defaultdict(list))
    for r in csv.DictReader(open(os.path.join(DATA, 'candidates.tsv')), delimiter='\t'):
        if r['state'] != 'add' or r['variant'] not in labels or only not in labels[r['variant']]:
            continue
        t = subprocess.run(['pdftotext', '-q', '-layout', os.path.join(ROOT, r['pdf']), '-'], capture_output=True, text=True).stdout
        w, d = read(r['maker'], t)
        n = int(r['positions'])
        extra = None if w is None else round(w - n * float(r['pitch']), 2)
        groups[r['variant']][(extra, d)].append((n, r['mpn']))
    for v, g in groups.items():
        print(f"{labels[v]}: {len(g)} group(s)")
        for (extra, d), rows in sorted(g.items(), key=lambda kv: (kv[0][0] is None, kv[0][0] or 0, kv[0][1] or 0)):
            rows.sort()
            print(f"   extra {extra}, depth {d}: " + ', '.join(f'{n}:{m}' for n, m in rows[:14]) + ('…' if len(rows) > 14 else ''))


main()
