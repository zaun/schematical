#!/usr/bin/env python3
"""plan-40-B (Corrections 1): the spec rows the final pick can depend on.

usage: python3 -I need-groups.py <candidates.tsv> <specs.tsv> [<margin>]

Runs the pick of select.py (the duplicate rule, then the per-(family,
maker) share, the named parts always) over the eligible rows that are not
yet known to fail: a PDF, `dead` not 1, and — where their spec row is read
(`scheme` filled and not `n/r`) — in the ordering table with a pinout. A
row whose spec row is not read counts as passing (optimistic). Prints one
line per spec row (maker TAB series TAB pdf TAB package) that a picked row,
or one of the next <margin> (default 10) rows of its (family, maker), uses
and that is not read yet; the counts go to stderr. An empty output means
every spec row the pick can reach is read.
"""
import collections
import csv
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402
import specs  # noqa: E402

QUOTA = {'bjt': 450, 'bjt-array': 60, 'bjt-prebiased': 120, 'bjt-prebiased-array': 40,
         'mosfet': 600, 'mosfet-array': 80}
NAMED = ['2N3904', '2N3906', 'BC547B', 'BC557B', '2N7000', 'BS170', '2N7002']


def is_read(s):
    return s is not None and s['scheme'] not in ('', 'n/r')


def passes(r, s):
    if not r['pdf'] or r['dead'] == '1':
        return False
    if not is_read(s):
        return True
    return specs.in_table(r, s) and s['pinout'] not in ('', '-')


def main(cand, spec_path, margin=10):
    table = specs.load(spec_path)
    rows = [r for r in csv.DictReader(open(cand), delimiter='\t') if common.eligible(r)]
    ok = sorted((r for r in rows if passes(r, table.get(specs.key(r)))), key=lambda r: -int(r['stock']))
    best = {}
    for r in ok:
        best.setdefault((r['family'], r['base'].upper(), r['package'], r['supplier_package']), r)
    winners = {id(r) for r in best.values()}
    count = collections.Counter()
    extra = collections.Counter()
    need = {}
    for r in ok:
        if id(r) not in winners:
            continue
        fm = (r['family'], r['maker'])
        named = any(r['base'].upper() == n or r['mpn'].upper().startswith(n) for n in NAMED)
        k = specs.key(r)
        if count[fm] < -(-QUOTA[r['family']] // 3) or named:
            count[fm] += 1
        elif extra[fm] < margin:
            extra[fm] += 1
        else:
            continue
        if not is_read(table.get(k)):
            need[k] = 1
    for k in sorted(need):
        print('\t'.join(k))
    print(f'need-groups: {sum(count.values())} picked; {len(need)} spec rows to read',
          dict(collections.Counter(k[0] for k in need)), file=sys.stderr)


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2], int(sys.argv[3]) if len(sys.argv) > 3 else 10)
