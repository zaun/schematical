#!/usr/bin/env python3
"""plan-45-B Phase 2: the ordering schemes of the series-level Wurth sheets, into specs.tsv.

usage: python3 -I series-schemes.py <specs.tsv>

Most Wurth, Phoenix and Weidmuller PDFs are per-item: the part number is printed in them (`hit` 1). Two
Wurth sheets cover a whole series and print the numbering rule in the title block instead
(`pdftotext -layout`, 2026-11-24):

  691311400102.pdf  (series 3114, 7.62 mm close vertical PCB header, 1 page)    "WERI PART NO: 691 311 400 1xx"
  691412120002B.pdf (series 412 B, 3.5 mm screwless 45° entry, 2 pages)         "WERI PART NO: 691 412 120 0xx B"
                                                                                 and "A = (3.5 x NB PINS) + 1.5"

`xx` is the number of pins (the 13 rows' DigiKey `positions` agree: scheme-check.py's `n` check). One
file serves several rows (same SHA-256 under several URLs, datasheets.tsv). A row keeps its scheme
when it has one.
"""
import csv
import sys

SCHEMES = {
    '691311400102.pdf': (r'6913114001(?P<n>\d\d)',
                         'series sheet, title block "WERI PART NO: 691 311 400 1xx", xx = pins (p1)'),
    '691412120002B.pdf': (r'6914121200(?P<n>\d\d)B',
                          'series sheet, title block "WERI PART NO: 691 412 120 0xx B", xx = pins; A = (3.5 x NB PINS) + 1.5 (p1)'),
}


def main():
    path = sys.argv[1]
    rows = list(csv.DictReader(open(path), delimiter='\t'))
    cols = list(rows[0].keys())
    done = 0
    for r in rows:
        name = r['pdf'].split('/')[-1]
        if r['maker'] == 'Wurth Elektronik' and name in SCHEMES and not r['scheme']:
            r['scheme'], r['note'] = SCHEMES[name]
            done += 1
    with open(path, 'w') as out:
        out.write('\t'.join(cols) + '\n')
        for r in rows:
            out.write('\t'.join(r[c] for c in cols) + '\n')
    print(f'{done} spec rows filled')


main()
