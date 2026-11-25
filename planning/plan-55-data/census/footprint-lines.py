#!/usr/bin/env python3
"""plan-55-E § 3.1: footprints-needed.txt from the `add` rows.

usage: python3 -I footprint-lines.py > footprints-needed.txt

One tab-separated line per distinct (company, DigiKey `Package / Case`, `Supplier Device Package`) of the `add`
rows of verdicts.tsv (candidates.tsv gives the package labels): letter (`F` for `ti`, the plan-55-F letter, `G` for
every other company, plan-55-G), company, package, supplier package, the number of `add` rows (column 5), the example
part (the most stocked), its pdf, the families it covers, the pin count of the spec row(s) and the number of spec rows.
A line starting with `#` is the header.
"""
import collections
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

cand = {(r['family'], r['mpn']): r for r in common.read_tsv(common.CANDIDATES)}
groups = collections.OrderedDict()
for v in common.read_tsv(common.VERDICTS):
    if v['verdict'] != 'add':
        continue
    c = cand[(v['family'], v['mpn'])]
    key = (v['company'], c['package'], c['supplier_package'])
    g = groups.setdefault(key, {'n': 0, 'best': c, 'families': set(), 'specs': set()})
    g['n'] += 1
    g['families'].add(v['family'])
    g['specs'].add(v['spec'])
    if int(c['stock'] or 0) > int(g['best']['stock'] or 0):
        g['best'] = c
print('# letter\tcompany\tpackage\tsupplier package\tadd rows\texample part\texample pdf\tfamilies\tspec rows')
for key in sorted(groups, key=lambda k: ('F' if k[0] == 'ti' else 'G',) + k):
    g = groups[key]
    print('\t'.join(['F' if key[0] == 'ti' else 'G'] + list(key) + [str(g['n']), g['best']['mpn'], g['best']['pdf'],
                                                                    ','.join(sorted(g['families'])), str(len(g['specs']))]))
