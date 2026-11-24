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


def build(cand, table, ids, fams=(), makers=()):
    """The lines: dicts (letter, family, maker, series, mounting, card, pitch, rows, positions, parts, example, pdf, existing,
    stem, status, variant, rows_of_line, spec) for the `add` rows (`candidate` rows when no row is `add` yet)."""
    wanted = ('add',) if any(r['state'] == 'add' for r in cand) else ('candidate',)
    groups = collections.defaultdict(list)
    held = set()   # groups that only hold rows skipped for want of a footprint: they keep their stem number
    for r in cand:
        if (fams and r['family'] not in fams) or (makers and r['maker'] not in makers):
            continue
        k = (letter(r), r['family'], r['maker'], r['variant'], r['pdf'])
        if r['state'] in wanted:
            groups[k].append(r)
        elif r['state'] == 'skip' and r['reason'].startswith('footprint'):
            held.add(k)
    seen = collections.Counter()
    out = []
    for k in sorted(set(groups) | held):
        g = groups.get(k)
        first = g[0] if g else None
        if first is None:
            # a group with no row left: it still takes its number (stem letter) in its series
            ex0 = next(r for r in cand if (letter(r), r['family'], r['maker'], r['variant'], r['pdf']) == k)
            seen[f"{KIND[ex0['family']]}-{MAKER[ex0['maker']]}-{slug(ex0)}"] += 1
            continue
        ex = max(g, key=lambda r: (int(r['stock'] or 0), r['mpn']))
        _, _, _, spec = specs.verdict(ex, table.get(specs.key(ex)))
        stem = f"{KIND[ex['family']]}-{MAKER[ex['maker']]}-{slug(ex)}"
        seen[stem] += 1
        if seen[stem] > 1:   # another variant of the same series: -B, -C ...
            stem += '-' + chr(ord('A') + seen[stem] - 1)
        out.append({'letter': k[0], 'family': k[1], 'maker': k[2], 'series': ex['series'], 'mounting': ex['mounting'],
                    'card': spec['card'] if spec else '', 'pitch': spec['pitch'] if spec else '', 'rows': ex['rows'],
                    'positions': sorted({int(r['positions']) for r in g}), 'parts': len(g), 'example': ex['mpn'],
                    'pdf': ex['pdf'], 'existing': [i for i in ids if i.startswith(stem)], 'stem': stem,
                    'status': 'edge-mount' if spec and 'edge-mount' in specs.status_words(spec) else '',
                    'variant': ex['variant'], 'members': g})
    return out


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
    ids = [f['id'] for f in json.load(open(FOOTPRINTS))['footprints']]
    print('# letter\tfamily\tmaker\tseries\tmounting\tcard\tpitch\trows\tpositions\tparts\texample\tdatasheet\texisting footprints\tid stem\tstatus\tvariant')
    for L in build(cand, specs.load(), ids, fams, makers):
        print('\t'.join([L['letter'], L['family'], L['maker'], L['series'], L['mounting'], L['card'], L['pitch'], L['rows'],
                         ','.join(str(n) for n in L['positions']), str(L['parts']), L['example'], L['pdf'],
                         ' '.join(L['existing']), L['stem'], L['status'], L['variant']]))


if __name__ == '__main__':
    main()
