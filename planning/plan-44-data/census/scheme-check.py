#!/usr/bin/env python3
"""plan-44-B section 3.1 / 3.4: the ordering-code schemes and the datasheet
facts against the download-set rows.

usage: python3 -I scheme-check.py [--maker <maker>] [--list] [--sample] [--only-specs]
                                  [--write <verdicts.tsv>] [<candidates.tsv> [<specs.tsv>]]

The download set is every row whose `state` is not `out`. Per row:

  unchecked   its spec row is missing, or its `scheme`, `hz_range`, `loads`,
              `tols`, `stabs` or `esr_max` is empty (`*` = the datasheet gives
              none: not checked)
  skip        not printed verbatim in its PDF (`hit` 1) and not matched by its
              spec row's scheme: `not in the datasheet's ordering table`; or a
              fact (frequency, load capacitance, tolerance, stability, ESR) the
              datasheet's tables do not give: `fact not in datasheet`
  ok          otherwise

Prints per maker the rows with a verdict, then `<n> rows: <k> ok, <u> unchecked`
(exit 1 when something is unchecked). --list names each skip and unchecked row.
--sample prints 20 `ok` rows per maker (a fixed random pick, verbatim rows only
when a maker has fewer than 20 others) with their scheme, for a hand check
against the PDF; nothing else is printed. --write writes mpn, verdict, reason.
--only-specs checks only the rows whose spec row is in the given specs file (a
fragment of specs.tsv filled by one reader), so a partial file has no
`unchecked` rows.
"""
import collections
import csv
import os
import random
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402
import specs  # noqa: E402

NOT_TABLE = "not in the datasheet's ordering table"
FACT = 'fact not in datasheet'
FILLED = ('scheme', 'hz_range', 'loads', 'tols', 'stabs', 'esr_max')


def verdict(row, spec):
    """(verdict, reason, detail)"""
    if spec is None:
        return 'unchecked', 'no spec row', ''
    for col in FILLED:
        if not spec.get(col):
            return 'unchecked', f'spec row has no {col}', ''
    if not specs.in_table(row, spec):
        return 'skip', NOT_TABLE, spec['scheme']
    problems = specs.fact_problems(row, spec)
    if problems:
        return 'skip', FACT, '; '.join(problems)
    return 'ok', '', ''


def main(argv):
    maker = None
    listing = '--list' in argv
    sample = '--sample' in argv
    only = '--only-specs' in argv
    out = None
    argv = [a for a in argv if a not in ('--list', '--sample', '--only-specs')]
    for flag in ('--maker', '--write'):
        if flag in argv:
            i = argv.index(flag)
            value = argv[i + 1]
            argv = argv[:i] + argv[i + 2:]
            if flag == '--maker':
                maker = value
            else:
                out = value
    cand = argv[0] if argv else specs.CANDIDATES
    table = specs.load(argv[1] if len(argv) > 1 else specs.SPECS)
    counts = collections.defaultdict(collections.Counter)
    oks = collections.defaultdict(list)
    results = []
    for r in csv.DictReader(open(cand), delimiter='\t'):
        if not common.eligible(r) or (maker and r['maker'] != maker):
            continue
        spec = table.get(specs.key(r))
        if only and spec is None:
            continue
        v, why, detail = verdict(r, spec)
        counts[r['maker']][v] += 1
        counts[r['maker']]['rows'] += 1
        results.append((r['mpn'], v, why))
        if v == 'ok':
            oks[r['maker']].append((r, spec))
        elif listing:
            print(v, r['mpn'], '|', r['series'], '|', why, '|', detail)
    if sample:
        for m in sorted(oks):
            pool = [x for x in oks[m] if x[0]['hit'] != '1'] or oks[m]
            pool = sorted(pool, key=lambda x: x[0]['mpn'])
            for r, s in random.Random(44).sample(pool, min(20, len(pool))):
                print(f"{m} | {r['mpn']} | {r['pdf']} | {s['scheme']}")
        return 0
    if out:
        with open(out, 'w') as f:
            f.write('mpn\tverdict\treason\n')
            for mpn, v, why in results:
                f.write(f'{mpn}\t{v}\t{why}\n')
    total = ok = unchecked = 0
    for m in sorted(counts):
        c = counts[m]
        print(f"{m}: {c['rows']} rows; {c['ok']} ok, {c['skip']} skip, {c['unchecked']} unchecked")
        total += c['rows']
        ok += c['ok']
        unchecked += c['unchecked']
    print(f'{total} rows: {ok} ok, {unchecked} unchecked')
    return 1 if unchecked else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
