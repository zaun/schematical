#!/usr/bin/env python3
"""plan-45-B Phase 2: the verdicts of the terminal-block rows (Phoenix Contact, Weidmuller, Wurth).

usage: python3 -I verdicts.py <candidates.tsv> [<specs.tsv>] [--check] [--list]

Per row of a terminal maker whose `state` is `candidate` (a row already `add` or `skip` is re-judged the
same way, so the script can be run again):

  in the ordering table   `hit` 1 (its part number is printed in its own PDF: a per-item sheet is its own
                          ordering table), or the part number full-matches the scheme of its spec row
                          (specs.tsv: a series-level sheet prints the numbering rule, series-schemes.py)
                          with `n` equal to the row's `positions`; else `skip`, reason
                          `not in the datasheet's ordering table`
  status                  `dead` 1: the PDF names the part with an obsolescence word. Each such row needs a
                          reading in READINGS below (`keep` or `skip`, with the reason); one without is
                          `undecided`. (2026-11-24: no row has `dead` 1.)

`state` becomes `add` or `skip` (with `reason`). --check changes nothing: it prints
`terminal: <rows> rows: <add> add, <skip> skip, <undecided> undecided` and exits 1 when something is
undecided. --list names the skip and undecided rows. JST rows are plan-45-B Phase 1's (scheme-check.py).
"""
import csv
import re
import sys

TERMINAL = ('Phoenix Contact', 'Weidmuller', 'Wurth Elektronik')
NOT_TABLE = "not in the datasheet's ordering table"
OBSOLETE = 'obsolete in the datasheet'
# part number -> ('keep' | 'skip', what the datasheet says): the reading of every `dead` 1 row.
READINGS = {}


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    check, listing = '--check' in sys.argv, '--list' in sys.argv
    cand = args[0]
    specs_path = args[1] if len(args) > 1 else cand.replace('candidates.tsv', 'specs.tsv')
    schemes = {}
    for s in csv.DictReader(open(specs_path), delimiter='\t'):
        if s['scheme'] and s['maker'] in TERMINAL:
            schemes[(s['maker'], s['variant'], s['pdf'])] = re.compile(s['scheme'])
    rows = list(csv.DictReader(open(cand), delimiter='\t'))
    cols = list(rows[0].keys())
    count = {'add': 0, 'skip': 0, 'undecided': 0}
    total = 0
    for r in rows:
        if r['maker'] not in TERMINAL or r['state'] not in ('candidate', 'add', 'skip') or not r['pdf']:
            continue
        total += 1
        state, reason = 'add', ''
        rx = schemes.get((r['maker'], r['variant'], r['pdf']))
        m = rx.fullmatch(r['mpn']) if rx else None
        if r['hit'] != '1' and not (m and int(m.group('n')) == int(r['positions'])):
            state, reason = 'skip', NOT_TABLE
        elif r['dead'] == '1':
            reading = READINGS.get(r['mpn'])
            if reading is None:
                state, reason = 'candidate', 'undecided'
            elif reading[0] == 'skip':
                state, reason = 'skip', OBSOLETE
        count['undecided' if state == 'candidate' else state] += 1
        if listing and state != 'add':
            print(state, r['mpn'], r['pdf'], reason)
        if not check and state != 'candidate':
            r['state'], r['reason'] = state, reason
    if not check:
        with open(cand, 'w') as out:
            out.write('\t'.join(cols) + '\n')
            for r in rows:
                out.write('\t'.join(r[c] for c in cols) + '\n')
    print(f"terminal: {total} rows: {count['add']} add, {count['skip']} skip, {count['undecided']} undecided")
    sys.exit(1 if count['undecided'] else 0)


main()
