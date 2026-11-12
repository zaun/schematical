#!/usr/bin/env python3
# plan-35-A Phase 3 check: every "skip" row of candidates.tsv appears exactly once
# (as a whole part name) in the plan-35 section of datasheets/skipped-parts.md.
#   usage: skipcheck.py <candidates.tsv> <skipped-parts.md>
import csv, re, sys
names = [r[1] for r in csv.reader(open(sys.argv[1]), delimiter='\t') if len(r) > 3 and r[3] == 'skip']
s = open(sys.argv[2]).read()
sec = s[s.index('## Character LCDs, 20x4 / 40x2 / 40x4 (plan-35)'):]
nxt = sec.find('\n## ', 5)
if nxt > 0:
    sec = sec[:nxt]
bad = 0
for n in names:
    c = len(re.findall(r'(?<![A-Za-z0-9#-])' + re.escape(n) + r'(?![A-Za-z0-9#-])', sec))
    if c != 1:
        print('BAD', n, c)
        bad += 1
print('%d skip names; %d found exactly once' % (len(names), len(names) - bad))
sys.exit(1 if bad else 0)
