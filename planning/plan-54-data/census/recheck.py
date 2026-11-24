#!/usr/bin/env python3
"""plan-54-A § 5: the stock re-check.

usage: python3 -I recheck.py <new listing dir> candidates.tsv > stock-check.tsv

The new listing is one `<category>-<maker id>.jsonl` per piece, each line one raw DigiKey response
({"Products": [...]}) as `scripts/digikey.js --opt2 --category C --manufacturer M --in-stock --active-only
--limit 100000 --json` prints it. For every `candidate` row of candidates.tsv the output has: mpn,
found (1 when the part number is in the new listing of its category with stock above 0 and status Active),
stock (the new quantity), status (the new status, empty when the part is not listed), date (the day of the
newest listing file). Rows are in candidates.tsv order. A one-line summary goes to stderr.
"""
import csv
import datetime
import glob
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402


def main():
    new, cand = sys.argv[1], sys.argv[2]
    listing = {}
    newest = 0
    pieces = 0
    for f in sorted(glob.glob(os.path.join(new, '*.jsonl'))):
        category = os.path.basename(f).split('-')[0]
        pieces += 1
        newest = max(newest, os.path.getmtime(f))
        with open(f) as fh:
            for line in fh:
                line = line.strip()
                if not line.startswith('{'):
                    continue
                for p in json.loads(line).get('Products', []):
                    key = (category, p['ManufacturerProductNumber'])
                    stock = p.get('QuantityAvailable') or 0
                    status = (p.get('ProductStatus') or {}).get('Status', '')
                    # one MPN under two packings: keep the best row
                    if key not in listing or stock > listing[key][0]:
                        listing[key] = (stock, status)
    date = datetime.date.fromtimestamp(newest).isoformat()
    cat_of = {v: k for k, v in common.FAMILY.items()}
    w = csv.writer(sys.stdout, delimiter='\t', lineterminator='\n')
    w.writerow(['mpn', 'found', 'stock', 'status', 'date'])
    total = bad = 0
    for r in csv.DictReader(open(cand), delimiter='\t'):
        if r['state'] != 'candidate':
            continue
        total += 1
        stock, status = listing.get((cat_of[r['family']], r['mpn']), (0, ''))
        found = 1 if stock > 0 and status == 'Active' else 0
        bad += 1 - found
        w.writerow([r['mpn'], found, stock, status, date])
    print(f'recheck: {pieces} pieces, {len(listing)} parts listed, {total} candidates, {bad} not found', file=sys.stderr)


main()
