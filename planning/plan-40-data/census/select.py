#!/usr/bin/env python3
"""plan-40 census: the pick, candidates.tsv.

usage: python3 -I select.py <raw dir> <out.tsv> [--stock <stock-check.tsv>]
                            [--verdicts <verdicts.tsv>] [--carry <candidates.tsv>] [--report]

Reads every <category>-<maker id>[-re].jsonl of top.js in <raw dir> (a `-re`
file is a deeper re-listing of the same piece and is read first) and writes
one row per listed part with the columns of COLUMNS. The `reason` column is
`candidate` for a part the pick takes and otherwise the rule that left it
out (plan-40-A section 3):

  no-stock          DigiKey gives no quantity above 0 (the in-stock filter lets a few through)
  not-a-transistor  DigiKey gives it no transistor parameters
  automotive        Grade or Qualification names Automotive / AEC-Q
  chassis           Mounting Type is not Surface Mount / Through Hole
  has-diode         the type names a built-in diode or Zener (not a pure transistor package)
  driver-ic         an array of more than 4 elements (a Darlington driver IC)
  duplicate         another listed part has the same (family, base, package, supplier
                    package) and more stock
  stock check       the part-by-part look-up (--stock, plan-40-A section 5 rule 2) did not
                    find it in stock, Active or Last Time Buy
  below quota       the (family, maker) share of the family's quota is full of parts
                    with more stock

With --verdicts (plan-40-B section 3.2) the reasons become final: a row out
for a rule above is `out:<reason>`; an eligible row takes its verdict (`skip:
<why>`, `out:datasheet status`); the duplicate rule and the quota then run
again over the rows whose verdict is `add` only, so a skipped winner's slot
goes to its duplicate or to the next row of its (family, maker): `add`,
`out:duplicate`, `out:below quota`. `pdf` comes from the verdicts file;
--carry copies `pdf`, `hit` and `dead` by part number from an earlier
candidates.tsv (plan-40-A Phase 3's columns, which the raw listing lacks).

`cut` is 1 for a part listed by a piece DigiKey cut at 300 (its 300 most-stocked
parts only): plan-40-A re-checks those part by part; `stock_checked` is 1 for
every part number the --stock file holds. `--report` prints the
counts behind every number the plan quotes, one `cut piece` line per cut piece
with its verdict (`complete for the pick`, or `NEEDS DEEPER LISTING` when the
300th part's stock is at or above the lowest stock the pick takes from that
(family, maker)).
"""
import collections
import csv
import glob
import json
import os
import re
import sys

MAKERS = {
    '488': 'onsemi', '600039': 'onsemi',
    '1727': 'Nexperia',
    '31': 'Diodes Incorporated',
    '448': 'Infineon Technologies', '21': 'Infineon Technologies', '600047': 'Infineon Technologies',
    '742': 'Vishay Siliconix', '600092': 'Vishay Siliconix',
}
FAMILY = {
    '276': 'bjt', '277': 'bjt-array', '292': 'bjt-prebiased', '293': 'bjt-prebiased-array',
    '278': 'mosfet', '289': 'mosfet-array',
}
# Quota per family; each of the family's three makers gets an equal share, most
# stock first (plan-40-A section 3 rule 5).
QUOTA = {
    'bjt': 450, 'bjt-array': 60, 'bjt-prebiased': 120, 'bjt-prebiased-array': 40,
    'mosfet': 600, 'mosfet-array': 80,
}
# Parts the owner named: always picked when listed (plan-40-A Decisions).
NAMED = ['2N3904', '2N3906', 'BC547B', 'BC557B', '2N7000', 'BS170', '2N7002']

COLUMNS = [
    'mpn', 'maker', 'category', 'family', 'piece', 'stock', 'status', 'base', 'series',
    'type', 'package', 'supplier_package', 'mounting', 'vmax_v', 'imax_a', 'power_w',
    'rdson_ohm', 'vgsth_v', 'drive_v', 'r1_ohm', 'r2_ohm', 'grade', 'datasheet_url',
    'reason', 'cut', 'stock_checked', 'pdf', 'hit', 'dead', 'vgs_v', 'hfe_min',
]


def num(text):
    """The first number of a DigiKey value text, in base units (V, A, W, ohm)."""
    m = re.match(r'\s*[±+]?\s*(-?[0-9.]+)\s*([a-zA-ZµΩΩ]*)', text or '')
    if not m:
        return ''
    value = float(m.group(1))
    unit = m.group(2)
    for prefix, factor in {'m': 1e-3, 'µ': 1e-6, 'u': 1e-6, 'n': 1e-9, 'k': 1e3, 'M': 1e6, 'G': 1e9}.items():
        if len(unit) > 1 and unit.startswith(prefix):
            value *= factor
            break
    return value


def first(p, *names):
    for n in names:
        if n in p and p[n] not in ('', '-'):
            return p[n]
    return ''


def row_of(d):
    p = d['params']
    return {
        'mpn': d['mpn'], 'maker': MAKERS[d['makerId']], 'category': d['category'],
        'family': FAMILY[d['category']], 'piece': d['piece'], 'stock': d.get('stock') or 0,
        'status': d['status'], 'base': d.get('base') or d['mpn'], 'series': d['series'],
        'type': first(p, 'Transistor Type', 'FET Type', 'Configuration'),
        'package': first(p, 'Package / Case'),
        'supplier_package': first(p, 'Supplier Device Package'),
        'mounting': first(p, 'Mounting Type'),
        'vmax_v': num(first(p, 'Voltage - Collector Emitter Breakdown (Max)',
                            'Drain to Source Voltage (Vdss)')),
        'imax_a': num(first(p, 'Current - Collector (Ic) (Max)',
                            'Current - Continuous Drain (Id) @ 25°C')),
        'power_w': num(first(p, 'Power - Max', 'Power Dissipation (Max)')),
        'rdson_ohm': num(first(p, 'Rds On (Max) @ Id, Vgs')),
        'vgsth_v': num(first(p, 'Vgs(th) (Max) @ Id')),
        'drive_v': num(first(p, 'Drive Voltage (Max Rds On, Min Rds On)')),
        'r1_ohm': num(first(p, 'Resistor - Base (R1)')),
        'r2_ohm': num(first(p, 'Resistor - Emitter Base (R2)')),
        'grade': ' '.join(x for x in (first(p, 'Grade'), first(p, 'Qualification')) if x),
        'datasheet_url': d['datasheet'], 'reason': '', 'cut': '', 'stock_checked': '',
        'pdf': '', 'hit': '', 'dead': '',
        'vgs_v': num(first(p, 'Vgs (Max)')),
        'hfe_min': num(first(p, 'DC Current Gain (hFE) (Min) @ Ic, Vce')),
        '_has_params': bool(p),
    }


def stems(raw):
    """The file stems of <raw dir>, a `-re` re-listing before the piece it re-lists."""
    found = [os.path.basename(f)[:-4] for f in glob.glob(os.path.join(raw, '*.err'))]
    return sorted(found, key=lambda s: (s[:-3] if s.endswith('-re') else s, 0 if s.endswith('-re') else 1))


def pieces_of(path):
    pieces = []
    for line in open(path):
        m = re.match(r'piece (.*): (\d+) of (\d+)( \(cut at 300: (\d+)\))?', line)
        if m:
            pieces.append((m.group(1), int(m.group(2)), int(m.group(3)), m.group(5)))
    return pieces


def cut_pieces(raw):
    """{(stem, piece label): (parts in the piece, stock of its 300th part)} for every piece
    cut at 300. Not counted: a whole piece when every part has a value of the type facet
    (its own pieces list it), and a base file's whole piece when a `-re` re-listing of it exists."""
    cut = {}
    names = stems(raw)
    for stem in names:
        pieces = pieces_of(os.path.join(raw, stem + '.err'))
        sub = [p for p in pieces if p[0] != 'all']
        whole = [p for p in pieces if p[0] == 'all'][0]
        remainder = whole[2] - sum(p[2] for p in sub) if sub else whole[2]
        for label, _listed, count, stock in pieces:
            if stock is None:
                continue
            if label == 'all' and sub and remainder <= 0:
                continue
            if label == 'all' and stem + '-re' in names:
                continue
            cut[(stem, label)] = (count, int(stock))
    return cut


def main():
    raw, out = sys.argv[1], sys.argv[2]
    report = '--report' in sys.argv
    checked = {}
    if '--stock' in sys.argv:
        path = sys.argv[sys.argv.index('--stock') + 1]
        if os.path.exists(path):
            for r in csv.DictReader(open(path), delimiter='\t'):
                checked[r['mpn']] = r['found'] == '1'
    verdicts = {}
    if '--verdicts' in sys.argv:
        for r in csv.DictReader(open(sys.argv[sys.argv.index('--verdicts') + 1]), delimiter='\t'):
            verdicts[r['mpn']] = r
    carry = {}
    if '--carry' in sys.argv:
        for r in csv.DictReader(open(sys.argv[sys.argv.index('--carry') + 1]), delimiter='\t'):
            carry[r['mpn']] = r
    rows = {}
    for stem in stems(raw):
        path = os.path.join(raw, stem + '.jsonl')
        for line in open(path):
            d = json.loads(line)
            r = row_of(d)
            r['_file'] = stem
            rows.setdefault(d['mpn'], r)
    rows = list(rows.values())
    listed = len(rows)
    cuts = cut_pieces(raw)
    for r in rows:
        r['cut'] = 1 if (r['_file'], r['piece']) in cuts else 0

    for r in rows:
        if r['stock'] <= 0:
            r['reason'] = 'no-stock'
        elif not r['_has_params'] or not r['type']:
            r['reason'] = 'not-a-transistor'
        elif re.search(r'Automotive|AEC-Q', r['grade']):
            r['reason'] = 'automotive'
        elif r['mounting'] not in ('Surface Mount', 'Through Hole'):
            r['reason'] = 'chassis'
        elif re.search(r'Diode|Zener', r['type']):
            r['reason'] = 'has-diode'
        elif re.match(r'(\d+) ', r['type']) and int(re.match(r'(\d+) ', r['type']).group(1)) > 4:
            r['reason'] = 'driver-ic'
        if r['mpn'] in checked:
            r['stock_checked'] = 1
            if not r['reason'] and not checked[r['mpn']]:
                r['reason'] = 'stock check'

    for r in rows:
        if r['mpn'] in carry:
            for c in ('pdf', 'hit', 'dead'):
                r[c] = carry[r['mpn']][c]
    if verdicts:
        for r in rows:
            if r['reason']:
                r['reason'] = 'out:' + r['reason']
                continue
            v = verdicts.get(r['mpn'])
            if v is None:
                sys.exit(f"select.py: no verdict for {r['mpn']} (re-run verdicts.py)")
            r['pdf'] = v['pdf']
            if v['verdict'] == 'skip':
                r['reason'] = 'skip:' + v['reason']
            elif v['verdict'] == 'out':
                r['reason'] = 'out:' + v['reason']

    # Duplicates: one part per (family, base, package, supplier package): the most-stocked.
    best = {}
    for r in sorted((r for r in rows if not r['reason']), key=lambda r: -r['stock']):
        key = (r['family'], r['base'].upper(), r['package'], r['supplier_package'])
        if key in best:
            r['reason'] = 'out:duplicate' if verdicts else 'duplicate'
        else:
            best[key] = r

    # Quota per (family, maker): an equal share of the family's quota, most stock
    # first, so each of the family's three makers is represented; the named parts always.
    count = collections.Counter()
    for r in sorted((r for r in rows if not r['reason']), key=lambda r: -r['stock']):
        named = any(r['base'].upper() == n or r['mpn'].upper().startswith(n) for n in NAMED)
        share = -(-QUOTA[r['family']] // 3)
        if count[(r['family'], r['maker'])] < share or named:
            count[(r['family'], r['maker'])] += 1
            r['reason'] = 'add' if verdicts else 'candidate'
        else:
            r['reason'] = 'out:below quota' if verdicts else 'below quota'

    rows.sort(key=lambda r: (r['family'], -r['stock'], r['mpn']))
    with open(out, 'w', newline='') as fh:
        w = csv.writer(fh, delimiter='\t', lineterminator='\n')
        w.writerow(COLUMNS)
        for r in rows:
            w.writerow([r[c] for c in COLUMNS])

    if report:
        print(f'{listed} parts listed')
        print('reason:', dict(collections.Counter(r['reason'] for r in rows)))
        floor = {}
        picked = 'add' if verdicts else 'candidate'
        for fam in QUOTA:
            sel = [r for r in rows if r['family'] == fam and r['reason'] == picked]
            print(f'{fam}: {len(sel)} picked; per maker',
                  dict(collections.Counter(r['maker'] for r in sel)))
            for mk in sorted({r['maker'] for r in sel}):
                floor[(fam, mk)] = min(r['stock'] for r in sel if r['maker'] == mk)
        for (stem, label), (count_, stock) in sorted(cuts.items()):
            cat, mid = stem.split('-')[:2]
            limit = floor.get((FAMILY[cat], MAKERS[mid]))
            verdict = ('NEEDS DEEPER LISTING' if limit is not None and stock >= limit
                       else 'complete for the pick')
            print(f'cut piece {stem} {label}: {count_} parts, 300th stock {stock}, floor {limit}: {verdict}')
        print(f'{len(cuts)} pieces cut at 300')
        print('candidates from a cut piece:', sum(1 for r in rows if r['reason'] == picked and r['cut']),
              'of', sum(1 for r in rows if r['reason'] == picked))
        print('picked from a cut piece, not stock-checked:',
              sum(1 for r in rows if r['reason'] == picked and r['cut'] and r['stock_checked'] != 1))


main()
