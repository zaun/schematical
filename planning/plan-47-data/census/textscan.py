#!/usr/bin/env python3
"""plan-47-A Phase 3: each download-set row's own datasheet, and whether it prints the row's part number.

usage: python3 -I textscan.py <candidates.tsv> <pdf-dir> <out.tsv>

For each download-set row whose `url` downloaded as a PDF: `pdf` = that URL
(place.py turns it into the file's path in the tree), `hit` = 1 when the part
number stands verbatim between token boundaries in the `pdftotext -layout` text
(cached as <md5>.txt in <pdf-dir>), `dead` = 1 when an obsolete / discontinued /
NRND / end-of-life word stands within 300 characters of it. Other rows get
empty `pdf`, `hit` and `dead` (sibling.py may give them a PDF; a row outside the
download set keeps them empty).
"""
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import scancommon as sc  # noqa: E402


def main(tsv, directory, out_path):
    rows = sc.read(tsv)
    by_url = {}
    for r in rows:
        r['pdf'], r['hit'], r['dead'] = '', '', ''
        if not sc.in_set(r):
            continue
        r['hit'], r['dead'] = '0', '0'
        if r['url']:
            by_url.setdefault(r['url'], []).append(r)
    for url, group in by_url.items():
        text = sc.text_of(directory, url)
        if text is None:
            continue
        body = sc.norm(text)
        for r in group:
            r['pdf'] = url
            r['hit'], r['dead'] = sc.scan(body, r['mpn'])
    sc.write(out_path, rows)
    print(sum(1 for r in rows if r['pdf']), 'rows with their own PDF;',
          sum(1 for r in rows if r['hit'] == '1'), 'printed verbatim;',
          sum(1 for r in rows if r['dead'] == '1'), 'dead')


if __name__ == '__main__':
    main(*sys.argv[1:4])
