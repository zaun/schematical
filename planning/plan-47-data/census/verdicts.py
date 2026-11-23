#!/usr/bin/env python3
"""plan-47-C section 3.4: the final pick.

usage: python3 -I verdicts.py [--apply]

For every (type, maker, class) of the download set the rows (state `candidate`
or `reserve`) go in rank order (the `rank` column, select.py's round robin over
the series). The class quota is the number of `candidate` rows the group has
(select.py already redistributed an unused quota). A row whose verdict is `ok`
is `add` until the quota is full; the other `ok` rows are `reserve`; a row
whose verdict is `skip` or `out` keeps it. Prints the pick per (type, maker,
class) with its quota and every shortfall. --apply writes `state` (`add`, `reserve`,
`skip` or `out`) and `reason` (the verdict's reason for a skip or out) of the download-set
rows of candidates.tsv; rows outside the download set are untouched. Rows without a
verdict stop the run (B and C have not covered them).
"""
import collections
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs47 as sp  # noqa: E402


def main(argv):
    rows = sp.read(sp.CANDIDATES)
    cols = list(rows[0].keys())
    groups = collections.defaultdict(list)
    for r in rows:
        r['final'] = r['why'] = ''
        if sp.in_set(r):
            groups[(r['type'], r['company'], r['class'])].append(r)
    unjudged = [r for g in groups.values() for r in g if not r.get('verdict')]
    if unjudged:
        print(f"{len(unjudged)} download-set rows have no verdict, e.g. {unjudged[0]['mpn']}")
        return 1
    counts = collections.Counter()
    for k in sorted(groups):
        g = sorted(groups[k], key=lambda r: (int(r['rank'] or 0), r['mpn']))
        quota = sum(1 for r in g if r['state'] == 'candidate')
        added = 0
        for r in g:
            if r['verdict'] == 'ok':
                if added < quota:
                    r['final'] = 'add'
                    added += 1
                else:
                    r['final'] = 'reserve'
            else:
                r['final'], r['why'] = r['verdict'], r['vreason']
            counts[(r['type'], r['final'])] += 1
        short = quota - added
        print(f"{k[0]} {k[1]} {k[2] or '-'}: quota {quota}, add {added}" + (f"  SHORT by {short}" if short else ''))
    for k in sorted(counts):
        print(f"{k[0]} {k[1]}: {counts[k]}")
    if '--apply' in argv:
        for r in rows:
            if r['final']:
                r['state'], r['reason'] = r['final'], r['why']
        sp.write(sp.CANDIDATES, rows, cols)
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
