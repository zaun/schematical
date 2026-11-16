#!/usr/bin/env python3
"""plan-45 census: candidates.tsv from the raw listing.

usage: python3 -I select.py <raw dir> <out.tsv> [--urls url-check.tsv] [--stock stock-check.tsv] [--report]

Reads `<raw dir>/<piece>.jsonl*` (list.js) and applies plan-45-A § 3:

  scope        (reason)
    status       status is not Active or Last Time Buy
    no-stock     stock 0
    positions    fewer than 2 or more than 12 positions per level
    levels       a stacked (2..4 level) terminal block
    pitch        not 3.50, 3.81, 5.00, 5.08, 7.50 or 7.62 mm (terminal blocks)
    mounting     press-fit, SKEDD, panel, DIN rail, free hanging
    termination  not a screw, spring-cage or push-in clamp (IDC, quick connect)
  duplicate      the same (maker, series, pitch, positions, orientation, mounting, class,
                 current, voltage, type, shrouding): the most-stocked row is kept
  no datasheet   no maker-site URL with a file to download (common.resolve_urls), or with --urls
                 a URL that url-check.tsv lists and that is not a PDF (a URL it does not list,
                 because its row was not a candidate in the first pass, is not judged)
  stock check    with --stock: a part number `found` 0 in the file is out
  variant        the footprint family of a row (common.variant). Terminal blocks: per (maker,
                 pitch) the top fixed-block variant (category 371) and the top pluggable header
                 variant (category 370) among those with at least 4 in-scope rows, by the
                 summed stock of their in-scope rows;
                 every in-scope row of a chosen variant is a `candidate`. JST: every variant
                 with at least 2 in-scope rows. The rest are out, reason `below quota`.

Output (tab-separated, header first): see COLS; the last four columns (pdf, hit, dead,
stock_checked) are filled by plan-45-A.
"""
import collections
import csv
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

K_FIXED = 1
K_PLUGGABLE = 1
MIN_ROWS = 4
COLS = ['mpn', 'maker', 'group', 'category', 'series', 'class', 'pitch', 'positions', 'orientation',
        'mounting', 'amps', 'volts', 'type', 'shroud', 'stock', 'status', 'datasheet', 'host', 'url',
        'src', 'variant', 'state', 'reason', 'pdf', 'hit', 'dead', 'stock_checked']


def opt(name):
    return sys.argv[sys.argv.index(name) + 1] if name in sys.argv else None


def vname(v):
    maker, group, series, pitch, orient, mount, cls, typ, shroud = v
    bits = [common.SHORT[maker], series.replace(' ', '-'), f'{pitch:.2f}' if pitch else '', orient[:1].upper(), mount]
    if cls:
        bits.append(cls)
    if typ:
        bits.append(typ.replace(' ', '').replace(',', '-')[:24])
    return '|'.join(b for b in bits if b)


def main():
    stock = {}
    if opt('--stock'):
        for i, line in enumerate(open(opt('--stock'))):
            f = line.rstrip('\n').split('\t')
            if i and len(f) >= 2:
                stock[f[0]] = f[1]
    good = None
    if opt('--urls'):
        good = {}
        for i, line in enumerate(open(opt('--urls'))):
            f = line.rstrip('\n').split('\t')
            if i:
                good[f[0]] = f[3] == '1'
    rows = common.read_raw(sys.argv[1])
    listed = len(rows)
    for r in rows:
        r['state'], r['reason'] = 'out', common.scope(r)
        r['pdf'] = r['hit'] = r['dead'] = r['stock_checked'] = ''
        if r['mpn'] in stock:
            r['stock_checked'] = 1 if stock[r['mpn']] == '1' else 0
            if stock[r['mpn']] != '1' and not r['reason']:
                r['reason'] = 'stock check'
    common.dedupe(rows)
    common.resolve_urls(rows)
    for r in rows:
        r['variant'] = vname(common.variant(r))
        if not r['reason'] and not r['src']:
            r['reason'] = 'no datasheet'
        if good is not None and not r['reason'] and r['url'] in good and not good[r['url']]:
            r['reason'] = 'no datasheet'
    elig = collections.defaultdict(list)
    for r in rows:
        if not r['reason']:
            elig[common.variant(r)].append(r)
    chosen = set()
    by_slot = collections.defaultdict(list)
    for v, rs in elig.items():
        maker, group, series, pitch, orient, mount, cls, typ, shroud = v
        if group == 'jst':
            if len(rs) >= 2:
                chosen.add(v)
            continue
        if len(rs) < MIN_ROWS:
            continue
        slot = (maker, pitch, 'pluggable' if cls == 'pluggable' else 'fixed')
        by_slot[slot].append((sum(r['stock'] for r in rs), v))
    for slot, vs in by_slot.items():
        vs.sort(key=lambda t: (-t[0], t[1]))
        for _, v in vs[:K_PLUGGABLE if slot[2] == 'pluggable' else K_FIXED]:
            chosen.add(v)
    for r in rows:
        if r['reason']:
            continue
        if common.variant(r) in chosen:
            r['state'] = 'candidate'
        else:
            r['reason'] = 'below quota'
    rows.sort(key=lambda r: (r['maker'], r['state'] != 'candidate', r['variant'], r['positions'], r['mpn']))
    with open(sys.argv[2], 'w', newline='') as f:
        w = csv.writer(f, delimiter='\t', lineterminator='\n')
        w.writerow(COLS)
        for r in rows:
            w.writerow([r.get(c, '') for c in COLS])
    if '--report' in sys.argv:
        print(f'{listed} parts listed')
        c = collections.Counter(r['reason'] for r in rows if r['state'] == 'out')
        print('out: ' + ', '.join(f'{k} {v}' for k, v in sorted(c.items(), key=lambda kv: -kv[1])))
        for maker in common.MAKERS.values():
            cand = [r for r in rows if r['maker'] == maker and r['state'] == 'candidate']
            print(f'{maker}: candidates {len(cand)}, variants {len({r["variant"] for r in cand})}, '
                  f'series {len({r["series"] for r in cand})}')
        cand = [r for r in rows if r['state'] == 'candidate']
        print(f'candidates: {len(cand)}; distinct URLs {len({r["url"] for r in cand})}; '
              f'variants {len({r["variant"] for r in cand})}')


main()
