#!/usr/bin/env python3
"""plan-54-H to K: assemble the readers' files into footprints-<letter>.json and footprint-map-<letter>.tsv.

usage: python3 -I fp-assemble.py <letter>...

Reads planning/plan-54-data/fp-parts/<name>.json (and <name>.map.tsv) of the letter's readers (H: card-*, module-*; I: edge-samtec-*;
J: edge-te-*; K: edge-amphenol-*), checks that no id repeats and every pad number is in 1..pins, writes
footprints-<letter>.json (the footprints sorted by id, one per line, the format of footprints.json) and the map (columns stem,
positions, footprint, note). Prints the counts.
"""
import glob
import json
import os
import re
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs  # noqa: E402

PARTS = os.path.join(specs.DATA, 'fp-parts')
PATTERN = {'H': ['card-', 'module-'], 'I': ['edge-samtec-'], 'J': ['edge-te-'], 'K': ['edge-amphenol-']}


def dropped():
    """stems of fp-notdrawn.tsv with `drop` 1: their footprints are left out although a reader drew them (judged unreliable)."""
    p = os.path.join(specs.DATA, 'fp-notdrawn.tsv')
    if not os.path.exists(p):
        return set()
    return {r['stem'] for r in specs.read_rows(p) if r.get('drop') == '1'}


def main(letters):
    drop = dropped()
    for L in letters:
        files = sorted(f for f in glob.glob(os.path.join(PARTS, '*.json'))
                       if os.path.basename(f).startswith(tuple(PATTERN[L])) and not f.endswith('-params.json'))
        fps = {}
        for f in files:
            for fp in json.load(open(f))['footprints']:
                if any(re.fullmatch(re.escape(d) + r'-\d+', fp['id']) for d in drop):
                    continue
                if fp['id'] in fps and fps[fp['id']] != fp:
                    sys.exit(f'fp-assemble: {fp["id"]} differs between files ({f})')
                fps[fp['id']] = fp
        for fid, fp in fps.items():
            for p in fp['pads']:
                if not (0 <= p['pin'] <= fp['pins']):
                    sys.exit(f'fp-assemble: {fid} pad pin {p["pin"]} outside 1..{fp["pins"]}')
        lines = ',\n'.join('    ' + json.dumps(fps[k], ensure_ascii=False, separators=(',', ':')) for k in sorted(fps))
        out = os.path.join(specs.DATA, f'footprints-{L}.json')
        open(out, 'w').write('{\n  "format": "schematical-footprints",\n  "version": 1,\n  "footprints": [\n' + lines + '\n  ]\n}\n')
        maps = []
        for f in sorted(glob.glob(os.path.join(PARTS, '*.map.tsv'))):
            if os.path.basename(f).startswith(tuple(PATTERN[L])):
                maps.extend(r for r in specs.read_rows(f) if r['stem'] not in drop)
        specs.write_rows(os.path.join(specs.DATA, f'footprint-map-{L}.tsv'), maps, ['stem', 'positions', 'footprint', 'note'])
        print(f'{L}: {len(fps)} footprints from {len(files)} files, {len(maps)} map rows')


main(sys.argv[1:])
