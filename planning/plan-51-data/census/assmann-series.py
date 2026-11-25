#!/usr/bin/env python3
"""plan-51-C: gives every Assmann candidate the series of its drawing (column 3 `-` -> `ASS_<n>`, from the
`ASS_<n>_CO.pdf` of column 11), because DigiKey lists all Assmann sockets under the series `-` while each
drawing is a socket line of its own with its own PCB layout (A xx-LC-TT hole 1.00, AR xx-HZL hole 0.90,
AR xx-HZW/TN hole 1.02, A-CCS Z-SM, A-CCS Z-T, A-CCS G-T). Run again over a changed file: it does nothing."""
import os, re

TSV = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'candidates.tsv')
lines = [l.rstrip('\n').split('\t') for l in open(TSV)]
changed = 0
for r in lines[1:]:
    if r[0] == 'Assmann':
        series = re.search(r'(ASS_\d+)_CO\.pdf$', r[10]).group(1)
        if r[2] != series:
            r[2] = series
            changed += 1
with open(TSV, 'w') as f:
    f.write('\n'.join('\t'.join(r) for r in lines) + '\n')
print(changed, 'Assmann rows changed')
