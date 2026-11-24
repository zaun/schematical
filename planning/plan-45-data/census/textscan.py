#!/usr/bin/env python3
"""plan-45-A Phase 3: does each candidate row's datasheet print its part number, and say it is dead?

usage: python3 -I textscan.py <candidates.tsv> <txt-dir>        (rewrites candidates.tsv)

For every `candidate` row with a placed PDF (`pdf`, the repo path place.py wrote): the text of the PDF
(`pdftotext -layout`, cached in <txt-dir>/<sha1 of the path>.txt) is cut into pages at the form feeds.
`hit` = 1 when the part number stands verbatim, between token boundaries (case-insensitive, whitespace
folded), on some page; `dead` = 1 when a page that names the part also holds an obsolescence word:
obsolete, discontinued, not recommended for new design, end of life, EOL, last time buy, NRND.
A row with no placed PDF keeps empty `hit` and `dead`. The JST series PDFs print a numbering table, not
the part: their rows are 0 here and plan-45-B reads them against the ordering scheme.
Prints, per maker, the rows with `hit` 1 and `dead` 1, and the `dead` rows (B reads every one).
"""
import collections
import csv
import hashlib
import os
import re
import subprocess
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..'))
# "discontinued" / "discontinuation", not the verb: every Wurth datasheet says "we reserve the right to
# discontinue production" (82 of its 95 candidate rows matched on that sentence alone, 2026-11-24).
DEAD = re.compile(r'OBSOLETE|DISCONTINUED|DISCONTINUATION|NOT RECOMMENDED FOR NEW|END OF LIFE|\bEOL\b|LAST TIME BUY|\bNRND\b', re.I)


def pages_of(txt_dir, rel):
    cache = os.path.join(txt_dir, hashlib.sha1(rel.encode()).hexdigest() + '.txt')
    if not os.path.exists(cache):
        subprocess.run(['pdftotext', '-q', '-layout', os.path.join(ROOT, rel), cache], check=False, timeout=300)
    if not os.path.exists(cache):
        return None
    return [re.sub(r'\s+', ' ', p).upper() for p in open(cache, errors='replace').read().split('\f')]


def scan(pages, mpn):
    key = re.sub(r'\s+', ' ', mpn).strip().upper()
    if not key:
        return '0', '0'
    pat = re.compile(r'(?<![A-Z0-9])' + re.escape(key) + r'(?![A-Z0-9])')
    hit = dead = False
    for page in pages:
        if pat.search(page):
            hit = True
            if DEAD.search(page):
                dead = True
    return '1' if hit else '0', '1' if dead else '0'


def main():
    cand, txt_dir = sys.argv[1], sys.argv[2]
    os.makedirs(txt_dir, exist_ok=True)
    rows = list(csv.DictReader(open(cand), delimiter='\t'))
    cols = list(rows[0].keys())
    cache = {}
    for r in rows:
        r['hit'] = r['dead'] = ''
        if r['state'] not in ('candidate', 'add') or not r['pdf']:
            continue
        if r['pdf'] not in cache:
            cache[r['pdf']] = pages_of(txt_dir, r['pdf'])
        pages = cache[r['pdf']]
        r['hit'], r['dead'] = ('0', '0') if pages is None else scan(pages, r['mpn'])
    with open(cand, 'w') as out:
        out.write('\t'.join(cols) + '\n')
        for r in rows:
            out.write('\t'.join(str(r[c]) for c in cols) + '\n')
    n = collections.Counter()
    for r in rows:
        if r['hit'] != '':
            n[(r['maker'], 'rows')] += 1
            n[(r['maker'], 'hit')] += r['hit'] == '1'
            n[(r['maker'], 'dead')] += r['dead'] == '1'
    for maker in sorted({k[0] for k in n}):
        print(f"{maker}: {n[(maker, 'rows')]} rows, {n[(maker, 'hit')]} printed verbatim, {n[(maker, 'dead')]} dead")
    for r in rows:
        if r['dead'] == '1':
            print('dead', r['mpn'], r['pdf'])


main()
