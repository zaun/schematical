#!/usr/bin/env python3
"""plan-54-B/C/D Phase 3: every specs row has a pinout the connector kind will accept.

usage: python3 -I pinout-check.py [--family F]... [--maker M]... [--list] [<specs.tsv> [<candidates.tsv>]]

Per spec line with a `pinout`:
  - it is `seq`, `AB` or an explicit `pad=NAME,...` list; an explicit list has unique pad labels, names unique except
    NC / GND / VSS / SHIELD (as the USB parts), at least 2 pins;
  - `AB` needs an even pin count;
  - for every candidate row that passes scheme-check (`ok`) and belongs to the line: the pin count the pinout gives
    (an explicit list: its length; `seq`/`AB`: DigiKey's positions) equals the row's pin count (specs.pin_count: DigiKey's
    positions, +1 for a SHIELD pin DigiKey leaves out); a row that differs is listed with both numbers.
A line whose `pinout` is empty counts as bad. Prints `<n> rows, <b> bad` (exit 1 when b > 0); --list prints each bad
line and each differing candidate row.
"""
import collections
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs  # noqa: E402


def main(argv):
    fams, makers, rest = [], [], []
    listing = '--list' in argv
    i = 0
    while i < len(argv):
        a = argv[i]
        if a == '--family':
            fams.append(argv[i + 1]); i += 2; continue
        if a == '--maker':
            makers.append(argv[i + 1]); i += 2; continue
        if not a.startswith('--'):
            rest.append(a)
        i += 1
    path = rest[0] if rest else specs.SPECS
    cand = rest[1] if len(rest) > 1 else specs.CANDIDATES
    lines = [s for s in specs.read_rows(path) if (not fams or s['family'] in fams) and (not makers or s['maker'] in makers)]
    table = specs.load(path)
    by_line = collections.defaultdict(list)
    for r in specs.candidates(cand):
        if r['state'] not in ('candidate', 'add'):
            continue
        sp = table.get(specs.key(r))
        if not sp:
            continue
        v, _, _, spec = specs.verdict(r, sp)
        if v == 'ok':
            by_line[id_of(spec)].append(r)
    bad = 0
    for s in lines:
        problems = []
        po = s['pinout']
        if not po:
            problems.append('empty pinout')
        elif po not in ('seq', 'AB'):
            e = specs.explicit_pins(po)
            if e is None:
                problems.append('neither seq, AB nor pad=NAME list')
            else:
                pads = [p for p, _ in e]
                if len(set(pads)) != len(pads):
                    problems.append('repeated pad label')
                if len(e) < 2:
                    problems.append('fewer than 2 pins')
                names = collections.Counter(n for _, n in e)
                for n, c in names.items():
                    if (not n) or (c > 1 and n not in specs.SHARED):
                        problems.append(f'name {n!r} x{c}')
        diffs = []
        for r in by_line.get(id_of(s), []):
            n = specs.pin_count(r, s)
            if po == 'AB' and n % 2:
                diffs.append(f"{r['mpn']}: AB with {n} pins")
            elif po not in ('seq', 'AB') and specs.explicit_pins(po) is not None and len(specs.explicit_pins(po)) != n:
                diffs.append(f"{r['mpn']}: pinout has {len(specs.explicit_pins(po))} pins, DigiKey {r['positions']}")
        if problems or diffs:
            bad += 1
            if listing:
                print('BAD', s['family'], s['maker'], s['series'], s['pdf'], '|', '; '.join(problems), '|', '; '.join(diffs[:6]), f'({len(diffs)} rows differ)' if diffs else '')
    print(f'{len(lines)} rows, {bad} bad')
    return 1 if bad else 0


def id_of(s):
    return specs.key(s) + (s['scheme'],)


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
