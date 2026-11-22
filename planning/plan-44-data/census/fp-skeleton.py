#!/usr/bin/env python3
"""plan-44-C/D: the footprint map skeleton, split into per-reader worksheets.

usage: python3 -I fp-skeleton.py              write planning/plan-44-data/fp-parts/<name>.tsv and <name>.json
                                              (never overwrites a worksheet that exists)
       python3 -I fp-skeleton.py --assemble   merge the worksheets: footprints-C.json, footprints-D.json,
                                              footprint-map.tsv (the C rows then the D rows)

Worksheet columns: the map's (letter, maker, package, supplier, size, label,
footprint, pinouts, note) and, for the reader, parts, example, datasheet and
the local PDFs of the line's `add` rows, and the existing library footprints
with the same body size code. `label` is the line's `footprints-needed.txt`
label (the footprint id without its maker suffix); `pinouts` are the distinct
pinouts of the line's `add` parts (`;`-separated `1=X1,2=NC,...`); `footprint`
and `note` are for the reader to fill. A reader's objects go into its
<name>.json (`{"format": "schematical-footprints", "version": 1, "footprints": [...]}`).
"""
import collections
import csv
import json
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402
import specs  # noqa: E402

PARTS = os.path.join(common.DATA, 'fp-parts')
MAPCOLS = ['letter', 'maker', 'package', 'supplier', 'size', 'label', 'footprint', 'pinouts', 'note']
HELP = ['parts', 'example', 'datasheet', 'pdfs', 'existing']
EMPTY = {'format': 'schematical-footprints', 'version': 1, 'footprints': []}
# reader -> (letter, package prefixes); the 4-SMD lines are split by label below
READERS = [('c1', 'C', ('2-SMD', '3-SMD')),
           ('c2', 'C', ('4-SMD, No Lead',)),
           ('c3', 'C', ('4-SMD, No Lead', '4-SMD, Flat')),
           ('c4', 'C', ('HC-49/US', '4-SOJ', 'Cylindrical')),
           ('d1', 'D', ('Cylindrical', 'HC-49/U', 'HC-46/X')),
           ('d2', 'D', ('3-SMD', '1512', '2-SMD'))]


def lines():
    needed = [l.rstrip('\n').split('\t') for l in open(os.path.join(common.DATA, 'footprints-needed.txt')) if not l.startswith('#')]
    add = collections.defaultdict(list)
    for r in csv.DictReader(open(specs.CANDIDATES), delimiter='\t'):
        if r['state'] == 'add':
            add[(r['maker'], r['package'], r['supplier_package'], r['size'])].append(r)
    table = specs.load()
    out = []
    for f in needed:
        letter, maker, mounting, package, supplier, size = f[:6]
        label = f[11]
        pads = int(label.rsplit('-', 1)[1])
        rows = [r for r in add[(maker, package, supplier, size)]
                if len(specs.pinout_items(table[specs.key(r)]['pinout'])) == pads]
        pinouts = sorted({table[specs.key(r)]['pinout'] for r in rows})
        pdfs = sorted({r['pdf'] for r in rows})
        out.append({'letter': letter, 'maker': maker, 'package': package, 'supplier': supplier, 'size': size,
                    'label': label, 'footprint': '', 'pinouts': ';'.join(pinouts), 'note': '',
                    'parts': f[7], 'example': f[8], 'datasheet': f[9], 'pdfs': ' '.join(pdfs), 'existing': f[10]})
    return out


def split(all_lines):
    mine = {}
    taken = set()
    four = sorted({l['label'] for l in all_lines if l['letter'] == 'C' and l['package'] == '4-SMD, No Lead'})
    half = set(four[:len(four) // 2])
    for name, letter, prefixes in READERS:
        sel = []
        for i, l in enumerate(all_lines):
            if i in taken or l['letter'] != letter or not l['package'].startswith(prefixes):
                continue
            if name == 'c2' and l['label'] not in half:
                continue
            if name == 'c3' and l['package'] == '4-SMD, No Lead' and l['label'] in half:
                continue
            taken.add(i)
            sel.append(l)
        mine[name] = sel
    left = [l for i, l in enumerate(all_lines) if i not in taken]
    if left:
        sys.exit(f'fp-skeleton.py: {len(left)} lines assigned to no reader, e.g. {left[0]["package"]}')
    return mine


def write(path, cols, rows):
    with open(path, 'w', newline='') as f:
        w = csv.writer(f, delimiter='\t', lineterminator='\n')
        w.writerow(cols)
        for r in rows:
            w.writerow([r[c] for c in cols])


def make():
    os.makedirs(PARTS, exist_ok=True)
    for name, rows in split(lines()).items():
        path = os.path.join(PARTS, name + '.tsv')
        if os.path.exists(path):
            print('exists', name)
            continue
        write(path, MAPCOLS + HELP, sorted(rows, key=lambda r: (r['label'], r['maker'], r['supplier'], r['size'])))
        with open(os.path.join(PARTS, name + '.json'), 'w') as f:
            json.dump(EMPTY, f, indent=2)
            f.write('\n')
        print(name, len(rows), 'lines,', len({r['label'] for r in rows}), 'labels')


def assemble():
    every = []
    for letter in 'CD':
        objs, rows = [], []
        for name, l, _ in READERS:
            if l != letter:
                continue
            objs += json.load(open(os.path.join(PARTS, name + '.json')))['footprints']
            rows += list(csv.DictReader(open(os.path.join(PARTS, name + '.tsv')), delimiter='\t'))
        ids = [o['id'] for o in objs]
        dup = sorted({i for i in ids if ids.count(i) > 1})
        if dup:
            sys.exit('duplicate ids: ' + ', '.join(dup))
        with open(os.path.join(common.DATA, f'footprints-{letter}.json'), 'w') as f:
            f.write('{\n  "format": "schematical-footprints",\n  "version": 1,\n  "footprints": [\n')
            f.write(',\n'.join('    ' + json.dumps(o, ensure_ascii=False, separators=(',', ':')) for o in objs))
            f.write('\n  ]\n}\n')
        every += rows
        print(f'letter {letter}: {len(objs)} footprints, {len(rows)} map rows')
    write(os.path.join(common.DATA, 'footprint-map.tsv'), MAPCOLS, every)


if __name__ == '__main__':
    assemble() if '--assemble' in sys.argv else make()
