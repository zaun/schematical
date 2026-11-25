#!/usr/bin/env python3
"""plan-45-C/D/E: make `add` rows `skip` because no footprint can be drawn for them.

usage: python3 -I skip-rows.py <reason> <mpn>[,<mpn>...]

Sets `state` `skip` and `reason` in candidates.tsv for the named `add` rows. The rows must be listed in
datasheets/skipped-parts.md (`final.py --verify-skipped`), the footprint list is regenerated with
`footprint-lines.py candidates.tsv generic-reps.tsv > footprints-needed.txt`, and the PDFs only these rows used are
deleted by `place.py --prune`.
"""
import csv
import os
import sys

DATA = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def main():
    reason, mpns = sys.argv[1], set(sys.argv[2].split(','))
    path = os.path.join(DATA, 'candidates.tsv')
    rows = list(csv.DictReader(open(path), delimiter='\t'))
    cols = list(rows[0].keys())
    n = 0
    for r in rows:
        if r['mpn'] in mpns:
            if r['state'] != 'add':
                sys.exit(f"{r['mpn']} is {r['state']}, not add")
            r['state'], r['reason'] = 'skip', reason
            n += 1
    if n != len(mpns):
        sys.exit(f'{n} of {len(mpns)} rows found')
    with open(path, 'w') as out:
        out.write('\t'.join(cols) + '\n')
        for r in rows:
            out.write('\t'.join(str(r[c]) for c in cols) + '\n')
    print(f'{n} rows skipped: {reason}')


main()
