#!/usr/bin/env python3
"""plan-54 census: candidates.tsv from the listing.

usage: python3 -I select.py raw candidates.tsv [--urls url-check.tsv] [--stock stock-check.tsv] [--report]
One row per listed part. `state` is `candidate` or `out`, `reason` says why. Rules (plan-54-A § 3), in order:
  maker        the maker is not one of the family's three (`common.MAKERS`)
  no stock     the listing gives no quantity (DigiKey's QuantityAvailable is absent or 0)
  no facts     the part has no Number of Positions (a part DigiKey gives no parameters)
  male         a plug (Gender Male, or a card type that "fits female edgecards"): the library wants slots
  duplicate    the MPN is another listed MPN plus `-TR` (the same part on tape and reel)
  no datasheet the part has no usable maker datasheet: no link, no maker URL (`common.src_for`), or, with
               --urls, the download source does not answer a PDF
  stock check  with --stock (recheck.py's file): a row still `candidate` whose `found` is 0 (not in stock and Active
               in the fresh listing); every row the file names gets `stock_checked` = its date
The last columns (pdf, hit, dead, stock_checked) are filled by plan-54-A.
"""
import collections
import csv
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

COLS = ['mpn', 'maker', 'maker_id', 'family', 'series', 'stock', 'status', 'positions', 'pitch_mm', 'rows',
        'mounting', 'termination', 'card_type', 'gender', 'features', 'variant', 'datasheet', 'url', 'src',
        'state', 'reason', 'pdf', 'hit', 'dead', 'stock_checked']


def variant(r, family):
    p = r['params']
    if family == 'edge':
        return '|'.join([r['maker'], r['series'], p.get('Mounting Type', ''), p.get('Pitch', ''), p.get('Termination', ''), p.get('Number of Rows', '')])
    if family == 'module':
        return '|'.join([r['maker'], r['series'], p.get('Connector Style', ''), p.get('Mounting Type', ''), p.get('Insertion Angle', '')])
    return '|'.join([r['maker'], r['series'], p.get('Card Type', ''), p.get('Mounting Type', ''), p.get('Insertion, Removal Method', '')])


def main():
    raw, out = sys.argv[1], sys.argv[2]
    urls = {}
    if '--urls' in sys.argv:
        for row in csv.DictReader(open(sys.argv[sys.argv.index('--urls') + 1]), delimiter='\t'):
            urls[row['src']] = row['ok'] == '1'
    stock = {}
    if '--stock' in sys.argv:
        for row in csv.DictReader(open(sys.argv[sys.argv.index('--stock') + 1]), delimiter='\t'):
            stock[row['mpn']] = (row['found'] == '1', row['date'])
    listing = common.read_listing(raw)
    mpns = {(r['category'], r['mpn']) for r in listing}
    rows = []
    for r in listing:
        p = r['params']
        family = common.FAMILY[r['category']]
        maker = common.MAKER[r['maker']]
        link = common.norm_url(r['datasheet'])
        url, src, why = common.src_for(maker, link)
        card = p.get('Card Type') or p.get('Connector Style') or ''
        state, reason = 'candidate', ''
        if maker not in common.MAKERS[family]:
            state, reason = 'out', 'maker'
        elif (r.get('stock') or 0) <= 0:
            state, reason = 'out', 'no stock'
        elif common.first_int(p.get('Number of Positions')) == 0:
            state, reason = 'out', 'no facts'
        elif p.get('Gender') == 'Male' or 'Fits Female' in card:
            state, reason = 'out', 'male'
        elif r['mpn'].endswith('-TR') and (r['category'], r['mpn'][:-3]) in mpns:
            state, reason = 'out', 'duplicate'
        elif not url or (urls and not urls.get(src, False)):
            state, reason = 'out', 'no datasheet'
        checked = stock.get(r['mpn'])
        if checked and state == 'candidate' and not checked[0]:
            state, reason = 'out', 'stock check'
        rows.append({
            'mpn': r['mpn'], 'maker': maker, 'maker_id': r['makerId'], 'family': family, 'series': r['series'],
            'stock': r.get('stock', ''), 'status': r.get('status', ''), 'positions': common.first_int(p.get('Number of Positions')),
            'pitch_mm': common.pitch_mm(p.get('Pitch', '')), 'rows': p.get('Number of Rows', ''),
            'mounting': p.get('Mounting Type', ''), 'termination': p.get('Termination', ''), 'card_type': card,
            'gender': p.get('Gender', ''), 'features': p.get('Features', ''), 'variant': variant(r, family),
            'datasheet': link, 'url': url, 'src': src, 'state': state, 'reason': reason,
            'pdf': '', 'hit': '', 'dead': '', 'stock_checked': checked[1] if checked else ''})
    with open(out, 'w', newline='') as fh:
        w = csv.DictWriter(fh, COLS, delimiter='\t', lineterminator='\n')
        w.writeheader()
        w.writerows(rows)
    if '--report' in sys.argv:
        print(f'{len(rows)} parts listed')
        cand = [x for x in rows if x['state'] == 'candidate']
        print(f'candidates: {len(cand)}; distinct URLs {len({x["src"] for x in cand})}')
        print('out:', ', '.join(f'{k} {v}' for k, v in collections.Counter(x['reason'] for x in rows if x['state'] == "out").most_common()))
        for fam in ('card', 'module', 'edge'):
            for mk in sorted({x['maker'] for x in rows if x['family'] == fam}):
                a = [x for x in rows if x['family'] == fam and x['maker'] == mk]
                c = [x for x in a if x['state'] == 'candidate']
                print(f'{fam:6} {mk:18} listed {len(a):4} candidates {len(c):4} variants {len({x["variant"] for x in c}):3} (variant,positions) {len({(x["variant"], x["positions"]) for x in c}):4}')


main()
