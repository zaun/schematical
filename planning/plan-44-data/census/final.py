#!/usr/bin/env python3
"""plan-44-B Phase 4: the final pick.

usage: python3 -I final.py [<candidates.tsv>]       set `state` add | skip | out on the download set, print the shortfalls
       python3 -I final.py --write-skipped          write datasheets/skipped-parts.md `## Crystals and resonators (plan-44)`
       python3 -I final.py --verify-skipped         every skip row is named in that section

The live rows are the download set that still has a chance: `state` candidate,
reserve, add or skip, and an `out` row with reason `below quota` that has a
`pdf` (a row final.py left out earlier); `out` for obsolete / stock check /
the rest stay out. Each live row gets scheme-check.py's verdict: `skip` rows
(reason `not in the datasheet's ordering table` or `fact not in datasheet`)
are `skip`. Per (maker, class) the passing rows in rank order (candidate rows
come first by rank, then reserve rows) become `add` up to the quota
(`quota.tsv`, A's per-class counts); the rest are `out`, reason `below quota`.
The footprints plan-44-F's generic parts choose among are pinned: the
most-stocked passing row of each pinned label (footprint-lines.py `label()`
with the datasheet's pad count) is `add` first and counts against its (maker,
class) quota; a pinned label with no passing row is an error.
"""
import collections
import csv
import importlib.util
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402
import specs  # noqa: E402


def load(name):
    spec = importlib.util.spec_from_file_location(name.replace('-', '_'), os.path.join(common.HERE, name + '.py'))
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


schemecheck = load('scheme-check')
footprint = load('footprint-lines')

QUOTA = os.path.join(common.DATA, 'quota.tsv')
SKIPPED = os.path.join(common.ROOT, 'datasheets', 'skipped-parts.md')
SECTION = '## Crystals and resonators (plan-44)'
PINNED = ['XTAL-3215-2', 'XTAL-2012-2', 'XTAL-1610-2', 'XTAL-CYL-2X6-2',    # 32.768 kHz
          'XTAL-HC49US-2', 'XTAL-3225-2', 'XTAL-5032-2', 'XTAL-6035-2',     # 8 and 16 MHz
          'CRES-3731-3', 'CRES-6030-3', 'CRES-3213-3', 'CRES-2520-3']       # the resonator
CLASS_NAME = {'khz': 'kHz', 'mhz': 'MHz', 'resonator': 'Ceramic resonator'}


def read(path):
    with open(path) as f:
        return list(csv.reader(f, delimiter='\t'))


def live(r):
    return r['state'] in ('candidate', 'reserve', 'add', 'skip') or (
        r['state'] == 'out' and r['reason'] == 'below quota' and r['pdf'])


def rank(r):
    return int(r['rank']) if r['rank'] else 10 ** 9


def apply(path):
    data = read(path)
    head = data[0]
    rows = [dict(zip(head, r)) for r in data[1:]]
    table = specs.load()
    pads = footprint.pads_of(table)
    quota = {(r['maker'], r['class']): int(r['quota']) for r in csv.DictReader(open(QUOTA), delimiter='\t')}
    verdicts = {}
    for r in rows:
        if live(r):
            verdicts[r['mpn']] = schemecheck.verdict(r, table.get(specs.key(r)))
    unchecked = [m for m, v in verdicts.items() if v[0] == 'unchecked']
    if unchecked:
        sys.exit(f'final.py: {len(unchecked)} rows unchecked, e.g. {unchecked[0]}')
    passing = collections.defaultdict(list)
    for r in rows:
        if r['mpn'] in verdicts and verdicts[r['mpn']][0] == 'ok':
            passing[(r['maker'], r['class'])].append(r)
    for k in passing:
        passing[k].sort(key=lambda r: (rank(r), r['mpn']))
    # pinned footprints: the most-stocked passing row of each pinned label, across makers
    by_label = collections.defaultdict(list)
    for k, rs in passing.items():
        for r in rs:
            by_label[footprint.label(r, pads(r))].append(r)
    pinned = {}
    missing = []
    for lab in PINNED:
        if not by_label[lab]:
            missing.append(lab)
            continue
        best = max(by_label[lab], key=lambda r: (int(r['stock']), r['mpn']))
        pinned[lab] = best['mpn']
    if missing:
        print('PINNED LABELS WITH NO PASSING ROW:', ', '.join(missing))
    chosen = set(pinned.values())
    short = {}
    for k, rs in sorted(passing.items()):
        q = quota.get(k, 0)
        mine = [r for r in rs if r['mpn'] in chosen]
        room = q - len(mine)
        for r in rs:
            if room <= 0:
                break
            if r['mpn'] not in chosen:
                chosen.add(r['mpn'])
                mine.append(r)
                room -= 1
        short[k] = (q, len(mine))
    counts = collections.Counter()
    for r in rows:
        if r['mpn'] not in verdicts:
            continue
        v, why, _ = verdicts[r['mpn']]
        if v == 'skip':
            r['state'], r['reason'] = 'skip', why
        elif r['mpn'] in chosen:
            r['state'], r['reason'] = 'add', ''
        else:
            r['state'], r['reason'] = 'out', 'below quota'
        counts[r['state']] += 1
    with open(path, 'w', newline='') as f:
        w = csv.writer(f, delimiter='\t', lineterminator='\n')
        w.writerow(head)
        for r in rows:
            w.writerow([r[c] for c in head])
    print(f"add {counts['add']}, skip {counts['skip']}, out (below quota) {counts['out']}")
    for (maker, cls), (q, n) in sorted(short.items()):
        print(f"{maker} {cls}: quota {q}, add {n}" + (f", SHORT by {q - n}" if n < q else ''))
    print('pinned: ' + ', '.join(f'{lab}={pinned.get(lab, "MISSING")}' for lab in PINNED))


def skip_rows(path):
    data = read(path)
    head = data[0]
    return [dict(zip(head, r)) for r in data[1:] if r[head.index('state')] == 'skip']


def write_skipped(path):
    table = specs.load()
    groups = collections.OrderedDict()
    for r in sorted(skip_rows(path), key=lambda r: (r['maker'], r['class'], r['series'], r['reason'], r['mpn'])):
        s = table[specs.key(r)]
        detail = ''
        v, why, d = schemecheck.verdict(r, s)
        if why == schemecheck.FACT:
            detail = ' (' + d + ')'
        groups.setdefault((r['maker'], r['series'], r['class'], r['package'], r['pdf'], r['reason']), []).append((r['mpn'], detail))
    lines = [SECTION, '',
             'Download-set parts (in stock, Active or Last Time Buy at DigiKey, plan-44-A) whose maker datasheet',
             "does not list them, so no part is added. They are still made; a later plan may add them with the",
             "datasheet's other options. Their PDFs may be in the tree for the parts that were added.", '',
             '| Part(s) | Device | Would have gone into | Why skipped |', '|---|---|---|---|']
    for (maker, series, cls, package, pdf, reason), items in groups.items():
        parts = ', '.join(m for m, _ in items)
        why = reason + ''.join(f' [{m}{d}]' for m, d in items if d)
        device = f"{maker} {series}, {package}, `{os.path.basename(pdf)}`"
        lines.append(f"| {parts} | {device} | `crystal` / Crystal → {CLASS_NAME[cls]} | {why} |")
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


def verify_skipped(path):
    text = open(SKIPPED).read()
    if SECTION not in text:
        print('no plan-44 section in skipped-parts.md')
        return 1
    start = text.index(SECTION)
    rest = text[start:]
    nxt = rest.find('\n## ', len(SECTION))
    section = rest if nxt < 0 else rest[:nxt]
    rows = skip_rows(path)
    missing = [r['mpn'] for r in rows if r['mpn'] not in section]
    for m in missing:
        print('missing', m)
    print(f'{len(rows)} skip rows, {len(missing)} missing from skipped-parts.md')
    return 1 if missing else 0


def main():
    args = sys.argv[1:]
    path = next((a for a in args if a.endswith('.tsv')), specs.CANDIDATES)
    if '--write-skipped' in args:
        write_skipped(path)
    elif '--verify-skipped' in args:
        sys.exit(verify_skipped(path))
    else:
        apply(path)


if __name__ == '__main__':
    main()
