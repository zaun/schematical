#!/usr/bin/env python3
"""plan-55 census: candidates.tsv from the listing (plan-55-B § 3.2).

usage: python3 -I select.py <raw dir> <candidates.tsv> [--carry old.tsv] [--urls url-check.tsv]
                            [--stock stock-check.tsv] [--report]

One row per listed part: `family`, `company`, `mpn`, `series`, `stock`, `status`, `package` (DigiKey `Package /
Case`), `supplier_package`, `grade` (DigiKey `Grade`: `Automotive` or empty), `datasheet_url` (DigiKey's link),
`maker_url` (the maker's document, common.alternatives; with --urls the first alternative that answered a PDF,
else the first), `state` (`candidate` or `out`), `reason`, `pdf`, `hit`, `dead`, `stock_checked`.
Rules, in order (the first that applies is the reason):
  duplicate    the (family, mpn) was listed already under another maker label
  maker        the company is not one of the family's three (common.MAKERS)
  no stock     the listing gives no quantity; not active: the status is not Active
  stock check  (--stock) the recheck listing did not find the part again
A row with no maker URL stays a candidate with an empty `maker_url`: plan-55-B Phase 3 (sibling.py) looks for
its part number in a same-company PDF, and a row still without a PDF is then `out`, reason `no datasheet`.
--carry keeps `pdf`, `hit`, `dead` and `stock_checked` of an earlier candidates.tsv (and its `out` rows of reason
`no datasheet` and `stock check`), so a rewrite loses no later phase's work (plan-42-J Correction 1).
--stock: stock-check.tsv from stock.py (the recheck listing): a part found again gets `stock_checked` 1.
--report prints the counts next to the census's `pick` counts (families.out).
"""
import collections
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

COLS = ['family', 'company', 'mpn', 'series', 'stock', 'status', 'package', 'supplier_package', 'grade',
        'datasheet_url', 'maker_url', 'state', 'reason', 'pdf', 'hit', 'dead', 'stock_checked']
CARRY = ['pdf', 'hit', 'dead', 'stock_checked']
CARRY_OUT = ('no datasheet', 'stock check')


def pick_sums():
    """The planning census's part counts per family (families.out `pick` lines)."""
    sums = collections.Counter()
    per = {}
    path = os.path.join(common.DATA, 'families.out')
    for line in open(path):
        f = line.rstrip('\n').split('\t')
        if f[0] == 'pick':
            n = int(f[3].split()[1])
            sums[f[1]] += n
            per[(f[1], f[2])] = n
    return sums, per


def main():
    args = sys.argv[1:]
    raw, out = args[0], args[1]

    def opt(name):
        return args[args.index(name) + 1] if name in args else None

    old = {}
    if opt('--carry') and os.path.exists(opt('--carry')):
        for r in common.read_tsv(opt('--carry')):
            old[(r['family'], r['mpn'])] = r
    ok = {}
    if opt('--urls'):
        for r in common.read_tsv(opt('--urls')):
            ok[r['url']] = r['ok'] == '1'
    stock = {}
    if opt('--stock'):
        for r in common.read_tsv(opt('--stock')):
            stock[(r['family'], r['mpn'])] = r
    rows = []
    for r in common.read_listing(raw):
        p = r['params']
        company = common.COMPANY[r['makerId']]
        link = common.norm_url(r['datasheet'])
        alts = common.alternatives(company, r['mpn'], link)
        url = ''
        if alts:
            url = next((u for u in alts if ok.get(u)), '') if ok else alts[0]
        key = (r['family'], r['mpn'])
        state, reason = 'candidate', ''
        if r['dup']:
            state, reason = 'out', 'duplicate'
        elif company not in common.MAKERS[r['family']]:
            state, reason = 'out', 'maker'
        elif (r.get('stock') or 0) <= 0:
            state, reason = 'out', 'no stock'
        elif r.get('status') != 'Active':
            state, reason = 'out', 'not active'
        row = {'family': r['family'], 'company': company, 'mpn': r['mpn'], 'series': r['series'],
               'stock': r.get('stock', ''), 'status': r.get('status', ''), 'package': p.get('Package / Case', ''),
               'supplier_package': p.get('Supplier Device Package', ''),
               'grade': 'Automotive' if p.get('Grade') == 'Automotive' else '', 'datasheet_url': link,
               'maker_url': url, 'state': state, 'reason': reason, 'pdf': '', 'hit': '', 'dead': '',
               'stock_checked': ''}
        if key in old and not r['dup']:
            for c in CARRY:
                row[c] = old[key][c]
            if state == 'candidate' and old[key]['state'] == 'out' and old[key]['reason'] in CARRY_OUT:
                row['state'], row['reason'] = 'out', old[key]['reason']
        if stock and not r['dup'] and row['state'] == 'candidate':
            s = stock.get(key)
            if s and s['found'] == '1':
                row['stock_checked'] = '1'
                row['stock'] = s['stock']
            else:
                row['state'], row['reason'] = 'out', 'stock check'
        rows.append(row)
    common.write_tsv(out, COLS, rows)
    if '--report' in args:
        report(rows)


def report(rows):
    sums, per = pick_sums()
    listed = [r for r in rows if r['reason'] != 'duplicate']
    print(f'listed: {len(rows)} (planning census pick sum {sum(sums.values())})')
    for fam in common.FAMILIES:
        n = sum(1 for r in rows if r['family'] == fam)
        print(f'{fam}: {n} (pick sum {sums[fam]})')
    for fam in common.FAMILIES:
        for co in common.COMPANIES:
            a = [r for r in rows if r['family'] == fam and r['company'] == co]
            if not a:
                continue
            c = [r for r in a if r['state'] == 'candidate']
            print(f'  {fam:8} {co:26} listed {len(a):4} candidates {len(c):4} with maker url {sum(1 for r in c if r["maker_url"]):4} '
                  f'stock_checked {sum(1 for r in c if r["stock_checked"] == "1"):4} pdf {sum(1 for r in c if r["pdf"]):4} hit {sum(1 for r in c if r["hit"] != ""):4}')
    cand = [r for r in rows if r['state'] == 'candidate']
    print(f'candidate: {len(cand)}; out: ' + ', '.join(f'{k} {v}' for k, v in
                                                       collections.Counter(r['reason'] for r in rows if r['state'] == 'out').most_common()))
    print(f'candidate rows with stock_checked 1: {sum(1 for r in cand if r["stock_checked"] == "1")}; '
          f'with pdf: {sum(1 for r in cand if r["pdf"])}; with hit filled: {sum(1 for r in cand if r["hit"] != "")}; '
          f'with an empty maker_url: {sum(1 for r in cand if not r["maker_url"])}')
    print(f'distinct maker URLs of candidates: {len({r["maker_url"] for r in cand if r["maker_url"]})}; '
          f'automotive grade: {sum(1 for r in cand if r["grade"])}')
    del listed


if __name__ == '__main__':
    main()
