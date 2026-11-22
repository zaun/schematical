#!/usr/bin/env python3
"""plan-44-A section 5: the download-set rows with no part-by-part stock check yet.

usage: python3 -I unchecked.py <raw dir> <candidates.tsv> [<stock-check.tsv>]

The download set is every row whose `state` is not `out` (candidate and
reserve). Prints `<maker id> TAB <mpn>` per such row, in the order of
candidates.tsv (the most wanted rows first), skipping part numbers already in
<stock-check.tsv> when given (the maker id is the row's own DigiKey id, from
the raw listing). The counts go to stderr.
"""
import csv
import glob
import gzip
import json
import os
import sys


def main():
    raw, cand = sys.argv[1], sys.argv[2]
    done = set()
    if len(sys.argv) > 3 and os.path.exists(sys.argv[3]):
        done = {r['mpn'] for r in csv.DictReader(open(sys.argv[3]), delimiter='\t')}
    ids = {}
    for path in sorted(glob.glob(os.path.join(raw, '*.jsonl*'))):
        for line in (gzip.open(path, 'rt') if path.endswith('.gz') else open(path)):
            d = json.loads(line)
            ids.setdefault(d['mpn'], d['makerId'])
    n = todo = 0
    for r in csv.DictReader(open(cand), delimiter='\t'):
        if r['state'] == 'out':
            continue
        n += 1
        if r['mpn'] in done:
            continue
        todo += 1
        print(f"{ids[r['mpn']]}\t{r['mpn']}")
    print(f'unchecked: {n} download-set rows, {todo} unchecked', file=sys.stderr)


if __name__ == '__main__':
    main()
