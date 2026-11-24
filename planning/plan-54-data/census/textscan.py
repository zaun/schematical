#!/usr/bin/env python3
"""plan-54-A Phase 3: each candidate row's own datasheet, and whether it prints the row's part number.

usage: python3 -I textscan.py candidates.tsv <pdf dir>      (rewrites candidates.tsv)

For each `candidate` row whose `src` downloaded as a PDF (fetch.py: <pdf dir>/<md5 of src>.pdf): `pdf` = that `src`
(place.py turns it into the file's path in the tree), `hit` = 1 when the part number stands verbatim between token
boundaries in the `pdftotext -layout` text (cached as <md5>.txt in <pdf dir>), `dead` = 1 when an obsolete /
discontinued / NRND / "not recommended for new designs" / last-time-buy / end-of-life word stands within 300
characters of it. A candidate with no PDF keeps `pdf`, `hit` and `dead` empty; a row that is not a candidate is
untouched.
"""
import csv
import hashlib
import os
import re
import subprocess
import sys

DEAD = re.compile(r'OBSOLETE|DISCONTINU|NOT RECOMMENDED FOR NEW|NRND|LAST TIME BUY|END OF LIFE|(?<![A-Z0-9])EOL(?![A-Z0-9])')


def md5(s):
    return hashlib.md5(s.encode()).hexdigest()


def text_of(directory, src):
    pdf = os.path.join(directory, md5(src) + '.pdf')
    if not os.path.exists(pdf):
        return None
    txt = pdf[:-4] + '.txt'
    if not os.path.exists(txt):
        subprocess.run(['pdftotext', '-q', '-layout', pdf, txt], check=False, timeout=300)
    if not os.path.exists(txt):
        return None
    return re.sub(r'[ \t]+', ' ', open(txt, errors='replace').read().upper())


def scan(body, mpn):
    key = mpn.upper()
    m = re.search(r'(?<![A-Z0-9])' + re.escape(key) + r'(?![A-Z0-9])', body)
    if not m:
        return '0', '0'
    return '1', '1' if DEAD.search(body[max(0, m.start() - 300):m.end() + 300]) else '0'


def main():
    tsv, directory = sys.argv[1], sys.argv[2]
    rows = list(csv.DictReader(open(tsv), delimiter='\t'))
    cols = list(rows[0].keys())
    cache = {}
    for r in rows:
        if r['state'] != 'candidate':
            continue
        r['pdf'], r['hit'], r['dead'] = '', '', ''
        if not r['src']:
            continue
        if r['src'] not in cache:
            cache[r['src']] = text_of(directory, r['src'])
        if cache[r['src']] is not None:
            r['pdf'] = r['src']
            r['hit'], r['dead'] = scan(cache[r['src']], r['mpn'])
    with open(tsv, 'w', newline='') as out:
        w = csv.DictWriter(out, cols, delimiter='\t', lineterminator='\n')
        w.writeheader()
        w.writerows(rows)
    c = [r for r in rows if r['state'] == 'candidate']
    print(sum(1 for r in c if r['pdf']), 'candidate rows with their own PDF of', len(c), ';',
          sum(1 for r in c if r['hit'] == '1'), 'printed verbatim;', sum(1 for r in c if r['dead'] == '1'), 'dead')


main()
