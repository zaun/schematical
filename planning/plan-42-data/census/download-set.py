#!/usr/bin/env python3
"""plan-42 census: the download set — the rows whose datasheets plan-42-A fetches
and whose stock it re-checks part by part.

usage: python3 -I download-set.py <candidates.tsv> [--report] > download-set.tsv

The set is, per (family, maker), the picked rows (`candidate`) and then the
`below quota` rows by falling stock, up to 1.5 x the maker's share of the
family's quota (linear-fixed 120, linear-adjustable 45, linear-multi 15,
switching 120), so plan-42-B can refill a slot it skips with the next row of
the same (family, maker) without another listing. Output: mpn, maker, family,
stock, reason, datasheet_url (tab-separated, a header line). --report prints
the counts to stderr: rows, distinct URLs, rows per datasheet host.
"""
import collections
import csv
import sys

QUOTA = {'linear-fixed': 240, 'linear-adjustable': 90, 'linear-multi': 30, 'switching': 240}

rows = list(csv.DictReader(open(sys.argv[1]), delimiter='\t'))
pool = [r for r in rows if r['reason'] in ('candidate', 'below quota')]
pool.sort(key=lambda r: (r['reason'] != 'candidate', -int(r['stock']), r['mpn']))
taken = collections.Counter()
chosen = []
for r in pool:
    key = (r['family'], r['maker'])
    if taken[key] < QUOTA[r['family']] // 3 * 3 // 2:
        taken[key] += 1
        chosen.append(r)
w = csv.writer(sys.stdout, delimiter='\t', lineterminator='\n')
w.writerow(['mpn', 'maker', 'family', 'stock', 'reason', 'datasheet_url'])
for r in chosen:
    w.writerow([r['mpn'], r['maker'], r['family'], r['stock'], r['reason'], r['datasheet_url']])
if '--report' in sys.argv:
    print(f'{len(chosen)} rows; {len({r["datasheet_url"] for r in chosen})} distinct URLs', file=sys.stderr)
    hosts = collections.Counter((r['datasheet_url'].split('/')[2] if r['datasheet_url'] else 'none') for r in chosen)
    print('hosts:', dict(hosts.most_common()), file=sys.stderr)
    print('per (family, maker):', dict(sorted(collections.Counter((r['family'], r['maker']) for r in chosen).items())),
          file=sys.stderr)
