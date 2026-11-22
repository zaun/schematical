#!/usr/bin/env python3
"""plan-44-C/D: does every land-pattern line have a footprint?

usage: python3 -I fp-check.py <letter> [--map <map.tsv>]... [--json <footprints-X.json>]... [--needed <footprints-needed.txt>]

Joins the letter's lines of footprints-needed.txt (letter, maker, mounting,
DigiKey Package / Case, supplier package, size, classes, parts, example,
datasheet, existing, label) with the footprint map (columns letter, maker,
package, supplier, size, label, footprint, pinouts, note; one row per land
pattern: `supplier` and `size` are the line's values, or `*` for any; `label` is
the line's last column).
Without --map the maps are footprint-map.tsv and footprint-map-C.tsv /
footprint-map-D.tsv; without --json the known ids are Parts/footprints.json,
footprints-C.json and footprints-D.json (the reader worksheets in fp-parts/
name their own --map / --json; with --map only the lines that have a row in
it are counted).

Every line needs a map row; its footprint must be known; the footprint's pad
count (`pins`) must equal the pin count of each pinout the row serves
(`pinouts`: `;`-separated `1=X1,2=NC,...`), so a part can be placed on it.
Prints each problem and `<n> lines, <m> without a footprint`.
"""
import csv
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
LIBRARY = os.path.join(ROOT, 'schematical', 'schematical', 'Parts', 'footprints.json')


def objects(paths):
    out = {}
    for p in paths:
        if os.path.exists(p):
            for o in json.load(open(p))['footprints']:
                out[o['id']] = o
    return out


def load_map(paths):
    fmap = {}
    for path in paths:
        if os.path.exists(path):
            for r in csv.DictReader(open(path), delimiter='\t'):
                fmap[(r['letter'], r['maker'], r['package'], r['supplier'], r['size'], r['label'])] = r
    return fmap


def lookup(fmap, letter, maker, package, supplier, size, label):
    for s in (supplier, '*'):
        for z in (size, '*'):
            r = fmap.get((letter, maker, package, s, z, label))
            if r:
                return r
    return None


def main(argv):
    letter = argv[0]
    maps, jsons = [], []
    needed = os.path.join(DATA, 'footprints-needed.txt')
    i = 1
    while i < len(argv):
        if argv[i] == '--map':
            maps.append(argv[i + 1])
        elif argv[i] == '--json':
            jsons.append(argv[i + 1])
        elif argv[i] == '--needed':
            needed = argv[i + 1]
        i += 2
    scoped = bool(maps)     # with --map only the lines that have a row in it (a reader's worksheet) are counted
    maps = maps or [os.path.join(DATA, 'footprint-map.tsv')] + [os.path.join(DATA, f'footprint-map-{L}.tsv') for L in 'CD']
    objs = objects([LIBRARY] + (jsons or [os.path.join(DATA, 'footprints-C.json'), os.path.join(DATA, 'footprints-D.json')]))
    fmap = load_map(maps)
    n = missing = 0
    for line in open(needed):
        if line.startswith('#') or not line.strip():
            continue
        f = line.rstrip('\n').split('\t')
        if f[0] != letter:
            continue
        m = lookup(fmap, f[0], f[1], f[3], f[4], f[5], f[11])
        if scoped and not m:
            continue
        n += 1
        bad = []
        if not m or not m['footprint']:
            bad.append('no footprint')
        elif m['footprint'] not in objs:
            bad.append(f"unknown id {m['footprint']}")
        else:
            pins = objs[m['footprint']]['pins']
            for pinout in [p for p in m['pinouts'].split(';') if p]:
                count = len(pinout.split(','))
                if count != pins:
                    bad.append(f"{m['footprint']} has {pins} pads, pinout {pinout} has {count} pins")
        if bad:
            missing += 1
            print('; '.join(bad) + ':', f[1], '|', f[3], '|', f[4], '|', f[5], '|', f[11])
    print(f'{n} lines, {missing} without a footprint')
    return 1 if missing else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
