#!/usr/bin/env python3
"""plan-54-B/C/D: footprints-needed.txt from candidates.tsv and specs.tsv.

usage: python3 -I footprint-lines.py <candidates.tsv> [--family F]... [--maker M]... > footprints-needed.txt

One line per variant (select.py's `variant`: the footprint family) of the `add` rows (the `candidate` rows when no `add`
row exists yet). Tab-separated, header lines start with `#`: letter (H memory / SIM and module sockets, I Samtec edge,
J TE edge, K Amphenol ICC edge), family, maker, series, mounting, card (the spec line's), pitch (the spec line's), rows,
the position counts, parts, example part (most stock), its datasheet file, the existing footprints with the id stem,
the id stem (`SOCK-<MAKER>-<series>`, `DIMM-<MAKER>-<series>`, `EDGE-<MAKER>-<series>`, series slug upper case;
`<pdf stem>` for a series `-`), `edge-mount` when the spec line says so.
"""
import collections
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import specs  # noqa: E402

FOOTPRINTS = os.path.join(specs.ROOT, 'schematical', 'schematical', 'Parts', 'footprints.json')
MAKER = {'Same Sky': 'SAMESKY', 'Amphenol ICC': 'AMPHENOL', 'GCT': 'GCT', 'TE Connectivity': 'TE', 'Molex': 'MOLEX',
         'Samtec': 'SAMTEC'}
KIND = {'card': 'SOCK', 'module': 'DIMM', 'edge': 'EDGE'}


def letter(r):
    if r['family'] != 'edge':
        return 'H'
    return {'Samtec': 'I', 'TE Connectivity': 'J', 'Amphenol ICC': 'K'}[r['maker']]


def slug(r):
    s = r['series'] if r['series'] not in ('', '-') else os.path.splitext(os.path.basename(r['pdf']))[0]
    return re.sub(r'[^A-Z0-9]+', '-', s.upper()).strip('-')


def main():
    args = sys.argv[1:]
    fams, makers, rest = [], [], []
    i = 0
    while i < len(args):
        if args[i] == '--family':
            fams.append(args[i + 1]); i += 2; continue
        if args[i] == '--maker':
            makers.append(args[i + 1]); i += 2; continue
        rest.append(args[i]); i += 1
    cand = specs.candidates(rest[0] if rest else specs.CANDIDATES)
    table = specs.load()
    ids = [f['id'] for f in json.load(open(FOOTPRINTS))['footprints']]
    wanted = ('add',) if any(r['state'] == 'add' for r in cand) else ('candidate',)
    groups = collections.defaultdict(list)
    for r in cand:
        if r['state'] in wanted and (not fams or r['family'] in fams) and (not makers or r['maker'] in makers):
            groups[(letter(r), r['family'], r['maker'], r['variant'])].append(r)
    print('# letter\tfamily\tmaker\tseries\tmounting\tcard\tpitch\trows\tpositions\tparts\texample\tdatasheet\texisting footprints\tid stem\tstatus')
    for k in sorted(groups):
        g = groups[k]
        ex = max(g, key=lambda r: (int(r['stock'] or 0), r['mpn']))
        spec = None
        _, _, _, spec = specs.verdict(ex, table.get(specs.key(ex)))
        stem = f"{KIND[ex['family']]}-{MAKER[ex['maker']]}-{slug(ex)}"
        same = [i for i in ids if i.startswith(stem)]
        pos = ','.join(str(n) for n in sorted({int(r['positions']) for r in g}))
        print('\t'.join([k[0], k[1], k[2], ex['series'], ex['mounting'], spec['card'] if spec else '', spec['pitch'] if spec else '',
                         ex['rows'], pos, str(len(g)), ex['mpn'], ex['pdf'], ' '.join(same), stem,
                         'edge-mount' if spec and 'edge-mount' in specs.status_words(spec) else '']))


main()
