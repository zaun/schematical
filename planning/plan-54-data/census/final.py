#!/usr/bin/env python3
"""plan-54-B/C/D Phase 4: the final pick.

usage: python3 -I final.py [--family F]... [--maker M]... [<candidates.tsv>]    set `state` add | skip, print counts
       python3 -I final.py [--family F]... [--maker M]... --write-skipped       write datasheets/skipped-parts.md
                                                                                `## Card slots (plan-54)`
       python3 -I final.py [--family F]... [--maker M]... --verify-skipped      every skip row of the selection is named there

Each live row (`state` candidate, add or skip) of the selection gets scheme-check's verdict: `ok` is `add`, `skip` is
`skip` with its reason; an `unchecked` row is an error (nothing is written). There is no quota (owner): every row that
passes is added. The skip rows are written per (family, maker, series, reason) group; the section holds all makers'
groups, so each run regenerates it from candidates.tsv whole (the selection only limits --verify-skipped).
"""
import collections
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs  # noqa: E402

SKIPPED = os.path.join(specs.ROOT, 'datasheets', 'skipped-parts.md')
SECTION = '## Card slots (plan-54)'
LEAF = {'card': 'Card slot → SD / microSD / SIM', 'module': 'Card slot → Memory module', 'edge': 'Card slot → Card edge'}


def select(argv):
    fams, makers, rest = [], [], []
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
    return fams, makers, (rest[0] if rest else specs.CANDIDATES)


def picked(r, fams, makers):
    return (not fams or r['family'] in fams) and (not makers or r['maker'] in makers)


def apply(fams, makers, path):
    rows = specs.candidates(path)
    cols = list(rows[0].keys())
    table = specs.load()
    counts = collections.Counter()
    verdicts = {}
    for r in rows:
        if r['state'] in ('candidate', 'add', 'skip') and picked(r, fams, makers):
            verdicts[r['mpn']] = specs.verdict(r, table.get(specs.key(r)))
    bad = [m for m, v in verdicts.items() if v[0] == 'unchecked']
    if bad:
        sys.exit(f'final.py: {len(bad)} rows unchecked, e.g. {bad[0]} ({verdicts[bad[0]][1]})')
    for r in rows:
        v = verdicts.get(r['mpn'])
        if not v:
            continue
        if v[0] == 'ok':
            r['state'], r['reason'] = 'add', ''
        else:
            r['state'], r['reason'] = 'skip', v[1]
        counts[(r['family'], r['maker'], r['state'])] += 1
    specs.write_rows(path, rows, cols)
    for (fam, maker, state), n in sorted(counts.items()):
        print(f'{fam} {maker} {state}: {n}')
    print('add', sum(n for k, n in counts.items() if k[2] == 'add'), 'skip', sum(n for k, n in counts.items() if k[2] == 'skip'))


def skip_groups(path, fams=(), makers=()):
    groups = collections.OrderedDict()
    for r in sorted((r for r in specs.candidates(path) if r['state'] == 'skip' and picked(r, fams, makers)),
                    key=lambda r: (r['family'], r['maker'], r['series'], r['reason'], r['mpn'])):
        groups.setdefault((r['family'], r['maker'], r['series'], r['reason'], r['pdf']), []).append(r['mpn'])
    return groups


def write_skipped(path):
    groups = skip_groups(path)
    lines = [SECTION, '',
             'Card-slot parts (memory / SIM card sockets, memory-module sockets, card-edge sockets; in stock and Active at',
             "DigiKey, plan-54-A) that the maker's datasheet does not list or that the library cannot hold, so no part is",
             'added. They are still made; a later plan may add them.', '',
             '| Part(s) | Device | Would have gone into | Why skipped |', '|---|---|---|---|']
    for (fam, maker, series, reason, pdf), items in groups.items():
        device = f"{maker} {series if series != '-' else ''}".strip() + f", `{os.path.basename(pdf)}`" if pdf else maker
        lines.append(f"| {', '.join(items)} | {device} | `connector` / Connector → {LEAF[fam]} | {reason} |")
    section = '\n'.join(lines) + '\n'
    text = open(SKIPPED).read()
    if SECTION in text:
        start = text.index(SECTION)
        rest = text[start + len(SECTION):]
        nxt = rest.find('\n## ')
        end = start + len(SECTION) + (nxt + 1 if nxt >= 0 else len(rest))
        text = text[:start] + section + ('\n' if nxt >= 0 else '') + text[end:]
    else:
        text = text.rstrip('\n') + '\n\n' + section
    open(SKIPPED, 'w').write(text)
    print(f'{sum(len(v) for v in groups.values())} skip rows in {len(groups)} groups written')


def verify_skipped(fams, makers, path):
    text = open(SKIPPED).read()
    if SECTION not in text:
        print('no plan-54 section in skipped-parts.md')
        return 1
    start = text.index(SECTION)
    rest = text[start:]
    nxt = rest.find('\n## ', len(SECTION))
    section = rest if nxt < 0 else rest[:nxt]
    names = {n for line in section.splitlines() if line.startswith('| ') for n in line.split('|')[1].replace(' ', '').split(',')}
    rows = [r for r in specs.candidates(path) if r['state'] == 'skip' and picked(r, fams, makers)]
    missing = [r['mpn'] for r in rows if r['mpn'] not in names]
    for m in missing:
        print('missing', m)
    print(f'{len(rows)} skip rows, {len(missing)} missing from skipped-parts.md')
    return 1 if missing else 0


def main():
    args = sys.argv[1:]
    fams, makers, path = select(args)
    if '--write-skipped' in args:
        write_skipped(path)
    elif '--verify-skipped' in args:
        sys.exit(verify_skipped(fams, makers, path))
    else:
        apply(fams, makers, path)


main()
