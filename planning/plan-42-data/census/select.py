#!/usr/bin/env python3
"""plan-42 census: the pick, candidates.tsv.

usage: python3 -I select.py <raw dir> <out.tsv> [--stock <stock-check.tsv>]
                            [--verdicts <verdicts.tsv>] [--carry <candidates.tsv>] [--report]

Reads every <category>-<maker id>[-re].jsonl of top.js in <raw dir> (a `-re`
file is a deeper re-listing of the same piece and is read first) and writes
one row per listed part with the columns of COLUMNS. The `reason` column is
`candidate` for a part the pick takes and otherwise the rule that left it
out (plan-42-A section 3):

  no-stock           DigiKey gives no quantity above 0
  automotive         Grade or Qualification names Automotive / AEC-Q / Military / MIL
  chassis            Mounting Type is not Surface Mount / Through Hole
  output-type        Output Type is not Fixed / Adjustable / Adjustable (Fixed)
  tracking           a switching part whose Function is Ratiometric (DDR / VTT tracking)
  too-many-outputs   more than 4 regulators / outputs in the package
  digital-interface  the description or a parameter names I2C, SPI, PMBus, SMBus or VID
  duplicate          another listed part has the same (family, base, package, supplier
                     package, output voltage) and more stock
  stock check        the part-by-part look-up (--stock, plan-42-A section 5) did not
                     find it in stock, Active or Last Time Buy
  below quota        the (family, maker) share of the family's quota is full of parts
                     with more stock

With --verdicts (plan-42-B section 3.2) the reasons become final: a row out
for a rule above is `out:<reason>`; a download-set row takes its verdict
(`skip:<why>`, `out:datasheet status`, `out:below the pick (not read)`); a row
outside the download set is `out:below the pick (not read)`; the duplicate
rule and the quota then run again over the rows whose verdict is `add` only,
so a skipped winner's slot goes to its duplicate or to the next row of its
(family, maker): `add`, `out:duplicate`, `out:below quota`. `pdf` comes from
the verdicts file; --carry copies `pdf`, `hit` and `dead` by part number from
an earlier candidates.tsv (plan-42-A Phase 3's columns, which the raw listing
lacks).

`cut` is 1 for a part listed by a piece DigiKey cut at 300 (its 300
most-stocked parts only). `--report` prints the counts behind every number the
plan quotes, and one `cut piece` line per cut piece with its verdict
(`complete for the pick`, or `NEEDS DEEPER LISTING` when the 300th part's stock
is at or above the lowest stock the pick takes from that (family, maker); the
families of a category share its pieces, so the floor is the lowest of the
category's families for that maker).
"""
import collections
import csv
import glob
import json
import os
import re
import sys

MAKERS = {
    '296': 'Texas Instruments', '14': 'Texas Instruments', '600062': 'Texas Instruments',
    '505': 'Analog Devices', '175': 'Analog Devices',
    '150': 'Microchip Technology', '576': 'Microchip Technology',
}
# Quota per family; each maker gets a third, most stock first (plan-42-A section 3).
QUOTA = {'linear-fixed': 240, 'linear-adjustable': 90, 'linear-multi': 30, 'switching': 240}
# Parts planning/todo.md names (78xx, LM317, LDOs, bucks) by DigiKey base part number:
# always picked when listed and eligible, whatever the quota (plan-42-A Decisions).
NAMED = {'LM7805', 'UA7805', 'LM340', 'LM7833', 'LM7812', 'UA7812', 'LM317', 'LM1117', 'TLV1117',
         'LM2596', 'LM2576', 'MCP1700', 'MCP1702'}
NAMED_MAX = 6
CATEGORY_FAMILIES = {'699': ('linear-fixed', 'linear-adjustable', 'linear-multi'), '739': ('switching',)}

COLUMNS = [
    'mpn', 'maker', 'category', 'family', 'piece', 'stock', 'status', 'base', 'series',
    'output_type', 'outputs', 'package', 'supplier_package', 'mounting', 'vin_min_v', 'vin_max_v',
    'vout_v', 'vout_max_v', 'iout_a', 'dropout_v', 'iq_a', 'topology', 'function', 'fsw_hz',
    'sync', 'control', 'grade', 'datasheet_url', 'reason', 'cut', 'stock_checked', 'pdf', 'hit',
    'dead',
]


def num(text):
    """The first number of a DigiKey value text, in base units (V, A, Hz)."""
    m = re.match(r'\s*[±+~]?\s*(-?[0-9.]+)\s*([a-zA-Zµ]*)', text or '')
    if not m:
        return ''
    value = float(m.group(1))
    unit = m.group(2)
    for prefix, factor in {'m': 1e-3, 'µ': 1e-6, 'u': 1e-6, 'n': 1e-9, 'k': 1e3, 'M': 1e6, 'G': 1e9}.items():
        if len(unit) > 1 and unit.startswith(prefix):
            value *= factor
            break
    return round(value, 9)


def first(p, *names):
    for n in names:
        if n in p and p[n] not in ('', '-'):
            return p[n]
    return ''


def family_of(category, p):
    if category == '739':
        return 'switching'
    try:
        if int(first(p, 'Number of Regulators') or 1) > 1:
            return 'linear-multi'
    except ValueError:
        pass
    return 'linear-fixed' if first(p, 'Output Type') == 'Fixed' else 'linear-adjustable'


def row_of(d):
    p = d['params']
    return {
        'mpn': d['mpn'], 'maker': MAKERS[d['makerId']], 'category': d['category'],
        'family': family_of(d['category'], p), 'piece': d['piece'], 'stock': d.get('stock') or 0,
        'status': d['status'], 'base': d.get('base') or d['mpn'], 'series': d['series'],
        'output_type': first(p, 'Output Type'),
        'outputs': first(p, 'Number of Regulators', 'Number of Outputs'),
        'package': first(p, 'Package / Case'),
        'supplier_package': first(p, 'Supplier Device Package'),
        'mounting': first(p, 'Mounting Type'),
        'vin_min_v': num(first(p, 'Voltage - Input (Min)')),
        'vin_max_v': num(first(p, 'Voltage - Input (Max)')),
        'vout_v': num(first(p, 'Voltage - Output (Min/Fixed)')),
        'vout_max_v': num(first(p, 'Voltage - Output (Max)')),
        'iout_a': num(first(p, 'Current - Output')),
        'dropout_v': num(first(p, 'Voltage Dropout (Max)')),
        'iq_a': num(first(p, 'Current - Quiescent (Iq)')),
        'topology': first(p, 'Topology'), 'function': first(p, 'Function'),
        'fsw_hz': num(first(p, 'Frequency - Switching')),
        'sync': first(p, 'Synchronous Rectifier'),
        'control': first(p, 'Control Features'),
        'grade': ' '.join(x for x in (first(p, 'Grade'), first(p, 'Qualification')) if x),
        'datasheet_url': d['datasheet'], 'reason': '', 'cut': '', 'stock_checked': '',
        'pdf': '', 'hit': '', 'dead': '',
        '_text': ' '.join([d.get('description') or ''] + [str(v) for v in p.values()]),
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
    (its own pieces list it), a base file's pieces when a `-re` re-listing of it exists, and
    a piece whose own sub-pieces list all its parts."""
    cut = {}
    names = stems(raw)
    for stem in names:
        pieces = pieces_of(os.path.join(raw, stem + '.err'))
        # the type pieces (`all/<value>`; a value may itself hold a `/`, as
        # "Adjustable/Programmable" does). A base listing splits once, so every
        # piece but `all` is a type piece; a `-re` re-listing also splits a type
        # piece above 300 by Package / Case, and those deeper pieces are left out.
        sub = [p for p in pieces if p[0] != 'all' and not (stem.endswith('-re') and any(
            p[0].startswith(q[0] + '/') for q in pieces if q[0] not in ('all', p[0]) and q[2] > 300))]
        whole = [p for p in pieces if p[0] == 'all']
        if not whole:   # a piece DigiKey refused (`piece all: failed`), listed in --report
            continue
        whole = whole[0]
        remainder = whole[2] - sum(p[2] for p in sub) if sub else whole[2]
        for label, _listed, count, stock in pieces:
            if stock is None:
                continue
            if label == 'all' and sub and remainder <= 0:
                continue
            if stem + '-re' in names:
                continue
            if label != 'all' and sum(p[1] for p in pieces if p[0].startswith(label + '/')) >= count:
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
        for line in open(os.path.join(raw, stem + '.jsonl')):
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
        try:
            outputs = int(r['outputs'] or 1)
        except ValueError:
            outputs = 99
        if r['stock'] <= 0:
            r['reason'] = 'no-stock'
        elif re.search(r'Automotive|AEC-Q|Military|MIL-', r['grade']):
            r['reason'] = 'automotive'
        elif r['mounting'] not in ('Surface Mount', 'Through Hole'):
            r['reason'] = 'chassis'
        elif r['output_type'] not in ('Fixed', 'Adjustable', 'Adjustable (Fixed)'):
            r['reason'] = 'output-type'
        elif r['family'] == 'switching' and r['function'].startswith('Ratiometric'):
            r['reason'] = 'tracking'
        elif outputs > 4:
            r['reason'] = 'too-many-outputs'
        elif re.search(r'\b(I2C|I²C|SPI|PMBus|SMBus|VID)\b', r['_text']):
            r['reason'] = 'digital-interface'
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
                r['reason'] = 'out:below the pick (not read)'
                continue
            r['pdf'] = v['pdf']
            if v['verdict'] == 'skip':
                r['reason'] = 'skip:' + v['reason']
            elif v['verdict'] == 'out':
                r['reason'] = 'out:' + v['reason']

    # Duplicates: one part per (family, base, package, supplier package, output voltage):
    # the most-stocked (packings of one part; a fixed regulator's voltages are different parts).
    best = {}
    for r in sorted((r for r in rows if not r['reason']), key=lambda r: (-r['stock'], r['mpn'])):
        key = (r['family'], r['base'].upper(), r['package'], r['supplier_package'], r['vout_v'])
        if key in best:
            r['reason'] = 'out:duplicate' if verdicts else 'duplicate'
        else:
            best[key] = r

    # A named base's NAMED_MAX most-stocked parts are taken first and count toward
    # their (family, maker) share; its other parts compete under the quota.
    named_taken = collections.Counter()
    for r in sorted((r for r in rows if not r['reason'] and r['base'].upper() in NAMED),
                    key=lambda r: (-r['stock'], r['mpn'])):
        named_taken[r['base'].upper()] += 1
        r['_named'] = named_taken[r['base'].upper()] <= NAMED_MAX
    count = collections.Counter()
    for r in sorted((r for r in rows if not r['reason']),
                    key=lambda r: (not r.get('_named'), -r['stock'], r['mpn'])):
        share = -(-QUOTA[r['family']] // 3)
        if count[(r['family'], r['maker'])] < share or r.get('_named'):
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
        print('reason:', dict(sorted(collections.Counter(r['reason'] for r in rows).items())))
        floor = {}
        pick_label = 'add' if verdicts else 'candidate'
        print('reason:', dict(sorted(collections.Counter(
            (r['reason'].split(':')[0] if r['reason'].startswith(('out:', 'skip:')) else r['reason'])
            for r in rows).items())))
        for fam in QUOTA:
            sel = [r for r in rows if r['family'] == fam and r['reason'] == pick_label]
            print(f'{fam}: {len(sel)} picked; per maker', dict(sorted(collections.Counter(r['maker'] for r in sel).items())))
            for mk in sorted({r['maker'] for r in sel}):
                quota_picks = [r['stock'] for r in sel if r['maker'] == mk and not r.get('_named')]
                floor[(fam, mk)] = min(quota_picks or [r['stock'] for r in sel if r['maker'] == mk])
        for (stem, label), (count_, stock) in sorted(cuts.items()):
            cat, mid = stem.split('-')[:2]
            limits = [floor[(f, MAKERS[mid])] for f in CATEGORY_FAMILIES[cat] if (f, MAKERS[mid]) in floor]
            limit = min(limits) if limits else None
            verdict = ('NEEDS DEEPER LISTING' if limit is not None and stock >= limit
                       else 'complete for the pick')
            print(f'cut piece {stem} {label}: {count_} parts, 300th stock {stock}, floor {limit}: {verdict}')
        print(f'{len(cuts)} pieces cut at 300')
        for stem in stems(raw):
            for line in open(os.path.join(raw, stem + '.err')):
                if ': failed' in line:
                    print(f'failed piece {stem}: {line.strip()[:90]}')
        picked = [r for r in rows if r['reason'] == pick_label]
        print('candidates from a cut piece:', sum(1 for r in picked if r['cut']), 'of', len(picked))
        print('picked per maker:', dict(sorted(collections.Counter(r['maker'] for r in picked).items())))
        print('named bases picked:', sorted({r['base'].upper() for r in picked if r['base'].upper() in NAMED}))
        print('named bases listed but not picked:', sorted({r['base'].upper() for r in rows if r['base'].upper() in NAMED} -
                                                         {r['base'].upper() for r in picked}))
        print('picked per status:', dict(sorted(collections.Counter(r['status'] for r in picked).items())))
        if verdicts:
            got = collections.Counter((r['family'], r['maker']) for r in picked)
            for fam in QUOTA:
                for mk in sorted({'Texas Instruments', 'Analog Devices', 'Microchip Technology'}):
                    share = -(-QUOTA[fam] // 3)
                    print(f'add {fam} {mk}: {got[(fam, mk)]} of {share}' + ('  SHORT' if got[(fam, mk)] < share else ''))
            print('add total:', len(picked))


main()
