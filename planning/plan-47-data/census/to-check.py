#!/usr/bin/env python3
"""plan-47-A section 5: the rows the part-by-part stock check has not looked up yet.

usage: python3 -I to-check.py <raw dir> <clips.jsonl> <candidates.tsv> [<stock-check.tsv>]

Prints `<maker id> TAB <mpn>` for every download-set row (state `candidate` or
`reserve`) of candidates.tsv and for the six clips of plan-47-A section 6,
skipping part numbers already in <stock-check.tsv> when given. The maker id is
the row's own DigiKey id from the raw listing (a company can have several), so
the look-up needs no extra call. The counts go to stderr.
"""
import csv
import glob
import gzip
import json
import os
import sys

# plan-47-A section 6: the most-stocked through-hole clip per (maker, fuse size).
CLIPS = ['01110501Z', '01020071Z', 'BK/1A5018-10-R', 'BK/1A1907-06-R', '0751.0052', '0751.0056']


def main():
    raw, clips, cand = sys.argv[1:4]
    done = set()
    if len(sys.argv) > 4 and os.path.exists(sys.argv[4]):
        done = {r['mpn'] for r in csv.DictReader(open(sys.argv[4], encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE)}
    ids = {}
    for path in sorted(glob.glob(os.path.join(raw, '*.jsonl')) + glob.glob(os.path.join(raw, '*.jsonl.gz'))):
        opener = gzip.open if path.endswith('.gz') else open
        with opener(path, 'rt', encoding='utf-8') as f:
            for line in f:
                d = json.loads(line)
                ids.setdefault(d['mpn'], d['makerId'])
    clip_ids = {}
    for line in open(clips, encoding='utf-8'):
        d = json.loads(line)
        clip_ids.setdefault(d['mpn'], d['makerId'])
    want = []
    for r in csv.DictReader(open(cand, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE):
        if r['state'] in ('candidate', 'reserve'):
            want.append((ids[r['mpn']], r['mpn']))
    for m in CLIPS:
        want.append((clip_ids[m], m))
    todo = [w for w in want if w[1] not in done]
    for i, m in todo:
        print(f'{i}\t{m}')
    print(f'to-check: {len(want)} rows of the download set and clips, {len(todo)} unchecked', file=sys.stderr)


if __name__ == '__main__':
    main()
