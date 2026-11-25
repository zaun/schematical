#!/usr/bin/env python3
"""plan-47-D section 3.3: does every land-pattern line have a footprint that exists?

usage: python3 -I fp-check.py <letter>... [--lines <footprints-needed.txt>]

Joins the letters' lines of footprints-needed.txt (columns letter, type, company, series,
mounting, package, supplier_package, size, detail, parts, example, datasheets, existing; one
line per distinct package, footprint-lines.py) with footprint-map.tsv and the per-letter
footprint-map-<name>.tsv files beside it (columns: the first nine of the line, then
footprint, label, note, and optionally series_split). A map row with an empty `series_split` serves the whole line; map rows
with a `series_split` serve those series of the line (a line whose datasheets disagree is split
that way): every series of the line's rows in candidates.tsv (state `add`, else `candidate`
and `reserve`) must be served. Every mapped footprint must be in Parts/footprints.json or in
one of the footprints-*.json files beside the map. Prints each line that is not served and
`<n> lines, <m> without a footprint`.
"""
import csv
import glob
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
LETTER = {'inductor': 'D', 'fuse': 'E', 'ptc': 'F', 'relay': 'G', 'buzzer': 'H'}
KEY = ['letter', 'type', 'company', 'series', 'mounting', 'package', 'supplier_package', 'size', 'detail']


def known_ids():
    text = open(os.path.join(ROOT, 'schematical', 'schematical', 'Parts', 'footprints.json'), encoding='utf-8').read()
    ids = set(re.findall(r'"id":"([^"]+)"', text))
    for p in glob.glob(os.path.join(DATA, 'footprints-*.json')):
        ids |= {o['id'] for o in json.load(open(p, encoding='utf-8'))['footprints']}
    return ids


def load_map():
    rows = []
    for p in [os.path.join(DATA, 'footprint-map.tsv')] + sorted(glob.glob(os.path.join(DATA, 'footprint-map-*.tsv'))):
        if os.path.exists(p):
            rows += [r for r in csv.DictReader(open(p, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE)
                     if r['footprint']]      # a skeleton row with no footprint serves nothing
    return rows


def line_series():
    """{line key (letter .. detail, series blanked except for relays): set of series}."""
    path = os.path.join(DATA, 'candidates-provisional.tsv')   # until plan-47-A is done and the pick is final
    if not os.path.exists(path):
        path = os.path.join(DATA, 'candidates.tsv')
    rows = list(csv.DictReader(open(path, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE))
    states = ('add',) if any(r['state'] == 'add' for r in rows) else ('candidate', 'reserve')
    out = {}
    for r in rows:
        if r['state'] not in states:
            continue
        relay = r['type'] == 'relay'
        det = (r['form'] + '; ' + r['detail']) if relay else (r['detail'] if r['type'] in ('ptc', 'buzzer') else '')
        k = (LETTER[r['type']], r['type'], r['company'], r['series'] if relay else '', r['mounting'], r['package'],
             r['supplier_package'], r['size'], det)
        out.setdefault(k, set()).add(r['series'])
    return out


def main(argv):
    lines_path = os.path.join(DATA, 'footprints-needed.txt')
    if '--lines' in argv:
        i = argv.index('--lines')
        lines_path = argv[i + 1]
        argv = argv[:i] + argv[i + 2:]
    letters = set(argv)
    ids = known_ids()
    fmap = {}
    for r in load_map():
        fmap.setdefault(tuple(r[k] for k in KEY), {})[r.get('series_split', '')] = r
    series_of = line_series()
    n = missing = 0
    for line in open(lines_path, encoding='utf-8'):
        if line.startswith('#') or not line.strip():
            continue
        f = line.rstrip('\n').split('\t')
        if f[0] not in letters:
            continue
        n += 1
        key = tuple(f[:9])
        rows = fmap.get(key, {})
        bad = None
        if '' in rows:
            r = rows['']
            if not r['footprint']:
                bad = 'no footprint'
            elif r['footprint'] not in ids:
                bad = f"unknown id {r['footprint']}"
        else:
            want = series_of.get(key, {''})
            for s in want:
                r = rows.get(s)
                if not r or not r['footprint']:
                    bad = f"no footprint for series {s!r}"
                    break
                if r['footprint'] not in ids:
                    bad = f"unknown id {r['footprint']}"
                    break
        if bad:
            missing += 1
            print(bad + ':', '|'.join(key[1:3]), '|', f[4], '|', f[5], '|', f[6], '|', f[7], '|', f[8])
    print(f"{n} lines, {missing} without a footprint")
    return 1 if missing else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
