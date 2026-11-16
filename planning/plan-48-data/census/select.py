#!/usr/bin/env python3
"""plan-48 census: candidates.tsv from the listing.
usage: select.py <raw dir> <candidates.tsv> --urls <url-check.tsv>
                 [--pdf <cache dir>] [--stock <stock-check.tsv>]
                 [--verdicts <verdicts.tsv>] [--report]
Every listed row gets a state:
  out        with its reason (legacy label, maker, no stock figure, status,
             military, hi-rel package, type, no datasheet; later: stock
             check, and plan-48-A's verdict reasons)
  candidate  timers: every in-scope row. op-amps: the QUOTA (200)
             most-stocked in-scope rows of each maker
  reserve    the in-scope op-amp rows after them, in stock order
--pdf fills `hit`: 1 when the part number stands in the text of its cached
PDF (pdftotext -layout, spaces ignored), 0 when not, empty when no PDF.
--stock (plan-48-A phase 1) marks rows that failed the part-by-part check
out; --verdicts (phase 3: mpn, verdict, reason) marks rows `skip`/`obsolete`
out; in both cases the next reserve row of that maker moves up."""
import collections
import csv
import hashlib
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

COLS = ['mpn', 'company', 'type', 'class', 'circuits', 'status', 'stock', 'package', 'supplier_package',
        'mounting', 'grade', 'supply_min', 'supply_max', 'output', 'piece', 'cut', 'maker', 'description',
        'via', 'url', 'final', 'state', 'reason', 'rank', 'pdf', 'hit', 'stock_checked']


def arg(name):
    return sys.argv[sys.argv.index(name) + 1] if name in sys.argv else None


def text_of(pdf, cache):
    txt = pdf[:-4] + '.txt'
    if not os.path.exists(txt):
        subprocess.run(['pdftotext', '-layout', pdf, txt], check=False, stderr=subprocess.DEVNULL)
    try:
        return ''.join(open(txt, encoding='utf-8', errors='replace').read().split()).upper()
    except OSError:
        return ''


def main():
    raw, out = sys.argv[1], sys.argv[2]
    urls = {}
    for r in csv.DictReader(open(arg('--urls'), encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE):
        urls[r['url']] = r
    cache = arg('--pdf')
    stock = {}
    if arg('--stock'):
        for r in csv.DictReader(open(arg('--stock'), encoding='utf-8'), delimiter='\t'):
            stock[r['mpn']] = r
    verdicts = {}
    if arg('--verdicts'):
        for r in csv.DictReader(open(arg('--verdicts'), encoding='utf-8'), delimiter='\t'):
            verdicts[r['mpn']] = r
    rows, seen, texts = [], set(), {}
    for r in common.read_raw(raw):
        if r['mpn'] in seen:
            continue
        seen.add(r['mpn'])
        p = r['params']
        reason = common.classify(r)
        via, url = ('', '') if reason else common.resolve(r)
        u = urls.get(url)
        if not reason and not (u and u['ok'] == '1'):
            reason = 'no datasheet'
        row = {'mpn': r['mpn'], 'company': r['company'], 'type': r['type'], 'class': r['class'],
               'circuits': r['circuits'], 'status': r['status'], 'stock': r.get('stock', ''),
               'package': p.get('Package / Case', ''), 'supplier_package': p.get('Supplier Device Package', ''),
               'mounting': p.get('Mounting Type', ''),
               'grade': 'AEC-Q100' if p.get('Grade') == 'Automotive' or r['mpn'].endswith('Q1') else '',
               'supply_min': p.get('Voltage - Supply Span (Min)', '') or p.get('Voltage - Supply', ''),
               'supply_max': p.get('Voltage - Supply Span (Max)', ''), 'output': p.get('Output Type', ''),
               'piece': r['piece'], 'cut': r['cut'], 'maker': r['maker'], 'description': r['description'],
               'via': via, 'url': url, 'final': u['final'] if u and not reason else '',
               'state': 'out' if reason else '', 'reason': reason, 'rank': '', 'pdf': '', 'hit': '',
               'stock_checked': ''}
        if not reason and cache:
            pdf = os.path.join(cache, hashlib.md5(url.encode()).hexdigest() + '.pdf')
            if os.path.exists(pdf):
                row['pdf'] = os.path.basename(pdf)
                if pdf not in texts:
                    texts[pdf] = text_of(pdf, cache)
                row['hit'] = int(''.join(r['mpn'].split()).upper() in texts[pdf])
        s = stock.get(r['mpn'])
        if s and not reason:
            row['stock_checked'] = s['checked']
            if not (s['found'] == '1' and int(s['stock'] or 0) > 0 and s['status'] in ('Active', 'Last Time Buy')):
                row['state'], row['reason'] = 'out', 'stock check'
        v = verdicts.get(r['mpn'])
        if v and row['state'] != 'out' and v['verdict'] != 'add':
            row['state'], row['reason'] = 'out', v['reason']
        rows.append(row)
    rows.sort(key=lambda x: (x['type'], x['company'], -int(x['stock'] or 0), x['mpn']))
    rank = collections.Counter()
    for row in rows:
        if row['state'] == 'out':
            continue
        k = (row['type'], row['company'])
        rank[k] += 1
        row['rank'] = rank[k]
        row['state'] = 'candidate' if row['type'] == 'timer' or rank[k] <= common.QUOTA else 'reserve'
    with open(out, 'w', encoding='utf-8', newline='') as f:
        w = csv.DictWriter(f, COLS, delimiter='\t', lineterminator='\n', quoting=csv.QUOTE_NONE, quotechar=None)
        w.writeheader()
        w.writerows(rows)
    if '--report' in sys.argv:
        print('%d parts listed' % len(rows))
        c = collections.Counter((r['type'], r['company'] or '-', r['state'], r['reason']) for r in rows)
        for k in sorted(c):
            print('  %-6s %-8s %-9s %-16s %d' % (k + (c[k],)))
        ds = [r for r in rows if r['state'] in ('candidate', 'reserve')]
        print('download set: %d rows; %d distinct URLs; %d distinct final PDFs; from a cut piece %d' % (
            len(ds), len({r['url'] for r in ds}), len({r['final'] for r in ds}), sum(int(r['cut']) for r in ds)))
        for st in ('candidate', 'reserve'):
            h = collections.Counter((r['type'], r['company'], str(r['hit'])) for r in rows if r['state'] == st)
            print('%s literal hits: %s' % (st, sorted(h.items())))


if __name__ == '__main__':
    main()
