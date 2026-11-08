#!/usr/bin/env python3
"""plan-24-A/B Phase 2 checks for one group: rows, undecided, add rows without a
footprint id, in-scope-interface violations.  usage: checks.py <GROUP>"""
import csv, os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
g = sys.argv[1]
cand = {r['name']: r for r in csv.DictReader(open(os.path.join(DATA, 'candidates.tsv')), delimiter='\t')}
rows = [r for r in csv.DictReader(open(os.path.join(DATA, 'parts.tsv')), delimiter='\t') if r['group'] == g]
und = [r for r in rows if r['decision'] not in ('add', 'skip') or (r['decision'] == 'skip' and not r['reason'])]
nofp = [r for r in rows if r['decision'] == 'add' and not r['footprint']]
bad_if = [r for r in rows if r['decision'] == 'add' and g != 'RTC'
          and cand[r['name']]['interface'] not in ('spi', 'i2c', 'parallel', 'parallel+spi', 'i2c+spi')]
print(f'{g}: {len(rows)} rows (candidates {sum(1 for c in cand.values() if c["group"] == g)}); '
      f'undecided {len(und)}; add without footprint {len(nofp)}; highspeed/other added {len(bad_if)}')
