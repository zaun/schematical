#!/usr/bin/env python3
"""plan-47-B: overlay filled fragments onto specs.tsv.

usage: python3 -I specs-merge.py <fragment.tsv>...

Each fragment row replaces the specs.tsv row with the same key (type, maker,
series, pdf, package) in the filled columns (scheme, label, page, pinout,
terminals, pads and the fact columns); `rows` and `example` stay from specs.tsv.
A fragment row with no specs.tsv row is an error. Prints the counts.
"""
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs47 as sp  # noqa: E402

FILLED = ['scheme', 'label', 'page', 'pinout', 'terminals'] + sp.FACT_COLS


def main(paths):
    rows = sp.read(sp.SPECS)
    cols = list(rows[0].keys())
    index = {(s['type'], s['maker'], s['series'], s['pdf'], s['package']): s for s in rows}
    n = 0
    for path in paths:
        for f in sp.read(path):
            k = (f['type'], f['maker'], f['series'], f['pdf'], f['package'])
            if k not in index:
                print('no specs row for', '|'.join(k))
                return 1
            for c in FILLED:
                index[k][c] = f[c]
            n += 1
    sp.write(sp.SPECS, rows, cols)
    print(f'{n} rows merged; {sum(1 for s in rows if s["scheme"])} of {len(rows)} specs rows have a scheme')
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
