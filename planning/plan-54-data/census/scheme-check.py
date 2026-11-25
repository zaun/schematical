#!/usr/bin/env python3
"""plan-54-B/C/D: the ordering-code schemes, the facts and the status against the candidate rows.

usage: python3 -I scheme-check.py [--family F]... [--maker M]... [--list] [--sample] [--only-specs]
                                  [--write <verdicts.tsv>] [<candidates.tsv> [<specs.tsv>]]

The rows checked are those whose `state` is `candidate` (a row final.py already made `add` or `skip` is checked too).
Per row (specs.verdict):

  unchecked   its spec row is missing, or a column of a spec row of its key is empty, or two schemes match it
  skip        not printed verbatim in its PDF (`hit` 1) and not matched by a scheme: `not in the datasheet's ordering
              table`; the status says `obsolete` or `footprint-skip:<feature>`; or DigiKey's positions or pitch is not
              in the datasheet's: `fact not in datasheet`
  ok          otherwise

Prints per (family, maker) the rows with a verdict, then `<n> rows: <k> ok, <u> unchecked` (exit 1 when something is
unchecked). --list names each skip and unchecked row. --sample prints 20 `ok` rows per maker (a fixed random pick,
rows not printed verbatim first) with their scheme, for a hand check against the PDF; nothing else is printed.
--write writes mpn, verdict, reason. --only-specs checks only the rows whose key is in the given specs file (a
fragment filled by one reader), so a partial file has no `unchecked` rows for the other keys.
"""
import collections
import os
import random
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs  # noqa: E402


def main(argv):
    fams, makers = [], []
    listing = '--list' in argv
    sample = '--sample' in argv
    only = '--only-specs' in argv
    out = None
    rest = []
    i = 0
    while i < len(argv):
        a = argv[i]
        if a == '--family':
            fams.append(argv[i + 1])
            i += 2
            continue
        if a == '--maker':
            makers.append(argv[i + 1])
            i += 2
            continue
        if a == '--write':
            out = argv[i + 1]
            i += 2
            continue
        if not a.startswith('--'):
            rest.append(a)
        i += 1
    cand = rest[0] if rest else specs.CANDIDATES
    table = specs.load(rest[1] if len(rest) > 1 else specs.SPECS)
    counts = collections.defaultdict(collections.Counter)
    oks = collections.defaultdict(list)
    results = []
    for r in specs.candidates(cand):
        if r['state'] not in ('candidate', 'add', 'skip'):
            continue
        if (fams and r['family'] not in fams) or (makers and r['maker'] not in makers):
            continue
        sp = table.get(specs.key(r))
        if only and sp is None:
            continue
        v, why, detail, spec = specs.verdict(r, sp)
        group = f"{r['family']} {r['maker']}"
        counts[group][v] += 1
        counts[group]['rows'] += 1
        results.append((r['mpn'], v, why))
        if v == 'ok':
            oks[group].append((r, spec))
        elif listing:
            print(v, r['mpn'], '|', r['series'], '|', why, '|', detail)
    if sample:
        for g in sorted(oks):
            pool = [x for x in oks[g] if x[0]['hit'] != '1'] or oks[g]
            pool = sorted(pool, key=lambda x: x[0]['mpn'])
            for r, s in random.Random(54).sample(pool, min(20, len(pool))):
                print(f"{g} | {r['mpn']} | {r['pdf']} | {s['scheme']} | pos={r['positions']}")
        return 0
    if out:
        with open(out, 'w') as f:
            f.write('mpn\tverdict\treason\n')
            for mpn, v, why in results:
                f.write(f'{mpn}\t{v}\t{why}\n')
    total = ok = unchecked = 0
    for g in sorted(counts):
        c = counts[g]
        print(f"{g}: {c['rows']} rows; {c['ok']} ok, {c['skip']} skip, {c['unchecked']} unchecked")
        total += c['rows']
        ok += c['ok']
        unchecked += c['unchecked']
    print(f'{total} rows: {ok} ok, {unchecked} unchecked')
    return 1 if unchecked else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
