#!/usr/bin/env python3
"""plan-48-A phase 3: a verdict for every picked row (§ 4 of plan-48-A).
usage: python3 -I verdicts.py <candidates.tsv> <textscan.tsv> <verdicts.tsv>
Per row of state `candidate`, in this order:
  no PDF in the scan            -> out,  no datasheet
  hit 0 and scheme empty        -> skip, not in the datasheet's ordering table
  dead 1                        -> out,  datasheet status
  otherwise                     -> add
The file is a ledger: a row that left
`candidate` because of its verdict (select.py moved the next reserve row up)
keeps its line (an `add` line of a row that is no longer picked goes); a row in the scan is decided again. `verdict-overrides.tsv` (mpn, verdict, reason, why) is applied last: a row
the text rules get wrong, read by hand (a status word of the next row, a
revision-history line). Writes mpn, verdict, reason, pdf and prints the
counts."""
import collections
import csv
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
OVERRIDES = os.path.join(HERE, '..', 'verdict-overrides.tsv')


def overrides():
    if not os.path.exists(OVERRIDES):
        return {}
    return {r['mpn']: r for r in csv.DictReader(open(OVERRIDES, encoding='utf-8'), delimiter='\t')}


def main():
    cand, scan_path, out = sys.argv[1:4]
    scan = {r['mpn']: r for r in csv.DictReader(open(scan_path, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE)}
    manual = overrides()
    counts = collections.Counter()
    ledger = {}
    if os.path.exists(out):
        ledger = {r['mpn']: r for r in csv.DictReader(open(out, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE)}
    new = {}
    if True:
        for r in csv.DictReader(open(cand, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE):
            if r['state'] not in ('candidate', 'add'):
                continue
            s = scan.get(r['mpn'])
            if s is None:
                sys.exit('verdicts.py: no scan row for %s (run textscan.py again)' % r['mpn'])
            if s['hit'] == '':
                v, why = 'out', 'no datasheet'
            elif s['hit'] == '0' and not s['scheme']:
                v, why = 'skip', "not in the datasheet's ordering table"
            elif s['dead'] == '1':
                v, why = 'out', 'datasheet status'
            else:
                v, why = 'add', ''
            if r['mpn'] in manual:
                v, why = manual[r['mpn']]['verdict'], manual[r['mpn']]['reason']
            counts[(r['type'], r['company'], v, why)] += 1
            new[r['mpn']] = {'mpn': r['mpn'], 'verdict': v, 'reason': why, 'pdf': r['pdf']}
    ledger.update(new)
    # an `add` of a row that is no longer picked (its stock check failed)
    ledger = {m: v for m, v in ledger.items() if v['verdict'] != 'add' or m in new}
    for m, o in manual.items():
        if m in ledger:
            ledger[m].update(verdict=o['verdict'], reason=o['reason'])
    with open(out, 'w', encoding='utf-8', newline='') as f:
        f.write('mpn\tverdict\treason\tpdf\n')
        for m in sorted(ledger):
            f.write('\t'.join(ledger[m][c] for c in ('mpn', 'verdict', 'reason', 'pdf')) + '\n')
    for k in sorted(counts):
        print(counts[k], *k)


if __name__ == '__main__':
    main()
