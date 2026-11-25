#!/usr/bin/env python3
"""plan-54-H to K: does every land-pattern line have a footprint with the right pad count?

usage: python3 -I fp-check.py <letter> [--scoped] [--map <map.tsv>]... [--json <footprints-X.json>]... [--needed <footprints-needed.txt>]

footprints-needed.txt has one line per variant of a letter (footprint-lines.py: letter, family, maker, series, mounting, card,
pitch, rows, positions, parts, example, datasheet, existing, id stem, status, variant). The footprint of a line for
position count N is the id `<stem>-<N>` (plus the map's choice below). The map `footprint-map-<letter>.tsv` (columns
`stem`, `positions`, `footprint`, `note`; one row per (stem, N) whose footprint is not simply `<stem>-<N>` of
footprints-<letter>.json: an existing library footprint, another variant's one) overrides that default. Known ids are
Parts/footprints.json and the footprints-<letter>.json files (default: every footprints-?.json of the data directory).

Checks, per (line, N): the footprint exists; its `pins` equals the pin count (specs.pin_count) of every `add` part of
that line with DigiKey positions N; with `edgeMount` / `opposite` pads matching the line's `edge-mount` status; and
every footprint of footprints-<letter>.json is used (else `unused`). Prints each problem and
`<n> footprint keys, <m> without a footprint` (exit 1 when m > 0 or something is unused).
"""
import csv
import glob
import json
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs  # noqa: E402

LIBRARY = os.path.join(specs.ROOT, 'schematical', 'schematical', 'Parts', 'footprints.json')


def objects(paths):
    out = {}
    for p in paths:
        if os.path.exists(p):
            for o in json.load(open(p))['footprints']:
                out[o['id']] = o
    return out


def main(argv):
    letter = argv[0]
    maps, jsons = [], []
    needed = os.path.join(specs.DATA, 'footprints-needed.txt')
    prune = '--prune' in argv   # rewrite the letter's footprints-<letter>.json without the footprints no line uses
    argv = [a for a in argv if a != '--prune']
    scoped = '--scoped' in argv   # a reader's worksheet: count only the keys its own json / map name
    argv = [a for a in argv if a != '--scoped']
    i = 1
    while i < len(argv):
        if argv[i] == '--map':
            maps.append(argv[i + 1])
        elif argv[i] == '--json':
            jsons.append(argv[i + 1])
        elif argv[i] == '--needed':
            needed = argv[i + 1]
        i += 2
    maps = maps or [os.path.join(specs.DATA, f'footprint-map-{letter}.tsv')]
    own = jsons or [os.path.join(specs.DATA, f'footprints-{letter}.json')]
    every = jsons or sorted(glob.glob(os.path.join(specs.DATA, 'footprints-?.json')))
    objs = objects([LIBRARY] + every)
    mine = objects(own)
    fmap = {}
    for path in maps:
        if os.path.exists(path):
            for r in csv.DictReader(open(path), delimiter='\t'):
                fmap[(r['stem'], int(r['positions']))] = r
    table = specs.load()
    cand = specs.candidates()
    by_variant = {}
    for r in cand:
        if r['state'] == 'add':
            by_variant.setdefault((r['maker'], r['variant'], r['pdf']), []).append(r)
    used = set()
    n = missing = 0
    for line in open(needed):
        if line.startswith('#') or not line.strip():
            continue
        f = line.rstrip('\n').split('\t')
        if f[0] != letter:
            continue
        stem, edge_mount = f[13], f[14] == 'edge-mount'
        variant = f[15]
        parts = by_variant.get((f[2], variant, f[11]), [])
        for N in [int(x) for x in f[8].split(',')]:
            m = fmap.get((stem, N))
            fid = m['footprint'] if m and m['footprint'] else f'{stem}-{N}'
            if scoped and not m and fid not in mine:
                continue
            n += 1
            bad = []
            if fid not in objs:
                bad.append(f'no footprint {fid}')
            else:
                used.add(fid)
                fp = objs[fid]
                for r in [r for r in parts if int(r['positions']) == N]:
                    v, _, _, spec = specs.verdict(r, table.get(specs.key(r)))
                    want = specs.pin_count(r, spec)
                    if fp['pins'] != want:
                        bad.append(f"{fid} has {fp['pins']} pins, {r['mpn']} needs {want}")
                        break
                has_opp = any(p.get('opposite') for p in fp.get('pads', []))
                if edge_mount and not (fp.get('edgeMount') and has_opp):
                    bad.append(f'{fid}: an edge-mount line needs edgeMount and opposite pads')
                if not edge_mount and fp.get('edgeMount'):
                    bad.append(f'{fid}: edgeMount on a line that is not edge-mount')
            if bad:
                missing += 1
                print('; '.join(bad) + ':', stem, N)
    unused = sorted(set(mine) - used)
    if prune and unused and not jsons:
        path = own[0]
        keep = [o for o in json.load(open(path))['footprints'] if o['id'] not in set(unused)]
        lines = ',\n'.join('    ' + json.dumps(o, ensure_ascii=False, separators=(',', ':')) for o in keep)
        open(path, 'w').write('{\n  "format": "schematical-footprints",\n  "version": 1,\n  "footprints": [\n' + lines + '\n  ]\n}\n')
        print(f'pruned {len(unused)} unused footprints from {os.path.basename(path)}')
        unused = []
    for u in unused:
        print('unused footprint', u)
    print(f'{n} footprint keys, {missing} without a footprint')
    return 1 if missing or unused else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
