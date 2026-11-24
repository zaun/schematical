#!/usr/bin/env python3
"""plan-45-A § 5: the stock re-check of the candidate rows from a fresh listing.

usage: python3 -I recheck.py <fresh raw dir> <candidates.tsv> > stock-check.tsv

<fresh raw dir> is the output of `census/list.sh` (seven `<piece>.jsonl` + `.err` + `.done`). One row
per `candidate` row of candidates.tsv: mpn, found (1 when the part number is in the fresh listing with
stock above 0 and status Active or Last Time Buy), stock, status, date.

A piece cut at DigiKey's 300-result cap would make a part look out of stock when it only fell off the
list, so a fresh listing with a `cut at` line, or a piece without `.done`, is refused (exit 1).
"""
import csv
import datetime
import glob
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

PIECES = ['314-455-jst', '371-277-t', '371-281-t', '371-732-t', '370-277-t', '370-281-t', '370-732-t']


def main():
    raw, cand = sys.argv[1], sys.argv[2]
    for piece in PIECES:
        if not os.path.exists(os.path.join(raw, piece + '.done')):
            sys.exit(f'recheck.py: {piece} has no .done in {raw}: run census/list.sh first')
    for path in glob.glob(os.path.join(raw, '*.err')):
        for line in open(path):
            if 'cut at' in line:
                sys.exit(f'recheck.py: {os.path.basename(path)}: {line.strip()}: a cut piece needs part-by-part lookups')
    fresh = {}
    for r in common.read_raw(raw):
        fresh[r['mpn']] = (r['stock'], r['status'])
    today = datetime.datetime.now(datetime.timezone.utc).date().isoformat()     # UTC, as the plans date things
    print('mpn\tfound\tstock\tstatus\tdate')
    found = lost = 0
    for row in csv.DictReader(open(cand), delimiter='\t'):
        if row['state'] != 'candidate':
            continue
        stock, status = fresh.get(row['mpn'], (0, ''))
        ok = stock > 0 and status in ('Active', 'Last Time Buy')
        found += ok
        lost += not ok
        print(f"{row['mpn']}\t{1 if ok else 0}\t{stock}\t{status}\t{today}")
    print(f'recheck.py: {found} found, {lost} not found, {len(fresh)} parts in the fresh listing', file=sys.stderr)


main()
