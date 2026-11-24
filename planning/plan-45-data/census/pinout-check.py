#!/usr/bin/env python3
"""plan-45-B Phase 3: the pin rule of § 3 holds for every row that is in play.

usage: python3 -I pinout-check.py <candidates.tsv> <specs.tsv> [--fill]

§ 3 rule 4: a part has `positions` pins, numbered 1..n, named "1".."n", role `inout` (a terminal block or a
header pad has no function the library models; the pin-1 end is each footprint's, C, D and E read the drawing).
For every `candidate` / `add` row: `positions` is an integer 2..12, its spec row (maker, variant, PDF)
exists, and that spec row's `pinout` is `1..n` (--fill writes it). Prints
`pinout: <rows> rows, <bad> bad`; exit 1 when one is bad.
"""
import csv
import sys

RULE = '1..n'


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    cand, specs_path = args[0], args[1]
    fill = '--fill' in sys.argv
    specs = list(csv.DictReader(open(specs_path), delimiter='\t'))
    cols = list(specs[0].keys())
    by = {(s['maker'], s['variant'], s['pdf']): s for s in specs}
    rows = bad = 0
    used = set()
    for r in csv.DictReader(open(cand), delimiter='\t'):
        if r['state'] not in ('candidate', 'add'):
            continue
        rows += 1
        key = (r['maker'], r['variant'], r['pdf'])
        used.add(key)
        problem = ''
        if not (r['positions'].isdigit() and 2 <= int(r['positions']) <= 12):
            problem = f"positions {r['positions']!r}"
        elif key not in by:
            problem = 'no spec row'
        elif fill:
            by[key]['pinout'] = RULE
        elif by[key]['pinout'] != RULE:
            problem = f"spec pinout {by[key]['pinout']!r}"
        if problem:
            bad += 1
            print('bad', r['mpn'], problem)
    if fill:
        for k, s in by.items():
            if k not in used:
                s['pinout'] = ''             # a spec row no row in play uses (its rows are skip / out)
        with open(specs_path, 'w') as out:
            out.write('\t'.join(cols) + '\n')
            for s in specs:
                out.write('\t'.join(s[c] for c in cols) + '\n')
    print(f'pinout: {rows} rows, {bad} bad')
    sys.exit(1 if bad else 0)


main()
