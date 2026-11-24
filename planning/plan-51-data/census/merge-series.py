#!/usr/bin/env python3
"""plan-51-C: merges planning/plan-51-data/series-parts/*.tsv (the rows read per maker) into series.tsv,
sorted by maker, series, family, mounting, spacing. Prints the number of rows."""
import glob, os

DATA = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
header, rows = None, []
for path in sorted(glob.glob(os.path.join(DATA, 'series-parts', '*.tsv'))):
    lines = [l.rstrip('\n') for l in open(path) if l.strip()]
    header = header or lines[0]
    assert lines[0] == header, path
    rows += [l.split('\t') for l in lines[1:]]
rows.sort(key=lambda r: (r[0], r[1], r[2], r[3], r[4]))
with open(os.path.join(DATA, 'series.tsv'), 'w') as f:
    f.write(header + '\n')
    for r in rows:
        f.write('\t'.join(r) + '\n')
print(len(rows), 'rows')
