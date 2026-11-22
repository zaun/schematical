#!/usr/bin/env python3
"""plan-44-B: the download-set rows of each spec row of a specs file.

usage: python3 -I spec-rows.py [<specs.tsv> [<candidates.tsv>]]

For every spec row of the file (a fragment of specs.tsv is fine), prints a
header line `== maker | series | pdf | package` and one line per download-set
row of that group: part number, hz, load_pf, tol_ppm, stab_ppm, esr_ohm, the
DigiKey supplier package, `hit` (1: the part number is printed verbatim in
the PDF), class. The values are DigiKey's; the datasheet is the authority.
"""
import csv
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402
import specs  # noqa: E402


def main(argv):
    path = argv[0] if argv else specs.SPECS
    cand = argv[1] if len(argv) > 1 else specs.CANDIDATES
    order = [specs.key(s) for s in csv.DictReader(open(path), delimiter='\t')]
    groups = {k: [] for k in order}
    for r in csv.DictReader(open(cand), delimiter='\t'):
        if common.eligible(r) and specs.key(r) in groups:
            groups[specs.key(r)].append(r)
    for k in order:
        print('== ' + ' | '.join(k))
        for r in sorted(groups[k], key=lambda r: r['mpn']):
            print('\t'.join([r['mpn'], f"hz={r['hz']}", f"load={r['load_pf']}", f"tol={r['tol_ppm']}",
                             f"stab={r['stab_ppm']}", f"esr={r['esr_ohm']}", r['supplier_package'],
                             f"hit={r['hit']}", r['class']]))


main(sys.argv[1:])
