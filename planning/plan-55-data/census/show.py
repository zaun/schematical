#!/usr/bin/env python3
"""plan-55: print the spec rows whose PDF path contains the text (for the hand checks of C, D, E).

usage: python3 -I show.py <text in the pdf path> [<text in the package labels>]
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

for r in common.read_tsv(common.SPECS):
    if sys.argv[1] in r['pdf'] and (len(sys.argv) < 3 or sys.argv[2] in r['package'] + r['supplier_package']):
        print(os.path.basename(r['pdf']), '|', r['supplier_package'], '| ordering', r['ordering'], '| type', r['type'], '| skip', r['skip'])
        print('  pins', r['pinout'])
        print('  switches', r['switches'], 'analog', r['analog'])
        print('  facts', {k: r[k] for k in ('vin_min', 'vin_max', 'iout', 'ilimit', 'ron', 'iq')})
        print('  note', r['note'][:200])
