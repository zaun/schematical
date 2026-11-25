#!/usr/bin/env python3
"""plan-55 census: stock-check.tsv from the recheck listing (plan-55-B Phase 2).

usage: python3 -I stock.py <recheck raw dir> candidates.tsv > stock-check.tsv
One row per candidate row: family, mpn, found (1 when the second listing has the part again, in stock and
Active), stock (the quantity of the second listing, 0 when not found), status, date. The recheck listing is
`list.sh <dir> --opt2` run on a later call, so a part sold out or discontinued since the first listing is not in it.
"""
import datetime
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

again = {}
for r in common.read_listing(sys.argv[1]):
    if not r['dup']:
        again[(r['family'], r['mpn'])] = r
today = datetime.date.today().isoformat()
print('family\tmpn\tfound\tstock\tstatus\tdate')
for r in common.read_tsv(sys.argv[2]):
    if not common.eligible(r):
        continue
    a = again.get((r['family'], r['mpn']))
    ok = a is not None and (a.get('stock') or 0) > 0 and a.get('status') == 'Active'
    print(f"{r['family']}\t{r['mpn']}\t{1 if ok else 0}\t{a.get('stock') or 0 if a else 0}\t{a.get('status', '') if a else ''}\t{today}")
