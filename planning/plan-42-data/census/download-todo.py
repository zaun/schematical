#!/usr/bin/env python3
"""plan-42-A section 5: the download-set rows the part-by-part stock check
still has to look up.

usage: python3 -I download-todo.py <raw dir> <candidates.tsv> [<stock-check.tsv>]

Prints `<maker id> TAB <mpn>` per download-set row (download-set.py on
<candidates.tsv>) whose part number is not yet in <stock-check.tsv>; the maker
id is the row's own DigiKey id from the raw listing. Counts go to stderr.
"""
import csv
import glob
import io
import json
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))


def main():
    raw, cand = sys.argv[1], sys.argv[2]
    done = set()
    if len(sys.argv) > 3 and os.path.exists(sys.argv[3]):
        done = {r['mpn'] for r in csv.DictReader(open(sys.argv[3]), delimiter='\t')}
    ids = {}
    for path in sorted(glob.glob(os.path.join(raw, '*.jsonl'))):
        for line in open(path):
            d = json.loads(line)
            ids.setdefault(d['mpn'], d['makerId'])
    out = subprocess.run([sys.executable, '-I', os.path.join(HERE, 'download-set.py'), cand],
                         capture_output=True, text=True, check=True).stdout
    n = todo = 0
    for r in csv.DictReader(io.StringIO(out), delimiter='\t'):
        n += 1
        if r['mpn'] in done:
            continue
        todo += 1
        print(f"{ids[r['mpn']]}\t{r['mpn']}")
    print(f'download-todo: {n} download-set rows, {todo} unchecked', file=sys.stderr)


if __name__ == '__main__':
    main()
