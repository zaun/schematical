#!/usr/bin/env python3
# plan-36-A: one stock-check.tsv line from a dk.sh --json response (one JSON object per page).
# usage: python3 -I stock-parse.py <mpn> <response.json>
import json, sys, datetime
mpn, path = sys.argv[1], sys.argv[2]
best = 0
for line in open(path):
    line = line.strip()
    if not line.startswith('{'):
        continue
    page = json.loads(line)
    for p in (page.get('ExactMatches') or []) + (page.get('Products') or []):
        if (p.get('ManufacturerProductNumber') or '').upper() != mpn.upper():
            continue
        q = p.get('QuantityAvailable') or 0
        for v in p.get('ProductVariations') or []:
            q = max(q, v.get('QuantityAvailableforPackageType') or 0)
        best = max(best, q)
print(f"{mpn}\t{1 if best > 0 else 0}\t{best}\t{datetime.date.today().isoformat()}")
