#!/usr/bin/env python3
"""plan-47-G section 3.2: do the footprints' pad designators agree with the pinouts?

usage: python3 -I fp-pins.py G [--apply]

Reads pins-G-*.tsv (columns type maker series pdf package footprint renumber pinout: one row
per specs.tsv relay row, the footprint that serves it, the old=new designator pairs of the
renumbering to 1..n, and the pinout rewritten with the new designators) and checks, per row:
the pinout's designators are exactly 1..<pads> of the footprint (an `explicit` footprint's
numbered pads, a `dual` footprint's 1..pins), and the specs.tsv row exists. --apply writes
`renumber` and the new `pinout` into specs.tsv (a `renumber` column is added). Prints
`<m> rows: <e> errors`.
"""
import glob
import json
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs47 as sp  # noqa: E402


def pad_sets():
    """{footprint id: set of pad designators} of the plan-47 footprint files and the library."""
    out = {}
    paths = glob.glob(os.path.join(sp.DATA, 'footprints-*.json')) + [
        os.path.join(sp.ROOT if hasattr(sp, 'ROOT') else os.path.dirname(os.path.dirname(sp.DATA)),
                     'schematical', 'schematical', 'Parts', 'footprints.json')]
    for p in paths:
        for o in json.load(open(p, encoding='utf-8'))['footprints']:
            if o['style'] == 'explicit':
                out[o['id']] = {str(pad['pin']) for pad in o['pads'] if 'pin' in pad and pad['pin']}
            elif o['style'] == 'dual':
                out[o['id']] = {str(i) for i in range(1, o['pins'] + 1)}
    return out


def main(argv):
    apply = '--apply' in argv
    rows = []
    for p in sorted(glob.glob(os.path.join(sp.DATA, 'pins-G-*.tsv'))):
        rows += sp.read(p)
    pads = pad_sets()
    specs = sp.read(sp.SPECS)
    cols = list(specs[0].keys())
    if 'renumber' not in cols:
        cols.insert(cols.index('pinout') + 1, 'renumber')
    index = {(s['type'], s['maker'], s['series'], s['pdf'], s['package']): s for s in specs}
    errors = 0
    for r in rows:
        key = (r['type'], r['maker'], r['series'], r['pdf'], r['package'])
        s = index.get(key)
        if s is None:
            print('no specs row:', '|'.join(key))
            errors += 1
            continue
        want = {d for d, _ in (x.split('=', 1) for x in r['pinout'].split(','))}
        have = pads.get(r['footprint'])
        if have is None:
            print('unknown footprint', r['footprint'], 'for', '|'.join(key))
            errors += 1
        elif want != have:
            print(f"{r['footprint']}: pinout designators {sorted(want, key=lambda x: (len(x), x))} but pads "
                  f"{sorted(have, key=lambda x: (len(x), x))} for {'|'.join(key[2:4])}")
            errors += 1
        if apply:
            s['pinout'] = r['pinout']
            s['renumber'] = r['renumber']
    if apply and not errors:
        sp.write(sp.SPECS, specs, cols)
    print(f'{len(rows)} rows: {errors} errors')
    return 1 if errors else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
