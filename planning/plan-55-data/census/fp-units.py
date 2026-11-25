#!/usr/bin/env python3
"""plan-55-F/G: the land patterns the `add` rows need.

usage: python3 -I fp-units.py [<letter>]              one line per unit (letter F or G, or both)
       python3 -I fp-units.py --packets <dir> <n>     split the units into packets of at most <n> for the footprint readers

A unit is a (letter, company, DigiKey `Package / Case`, `Supplier Device Package`, pin count) of the `add` rows' spec
rows (verdicts.tsv `spec`, specs.tsv `pinout`): the footprint must have the pinout's pin count (a numbered exposed pad
that is a pin counts) and is drawn from the package drawing of one of the unit's PDFs. Lines: letter, company, package,
supplier_package, pins, add rows, the PDFs (space separated, most `add` rows first), the most stocked example part.
"""
import collections
import csv
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402


def units():
    specs = collections.defaultdict(list)
    for s in common.read_tsv(common.SPECS):
        specs[common.spec_key(s)].append(s)
    cand = {(r['family'], r['mpn']): r for r in common.read_tsv(common.CANDIDATES)}
    out = collections.OrderedDict()
    for v in common.read_tsv(common.VERDICTS):
        if v['verdict'] != 'add':
            continue
        c = cand[(v['family'], v['mpn'])]
        _, _, k = v['spec'].rpartition('#')
        spec = specs[common.spec_key(c)][int(k)]
        pins = len(json.loads(spec['pinout']))
        u = ('F' if v['company'] == 'ti' else 'G', v['company'], c['package'], c['supplier_package'], pins)
        e = out.setdefault(u, {'n': 0, 'best': c, 'pdfs': collections.Counter()})
        e['n'] += 1
        e['pdfs'][c['pdf']] += 1
        if int(c['stock'] or 0) > int(e['best']['stock'] or 0):
            e['best'] = c
    return out


def main():
    a = sys.argv[1:]
    us = units()
    if a and a[0] == '--packets':
        d, n = a[1], int(a[2])
        os.makedirs(d, exist_ok=True)
        packets, cur, key = [], [], None
        for u in sorted(us, key=lambda u: (u[0], u[1], u[2], u[3], u[4])):
            if cur and (u[0] != key[0] or u[1] != key[1] or len(cur) >= n):
                packets.append(cur)
                cur = []
            cur.append(u)
            key = u
        if cur:
            packets.append(cur)
        for i, items in enumerate(packets, 1):
            pd = os.path.join(d, f'f{i:02d}')
            os.makedirs(pd, exist_ok=True)
            with open(os.path.join(pd, 'assign.tsv'), 'w') as fh:
                w = csv.writer(fh, delimiter='\t', lineterminator='\n')
                w.writerow(['letter', 'company', 'package', 'supplier_package', 'pins', 'add_rows', 'pdfs', 'texts', 'example'])
                for u in items:
                    e = us[u]
                    pdfs = [p for p, _ in e['pdfs'].most_common()]
                    texts = ['/tmp/p55/text/' + p[len('datasheets/powerswitch/'):][:-4] + '.txt' for p in pdfs]
                    w.writerow(list(u) [:5] + [e['n'], ' '.join(pdfs), ' '.join(texts), e['best']['mpn']])
            print(f'f{i:02d}: {items[0][0]} {items[0][1]} {len(items)} units')
        return
    letter = a[0] if a else None
    for u, e in us.items():
        if letter is None or u[0] == letter:
            print('\t'.join(list(u[:4]) + [str(u[4]), str(e['n']), ' '.join(p for p, _ in e['pdfs'].most_common()), e['best']['mpn']]))


main()
