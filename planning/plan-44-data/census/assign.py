#!/usr/bin/env python3
"""plan-44-B: split specs.tsv into per-reader fragments.

usage: python3 -I assign.py                   write planning/plan-44-data/specs-parts/<name>.tsv (the skeleton rows
                                              of each reader, scheme columns empty); never overwrites a fragment
       python3 -I assign.py --merge           copy every fragment's filled rows into specs.tsv (a fragment row
                                              overwrites the specs.tsv row of the same key); prints the counts

Readers: abracon-1, abracon-2 (the Abracon rows in (series, pdf, package) order, halves),
ecs-1, ecs-2, ecs-3 (the ECS rows, thirds), suntsu.
"""
import csv
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs  # noqa: E402

PARTS = os.path.join(os.path.dirname(specs.SPECS), 'specs-parts')
SPLIT = {'Abracon': 2, 'ECS': 3, 'Suntsu': 1}


def rows():
    return list(csv.DictReader(open(specs.SPECS), delimiter='\t'))


def write(path, rs):
    with open(path, 'w', newline='') as f:
        w = csv.writer(f, delimiter='\t', lineterminator='\n')
        w.writerow(specs.COLS)
        for r in rs:
            w.writerow([r[c] for c in specs.COLS])


def make():
    os.makedirs(PARTS, exist_ok=True)
    allrows = rows()
    for maker, n in SPLIT.items():
        mine = sorted((r for r in allrows if r['maker'] == maker), key=lambda r: (r['series'], r['pdf'], r['package']))
        size = -(-len(mine) // n)
        for i in range(n):
            name = f"{maker.lower()}-{i + 1}" if n > 1 else maker.lower()
            path = os.path.join(PARTS, name + '.tsv')
            if os.path.exists(path):
                print('exists', name)
                continue
            write(path, mine[i * size:(i + 1) * size])
            print(name, len(mine[i * size:(i + 1) * size]), 'rows')


def merge():
    allrows = rows()
    by = {specs.key(r): r for r in allrows}
    n = filled = 0
    for name in sorted(os.listdir(PARTS)):
        if not name.endswith('.tsv'):
            continue
        for r in csv.DictReader(open(os.path.join(PARTS, name)), delimiter='\t'):
            k = specs.key(r)
            if k not in by:
                sys.exit(f'assign.py: {name} has a row that is not in specs.tsv: {k}')
            n += 1
            for c in specs.COLS[6:]:
                by[k][c] = r[c]
            filled += 1 if r['scheme'] else 0
    write(specs.SPECS, allrows)
    print(f'merged {n} fragment rows, {filled} with a scheme')


if __name__ == '__main__':
    merge() if '--merge' in sys.argv else make()
