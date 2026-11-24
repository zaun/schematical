#!/usr/bin/env python3
"""plan-51-C phase 2: every candidate that is not `out` becomes `add`; a PDF that only `out` rows cite is deleted from
datasheets/ic-socket/ (skipped-parts.md says the PDFs of skipped parts are not in the tree), its rows get `-` in the
pdf column, and its line leaves the folder's datasheets.tsv; the `parts` count of every other PDF is the number of
`add` rows citing it. Run it once, after the verdicts."""
import collections, os

DATA = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
ROOT = os.path.join(DATA, '..', '..')
TSV = os.path.join(DATA, 'candidates.tsv')
lines = [l.rstrip('\n').split('\t') for l in open(TSV)]
header, rows = lines[0], lines[1:]
for r in rows:
    if r[11] != 'out':
        r[11] = 'add'
cited = collections.Counter(r[10] for r in rows if r[11] == 'add')
removed = set()
for r in rows:
    if r[11] == 'out' and r[10] != '-' and cited[r[10]] == 0:
        removed.add(r[10])
        r[10] = '-'
for path in removed:
    os.remove(os.path.join(ROOT, path))
with open(TSV, 'w') as f:
    f.write('\t'.join(header) + '\n')
    for r in rows:
        f.write('\t'.join(r) + '\n')
sheets = {os.path.dirname(p) for p in cited if p != '-'} | {os.path.dirname(p) for p in removed}
for folder in sorted(sheets):
    path = os.path.join(ROOT, folder, 'datasheets.tsv')
    text = [l.rstrip('\n').split('\t') for l in open(path)]
    out = [text[0]]
    for t in text[1:]:
        key = f'{folder}/{t[0]}'
        if key in removed:
            continue
        t[3] = str(cited[key])
        out.append(t)
    with open(path, 'w') as f:
        f.write('\n'.join('\t'.join(t) for t in out) + '\n')
print('add', sum(1 for r in rows if r[11] == 'add'), 'out', sum(1 for r in rows if r[11] == 'out'),
      'PDFs removed', sorted(removed))
