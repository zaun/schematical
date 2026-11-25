#!/usr/bin/env python3
"""plan-51-C: corrects the mounting of the six Mill-Max 210-47-...-001000 candidates, which DigiKey lists as
Surface Mount (210-47-316-41-001000, the same series and suffix, as Through Hole). Mill-Max p.12 of 088-145.pdf
(file page 12) names the -001000 suffix "standard solder tail" (through hole); the gull-wing surface-mount version
of the series is -105000 (p.18). The row keeps state `candidate`; the reason column records the correction.
Running it again changes nothing."""
import os

TSV = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'candidates.tsv')
lines = [l.rstrip('\n').split('\t') for l in open(TSV)]
n = 0
for r in lines[1:]:
    if r[0] == 'Mill-Max' and r[1].startswith('210-47-') and r[1].endswith('-41-001000') and r[6] == 'Surface Mount':
        r[6] = 'Through Hole'
        r[12] = ('mounting corrected from DigiKey Surface Mount: the -001000 suffix is the standard solder tail '
                 '(088-145.pdf p.12); the SMT version is -105000 (p.18)')
        n += 1
with open(TSV, 'w') as f:
    f.write('\n'.join('\t'.join(r) for r in lines) + '\n')
print(n, 'rows corrected')
