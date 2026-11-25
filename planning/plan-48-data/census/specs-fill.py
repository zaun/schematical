#!/usr/bin/env python3
"""plan-48-B: merge hand-read fragment files into specs.tsv.
usage: python3 -I specs-fill.py <fragment.tsv>... [--specs <specs.tsv>]

A fragment has the columns `pdf`, `package`, `supplier_package`, `circuits`,
`scheme`, `label`, `pinout`, `facts` and (ignored) `note`: the evidence line.
Each row fills `scheme`, `label`, `pinout`, `facts` of the one specs.tsv row
with the same pdf, package, supplier_package and circuits. A fragment row
that fills no row is an error, as is a specs row filled twice with different
values. Prints `<n> filled, <m> errors`."""
import csv
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
KEY = ['pdf', 'package', 'supplier_package', 'circuits']
FILL = ['scheme', 'label', 'pinout', 'facts']


def main(argv):
    specs = os.path.join(HERE, '..', 'specs.tsv')
    if '--specs' in argv:
        i = argv.index('--specs')
        specs = argv[i + 1]
        argv = argv[:i] + argv[i + 2:]
    rows = list(csv.DictReader(open(specs, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE))
    cols = list(rows[0].keys())
    index = {tuple(r[c] for c in KEY): r for r in rows}
    done, bad = {}, 0
    for path in argv:
        for f in csv.DictReader(open(path, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE):
            k = tuple(f[c] for c in KEY)
            if k not in index:
                print('fills nothing:', path, '|'.join(k))
                bad += 1
                continue
            new = [f[c].strip() for c in FILL]
            if k in done and done[k] != new:
                print('filled twice:', '|'.join(k))
                bad += 1
                continue
            done[k] = new
            for c, v in zip(FILL, new):
                index[k][c] = v
    with open(specs, 'w', encoding='utf-8', newline='') as out:
        out.write('\t'.join(cols) + '\n')
        for r in rows:
            out.write('\t'.join(r[c] for c in cols) + '\n')
    print('%d filled, %d errors' % (len(done), bad))
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
