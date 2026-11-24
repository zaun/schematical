#!/usr/bin/env python3
"""plan-54-B/C/D: a row whose own datasheet does not list it may be listed by a sibling datasheet of the same maker.

usage: python3 -I sibling.py [--family F]... [--maker M]... [--apply] [candidates.tsv]

DigiKey links a part to one document, which is sometimes another part's drawing of the same family. For every row of the
selection that is `candidate`, or `skip` with reason `not in the datasheet's ordering table`, whose own PDF does not list it,
this looks for another spec line of the same family and maker (a PDF still in the tree, scheme not `-`) whose scheme
matches the part number and under which the row would be `ok`. A row found that way is re-pointed: `pdf` and `url` become the
sibling's (`datasheets.tsv` gives the maker URL), `series` the sibling's spec line's, `state` `candidate`, `reason` empty
(final.py then adds it). A row two siblings list is re-pointed to the first (sorted by path) and counted `ambiguous`.
Prints per (family, maker) the counts; without --apply nothing is written.
"""
import collections
import csv
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs  # noqa: E402

FOLDER = {'card': 'connector-card-socket', 'module': 'connector-memory-module', 'edge': 'connector-card-edge'}


def maker_url(pdf):
    folder = pdf.split('/')[1]
    tsv = os.path.join(specs.ROOT, 'datasheets', folder, 'datasheets.tsv')
    if not os.path.exists(tsv):
        return None
    rel = os.path.relpath(pdf, os.path.join('datasheets', folder))
    urls = sorted(r['url'] for r in specs.read_rows(tsv) if r['path'] == rel)
    return urls[0] if urls else None


def main(argv):
    fams, makers, rest = [], [], []
    apply = '--apply' in argv
    i = 0
    while i < len(argv):
        if argv[i] == '--family':
            fams.append(argv[i + 1]); i += 2; continue
        if argv[i] == '--maker':
            makers.append(argv[i + 1]); i += 2; continue
        if not argv[i].startswith('--'):
            rest.append(argv[i])
        i += 1
    path = rest[0] if rest else specs.CANDIDATES
    rows = specs.candidates(path)
    cols = list(rows[0].keys())
    table = specs.load()
    lines = [(k, s) for k, ss in table.items() for s in ss]
    counts = collections.Counter()
    for r in rows:
        if (fams and r['family'] not in fams) or (makers and r['maker'] not in makers):
            continue
        if not (r['state'] == 'candidate' or (r['state'] == 'skip' and r['reason'] == specs.NOT_TABLE)):
            continue
        v, why, _, _ = specs.verdict(r, table.get(specs.key(r)))
        if v == 'ok' or (v == 'skip' and why != specs.NOT_TABLE):
            continue
        if v == 'unchecked':
            continue
        found = []
        for k, s in sorted(lines, key=lambda x: x[0][3]):
            if k[0] != r['family'] or k[1] != r['maker'] or k[3] == r['pdf'] or s['scheme'] in ('', '-'):
                continue
            if not os.path.exists(os.path.join(specs.ROOT, k[3])) or not specs.scheme_matches(s, r['mpn']):
                continue
            trial = dict(r, series=k[2], pdf=k[3], hit='0')
            v2, _, _, _ = specs.verdict(trial, table.get(specs.key(trial)))
            if v2 == 'ok' and k[3] not in [f[0] for f in found]:
                found.append((k[3], k[2]))
        group = f"{r['family']} {r['maker']}"
        if not found:
            counts[(group, 'no sibling')] += 1
            continue
        counts[(group, 'rescued')] += 1
        if len({f[0] for f in found}) > 1:
            counts[(group, 'ambiguous')] += 1
        url = maker_url(found[0][0])
        if url is None:
            counts[(group, 'no url')] += 1
            continue
        if apply:
            r['pdf'], r['series'], r['url'], r['state'], r['reason'] = found[0][0], found[0][1], url, 'candidate', ''
    for (group, what), n in sorted(counts.items()):
        print(f'{group}: {what} {n}')
    if apply:
        specs.write_rows(path, rows, cols)
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
