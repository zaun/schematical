#!/usr/bin/env python3
"""plan-54-B/C/D: the candidate rows of each spec row of a specs file.

usage: python3 -I spec-rows.py [<specs.tsv> [<candidates.tsv>]]

For every distinct key of the file (a fragment of specs.tsv is fine), prints a header line
`== family | maker | series | pdf` and one line per candidate row (state not `out`) of that key: part number, DigiKey's
positions, pitch, rows, mounting, card type, `hit` (1: the part number is printed verbatim in the PDF) and the
`variant`. The values are DigiKey's; the datasheet is the authority.
"""
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs  # noqa: E402


def main(argv):
    path = argv[0] if argv else specs.SPECS
    cand = argv[1] if len(argv) > 1 else specs.CANDIDATES
    order = []
    for r in specs.read_rows(path):
        if specs.key(r) not in order:
            order.append(specs.key(r))
    groups = {k: [] for k in order}
    for r in specs.candidates(cand):
        k = specs.key(r)
        if r['state'] in ('candidate', 'add', 'skip') and k in groups:
            groups[k].append(r)
    for k in order:
        print('== ' + ' | '.join(k))
        for r in sorted(groups[k], key=lambda r: r['mpn']):
            print('\t'.join([r['mpn'], f"pos={r['positions']}", f"pitch={r['pitch_mm']}", f"rows={r['rows']}",
                             r['mounting'], r['card_type'], f"hit={r['hit']}", r['variant']]))


main(sys.argv[1:])
