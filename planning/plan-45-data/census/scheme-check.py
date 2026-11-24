#!/usr/bin/env python3
"""plan-45-B Phase 1: every JST candidate row is covered by exactly one ordering scheme.

usage: python3 -I scheme-check.py <specs.tsv> <candidates.tsv> [--list]

The schemes are the `scheme` column of the JST rows of specs.tsv (one per (JST PDF, variant); the group
`n` is the position count). For every JST `candidate` row (state `candidate`, or `add` once B has
decided):

  unmatched   no scheme (of any JST spec row) full-matches its part number
  ambiguous   the schemes of more than one variant match (one variant's scheme read in two PDFs is
              one scheme: the SH/SR sheets repeat each other)
  wrong spec  none of the matching schemes is the one of the row's own (variant, PDF)
  positions   `n` of the match is not the row's `positions` (DigiKey's Number of Positions)
  unread      `hit` is 0 (not printed verbatim) and the spec row has no `note` naming the rule that makes
              the number (the reading of the numbering table: jst-schemes.py)

Prints `jst: <rows> rows, <u> unmatched, <a> ambiguous` and the other counts; exit 1 when any is not 0.
--list names the rows.
"""
import csv
import re
import sys


def main():
    specs_path, cand_path = sys.argv[1], sys.argv[2]
    listing = '--list' in sys.argv
    specs = [r for r in csv.DictReader(open(specs_path), delimiter='\t') if r['maker'] == 'JST']
    schemes = {}
    for s in specs:
        if not s['scheme']:
            sys.exit(f"spec row without a scheme: {s['variant']} {s['pdf']}")
        schemes[(s['variant'], s['pdf'])] = (re.compile(s['scheme']), s)
    counts = {'rows': 0, 'unmatched': 0, 'ambiguous': 0, 'wrong spec': 0, 'positions': 0, 'unread': 0}
    bad = []
    for r in csv.DictReader(open(cand_path), delimiter='\t'):
        if r['maker'] != 'JST' or r['state'] not in ('candidate', 'add'):
            continue
        counts['rows'] += 1
        hits = [(k, m) for k, (rx, _) in schemes.items() for m in [rx.fullmatch(r['mpn'])] if m]
        if not hits:
            counts['unmatched'] += 1
            bad.append(('unmatched', r['mpn'], ''))
            continue
        # The same variant documented in two PDFs (the SH/SR sheets eSH, eSR_SZ and eSR_SZ-new print the
        # same numbering) is one scheme read twice, not an ambiguity; two variants matching is.
        if len({k[0] for k, _ in hits}) > 1:
            counts['ambiguous'] += 1
            bad.append(('ambiguous', r['mpn'], ' '.join(sorted({k[0] for k, _ in hits}))))
            continue
        own = [(k, m) for k, m in hits if k == (r['variant'], r['pdf'])]
        if not own:
            counts['wrong spec'] += 1
            bad.append(('wrong spec', r['mpn'], ' '.join(k[1] for k, _ in hits)))
            continue
        (variant, pdf), m = own[0]
        if int(m.group('n')) != int(r['positions']):
            counts['positions'] += 1
            bad.append(('positions', r['mpn'], f"{m.group('n')} vs {r['positions']}"))
        if r['hit'] != '1' and not schemes[(variant, pdf)][1]['note']:
            counts['unread'] += 1
            bad.append(('unread', r['mpn'], pdf))
    if listing:
        for b in bad:
            print(*b)
    print(f"jst: {counts['rows']} rows, {counts['unmatched']} unmatched, {counts['ambiguous']} ambiguous"
          f"; {counts['wrong spec']} wrong spec, {counts['positions']} position mismatches, {counts['unread']} unread")
    sys.exit(1 if any(v for k, v in counts.items() if k != 'rows') else 0)


main()
