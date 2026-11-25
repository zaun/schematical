#!/usr/bin/env python3
"""plan-45-C/D/E: does every line of footprints-needed.txt have its footprints?

usage: python3 -I fp-check.py <letter> [--json <footprints-X.json>] [--library]

Reads the lines of the letter (C fixed terminal blocks, D pluggable headers, E JST) of footprints-needed.txt
(letter, variant, maker, series, pitch, orientation, mounting, positions, parts, existing, label, generic) and
looks for one footprint per (line, position count) in <footprints-<letter>.json> (default
planning/plan-45-data/footprints-<letter>.json; --library also accepts the ids already in
schematical/schematical/Parts/footprints.json, for G's re-runs). The id is `TB-<label>-<NN>` for C and D and
`<label>-<NN>` for E (its labels start with `JST-`), NN the position count in two digits. A footprint's `pins`
must equal NN. Prints each missing id (up to 20 per line), then
`<n> lines, <k> without a footprint` (a line is without one when any of its position counts has none) and
`<m> footprints, <j> missing`. Exit 1 when something is missing.
"""
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
LIBRARY = os.path.join(ROOT, 'schematical', 'schematical', 'Parts', 'footprints.json')


def fp_id(letter, label, n):
    return f"{'' if letter == 'E' else 'TB-'}{label}-{n:02d}"


def main():
    args = sys.argv[1:]
    letter = args[0]
    json_path = os.path.join(DATA, f'footprints-{letter}.json')
    if '--json' in args:
        json_path = args[args.index('--json') + 1]
    have = {}
    if os.path.exists(json_path):
        for o in json.load(open(json_path))['footprints']:
            have[o['id']] = o
    if '--library' in args:
        for o in json.load(open(LIBRARY))['footprints']:
            have.setdefault(o['id'], o)
    lines = missing_lines = total = missing = 0
    for line in open(os.path.join(DATA, 'footprints-needed.txt')):
        if line.startswith('#') or not line.strip():
            continue
        f = line.rstrip('\n').split('\t')
        if f[0] != letter:
            continue
        lines += 1
        bad = []
        for n in (int(x) for x in f[7].split(',')):
            total += 1
            i = fp_id(letter, f[10], n)
            if i not in have:
                bad.append(f'{i} missing')
            elif have[i].get('pins') != n:
                bad.append(f"{i} has {have[i].get('pins')} pins")
        if bad:
            missing_lines += 1
            missing += len(bad)
            print(f[1], '|', '; '.join(bad[:20]))
    print(f'{lines} lines, {missing_lines} without a footprint')
    print(f'{total} footprints, {missing} missing')
    sys.exit(1 if missing else 0)


main()
