#!/usr/bin/env python3
"""plan-54-B/C/D: split specs.tsv into per-reader fragments and merge them back.

usage: python3 -I assign.py                       write planning/plan-54-data/specs-parts/<name>.tsv (the rows of each
                                                  reader, columns after the skeleton empty); never overwrites a fragment
       python3 -I assign.py --merge               copy every fragment's lines into specs.tsv (a fragment's lines replace
                                                  all specs.tsv lines of their key); prints the counts
       python3 -I assign.py --list                the fragment names and row counts

A fragment is named `<family>-<maker slug>[-<n>]`: the rows of one (family, maker) in (series, pdf) order, in SPLIT[...]
equal slices.
"""
import collections
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs  # noqa: E402

PARTS = os.path.join(specs.DATA, 'specs-parts')
SLUG = {'Same Sky': 'samesky', 'Amphenol ICC': 'amphenol', 'GCT': 'gct', 'TE Connectivity': 'te', 'Molex': 'molex',
        'Samtec': 'samtec'}
SPLIT = {('card', 'Same Sky'): 2, ('card', 'Amphenol ICC'): 1, ('card', 'GCT'): 1,
         ('module', 'TE Connectivity'): 1, ('module', 'Amphenol ICC'): 1, ('module', 'Molex'): 1,
         ('edge', 'Samtec'): 2, ('edge', 'Amphenol ICC'): 4, ('edge', 'TE Connectivity'): 5}


def names():
    rows = specs.read_rows(specs.SPECS)
    out = collections.OrderedDict()
    for (fam, maker), n in SPLIT.items():
        mine = sorted((r for r in rows if r['family'] == fam and r['maker'] == maker), key=lambda r: (r['series'], r['pdf']))
        size = -(-len(mine) // n) if mine else 0
        for i in range(n):
            name = f'{fam}-{SLUG[maker]}' + (f'-{i + 1}' if n > 1 else '')
            out[name] = mine[i * size:(i + 1) * size]
    return out


def make():
    os.makedirs(PARTS, exist_ok=True)
    for name, rs in names().items():
        path = os.path.join(PARTS, name + '.tsv')
        if os.path.exists(path):
            print('exists', name)
            continue
        specs.write_rows(path, rs, specs.COLS)
        print(name, len(rs), 'rows')


def merge():
    allrows = specs.read_rows(specs.SPECS)
    frag = collections.OrderedDict()
    n = filled = 0
    for name in sorted(os.listdir(PARTS)):
        if not name.endswith('.tsv'):
            continue
        for r in specs.read_rows(os.path.join(PARTS, name)):
            e = specs.explicit_pins(r['pinout'])
            if e is not None:   # the pad labels are the pin numbers 1..N in list order
                r['pinout'] = ','.join(f'{i}={nm}' for i, (_, nm) in enumerate(e, 1))
            k = specs.key(r)
            frag.setdefault(k, []).append(r)
            n += 1
            filled += 1 if r['scheme'] else 0
    known = {specs.key(r) for r in allrows}
    for k in frag:
        if k not in known:
            sys.exit(f'assign.py: a fragment has a row that is not in specs.tsv: {k}')
    out = []
    done = set()
    for r in allrows:
        k = specs.key(r)
        if k in frag:
            if k not in done:
                out.extend(frag[k])
                done.add(k)
        else:
            out.append(r)
    specs.write_rows(specs.SPECS, out, specs.COLS)
    print(f'merged {n} fragment lines ({filled} with a scheme) into {len(out)} specs.tsv lines')


if __name__ == '__main__':
    if '--merge' in sys.argv:
        merge()
    elif '--list' in sys.argv:
        for name, rs in names().items():
            print(name, len(rs))
    else:
        make()
