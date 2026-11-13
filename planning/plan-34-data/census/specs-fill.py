#!/usr/bin/env python3
"""plan-34-C/D: fill specs.tsv's package, pinout and footprint columns.

usage: python3 -I specs-fill.py <candidates.tsv with add rows> [<specs.tsv>]

Each spec row's `add` rows are looked up in footprint-map.tsv by their
land-pattern line (footprint-lines.py's key: category, maker, datasheet,
package, body; and the series where the map names one). The columns get one
entry per distinct line, `;`-separated, in the form `<part_package>` /
`<pinout>` / `<part_package>:<footprint>`. A spec row with no `add` row gets
`-` in all three (no part uses it). A row whose add rows have no map entry
is an error (fp-check.py lists the line).
"""
import csv
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)


def line_key(r):
    name = r['pdf'].split('/')[-1]
    if r['category'] == 'through-hole':
        m = re.search(r'\(([^)]*)\)', r['size_dim'])
        body = m.group(1) if m else r['size_dim']
    elif r['category'] == 'array':
        body = f"{r['pins']} pins, {r['circuit']}"
    else:
        body = ''
    return (r['category'], r['maker'], name, r['package'], body)


def load_map():
    out = {}
    for m in csv.DictReader(open(os.path.join(DATA, 'footprint-map.tsv')), delimiter='\t'):
        key = (m['category'], m['maker'], m['datasheet'], m['package'], m['body'])
        out[key + (m['series'],)] = m
        out.setdefault(key + ('',), m)
    return out


def entry(fmap, r):
    k = line_key(r)
    return fmap.get(k + (r['series'],)) or fmap.get(k + ('',))


def main(cand, specs):
    fmap = load_map()
    adds = {}
    for r in csv.DictReader(open(cand), delimiter='\t'):
        if r['decision'] == 'add':
            adds.setdefault((r['maker'], r['series'], r['pdf']), []).append(r)
    rows = list(csv.DictReader(open(specs), delimiter='\t'))
    cols = list(rows[0].keys())
    bad = 0
    for s in rows:
        group = adds.get((s['maker'], s['series'], s['datasheet']), [])
        if not group:
            s['package'] = s['pinout'] = s['footprint'] = '-'
            continue
        seen = {}
        for r in group:
            m = entry(fmap, r)
            if m is None:
                print('no map entry:', r['mpn'], line_key(r))
                bad += 1
                continue
            seen[(m['part_package'], m['pinout'], m['footprint'])] = 1
        s['package'] = ';'.join(dict.fromkeys(p for p, _, _ in seen))
        s['pinout'] = ';'.join(dict.fromkeys(q for _, q, _ in seen))
        s['footprint'] = ';'.join(f'{p}:{f}' for p, _, f in seen)
    with open(specs, 'w') as out:
        out.write('\t'.join(cols) + '\n')
        for s in rows:
            out.write('\t'.join(s[c] for c in cols) + '\n')
    print(sum(1 for s in rows if s['footprint'] != '-'), 'spec rows with parts filled;',
          sum(1 for s in rows if s['footprint'] == '-'), 'with no add row;', bad, 'add rows unmapped')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else os.path.join(DATA, 'specs.tsv')))
