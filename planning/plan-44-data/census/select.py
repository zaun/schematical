#!/usr/bin/env python3
"""plan-44 census: candidates.tsv from the raw listing.

usage: python3 -I select.py <raw dir> <out.tsv> [--urls url-check.tsv] [--stock stock-check.tsv] [--report]

Reads `<raw dir>/<category>-<maker>.jsonl` (list.js) and applies plan-44-A § 3:

  scope        (reason)
    overtone     Operating Mode names Overtone
    thermistor   Type names Thermistor
    saw          Type is SAW (resonators)
    mode         Operating Mode is not Fundamental (crystals)
    frequency    no frequency, or above 100 MHz
    status       status is not Active or Last Time Buy
    no-stock     stock 0
  duplicate      the same (maker, class, series, frequency, load capacitance,
                 tolerance, stability, ESR, temperature range, package,
                 supplier package, rating): the most-stocked part is kept
  quota          per maker 200 rows: up to 40 kHz crystals, up to 40 ceramic
                 resonators, the rest MHz crystals; inside a class the rows are
                 taken round-robin over the series (each series' most-stocked
                 row first, then each one's second, ...; the series with the
                 higher top stock first in a round). `candidate` = the first
                 quota rows, `reserve` = the next ceil(quota / 4) of the class
                 (the download set is candidate + reserve), `out` (reason
                 `below quota`) = the rest.
  no datasheet   with --urls: a row whose resolved maker-site URL (common.resolve_urls)
                 is not a PDF in url-check.tsv (urlcheck.js) is out
  stock check    with --stock: a row whose part number is `found` 0 in the file
                 is `out`, reason `stock check`, and the class refills from the
                 next rows in order.

Output (tab-separated, header first): mpn, maker, class (khz | mhz | resonator),
series, stock, status, hz, load_pf, tol_ppm, stab_ppm, esr_ohm, temp, mounting,
package, supplier_package, size, height, rating, features, datasheet (DigiKey's link), host,
url (the maker-site URL the download uses), via (direct | archive), cut (1 when the series piece was cut at 300), state (candidate | reserve | out),
reason, rank (position in its class's round-robin order, 1-based), then the
columns later letters fill: pdf, hit, dead, stock_checked.
"""
import collections
import csv
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

QUOTA = 200
KHZ_MAX = 40
RES_MAX = 40
RESERVE_DIV = 4
MAKERS = common.MAKERS
COLS = ['mpn', 'maker', 'class', 'series', 'stock', 'status', 'hz', 'load_pf', 'tol_ppm',
        'stab_ppm', 'esr_ohm', 'temp', 'mounting', 'package', 'supplier_package', 'size',
        'height', 'rating', 'features', 'datasheet', 'host', 'url', 'via', 'cut', 'state', 'reason',
        'rank', 'pdf', 'hit', 'dead', 'stock_checked']


def rank_class(rows):
    """Round-robin order over the series of one (maker, class)."""
    by = collections.defaultdict(list)
    for r in rows:
        by[r['series']].append(r)
    for v in by.values():
        v.sort(key=lambda r: (-r['stock'], r['mpn']))
    order = sorted(by, key=lambda s: (-by[s][0]['stock'], s))
    out = []
    for i in range(max((len(v) for v in by.values()), default=0)):
        for s in order:
            if i < len(by[s]):
                out.append(by[s][i])
    return out


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--') and not a.endswith('.tsv') or a in (sys.argv[1], sys.argv[2])]
    report = '--report' in sys.argv

    def opt(name):
        return sys.argv[sys.argv.index(name) + 1] if name in sys.argv else None

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
        r['state'], r['reason'], r['rank'] = 'out', common.scope(r), ''
        r['pdf'] = r['hit'] = r['dead'] = r['stock_checked'] = ''
        if r['mpn'] in stock:
            r['stock_checked'] = 1 if stock[r['mpn']] == '1' else 0
            if stock[r['mpn']] != '1' and not r['reason']:
                r['reason'] = 'stock check'
    common.dedupe(rows)
    common.resolve_urls(rows)
    if good is not None:
        for r in rows:
            if not r['reason'] and not good.get(r['url'], False):
                r['reason'] = 'no datasheet'
    for maker in MAKERS.values():
        elig = {c: rank_class([r for r in rows if r['maker'] == maker and r['class'] == c and not r['reason']])
                for c in ('khz', 'resonator', 'mhz')}
        q = {'khz': min(KHZ_MAX, len(elig['khz'])), 'resonator': min(RES_MAX, len(elig['resonator']))}
        q['mhz'] = min(len(elig['mhz']), QUOTA - q['khz'] - q['resonator'])
        for c in ('khz', 'resonator', 'mhz'):
            quota = q[c]
            reserve = math.ceil(quota / RESERVE_DIV)
            for i, r in enumerate(elig[c]):
                r['rank'] = i + 1
                if i < quota:
                    r['state'] = 'candidate'
                elif i < quota + reserve:
                    r['state'] = 'reserve'
                else:
                    r['reason'] = 'below quota'
    for r in rows:
        if r['state'] == 'out' and not r['reason']:
            r['reason'] = 'below quota'
    rows.sort(key=lambda r: (r['maker'], r['class'], r['state'] != 'candidate', r['state'] != 'reserve',
                             r['rank'] if r['rank'] != '' else 10 ** 9, r['mpn']))
    with open(sys.argv[2], 'w', newline='') as f:
        w = csv.writer(f, delimiter='\t', lineterminator='\n')
        w.writerow(COLS)
        for r in rows:
            w.writerow([r.get(c, '') for c in COLS])
    if report:
        print(f'{listed} parts listed')
        c = collections.Counter(r['reason'] for r in rows if r['state'] == 'out')
        print('out: ' + ', '.join(f'{k} {v}' for k, v in sorted(c.items(), key=lambda kv: -kv[1])))
        for maker in MAKERS.values():
            for cls in ('khz', 'mhz', 'resonator'):
                n = collections.Counter(r['state'] for r in rows if r['maker'] == maker and r['class'] == cls)
                if sum(n.values()):
                    print(f'{maker} {cls}: candidate {n["candidate"]}, reserve {n["reserve"]}, out {n["out"]}')
        cand = [r for r in rows if r['state'] == 'candidate']
        dl = [r for r in rows if r['state'] != 'out']
        print(f'candidates: {len(cand)}; from a cut piece: {sum(r["cut"] for r in cand)}')
        print(f'download set: {len(dl)} rows; {len({r["url"] for r in dl})} distinct URLs')


main()
