#!/usr/bin/env python3
"""plan-54-H to K: a part whose footprint cannot be drawn is not added.

usage: python3 -I fp-skip.py <letter>... [--apply]

For every line of footprints-needed.txt of the letters and every position count N whose footprint (`<stem>-<N>`, or the map's) is neither
in Parts/footprints.json nor in footprints-<letter>.json, the `add` rows of that line with DigiKey positions N become `skip` with reason
`footprint not drawn: <why>`: <why> is the line's entry in fp-notdrawn.tsv (columns stem, reason; written from the readers' reports:
`footprint not expressible: <feature>` or `no land pattern in the datasheet`), else `no land pattern in the datasheet`.
Prints the counts per (letter, reason); without --apply nothing is written.
"""
import collections
import csv
import json
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs  # noqa: E402

LIBRARY = os.path.join(specs.ROOT, 'schematical', 'schematical', 'Parts', 'footprints.json')


def main(argv):
    apply = '--apply' in argv
    letters = [a for a in argv if not a.startswith('--')]
    known = {f['id'] for f in json.load(open(LIBRARY))['footprints']}
    for L in letters:
        p = os.path.join(specs.DATA, f'footprints-{L}.json')
        if os.path.exists(p):
            known |= {f['id'] for f in json.load(open(p))['footprints']}
    fmap = {}
    for L in letters:
        p = os.path.join(specs.DATA, f'footprint-map-{L}.tsv')
        if os.path.exists(p):
            for r in specs.read_rows(p):
                if r['footprint']:
                    fmap[(r['stem'], int(r['positions']))] = r['footprint']
    why = {}
    p = os.path.join(specs.DATA, 'fp-notdrawn.tsv')
    if os.path.exists(p):
        why = {r['stem']: r['reason'] for r in specs.read_rows(p)}
    cand = specs.candidates()
    cols = list(cand[0].keys())
    fl = importlib_load('footprint-lines')
    lines = fl.build(cand, specs.load(), [])
    counts = collections.Counter()
    for L in lines:
        if L['letter'] not in letters:
            continue
        for N in L['positions']:
            fid = fmap.get((L['stem'], N)) or f"{L['stem']}-{N}"
            if fid in known:
                continue
            reason = 'footprint not drawn: ' + why.get(L['stem'], 'no land pattern in the datasheet')
            n = 0
            for r in L['members']:
                if int(r['positions']) == N and r['state'] == 'add':
                    if apply:
                        r['state'], r['reason'] = 'skip', reason
                    n += 1
            counts[(L['letter'], reason)] += n
    for (L, reason), n in sorted(counts.items()):
        print(f'{L}: {n} rows {reason}')
    print('total', sum(counts.values()))
    if apply:
        specs.write_rows(specs.CANDIDATES, cand, cols)


def importlib_load(name):
    import importlib.util
    spec = importlib.util.spec_from_file_location(name.replace('-', '_'), os.path.join(specs.HERE, name + '.py'))
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


main(sys.argv[1:])
