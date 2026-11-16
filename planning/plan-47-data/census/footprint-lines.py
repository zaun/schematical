#!/usr/bin/env python3
"""plan-47 census: footprints-needed.txt, one line per distinct package of
the download set (candidate + reserve rows of candidates.tsv).
usage: footprint-lines.py <candidates.tsv> <footprints.json> [--add] > footprints-needed.txt
(--add: the `add` rows, plan-47-C's final pick, instead of the download set.)
Key: (type, company, series, mounting, Package / Case, Supplier Device
Package, Size / Dimension, detail); `series` is filled for relays only (a
relay's pin pattern is its series', DigiKey gives no package for it) and
`detail` is candidates.tsv's: a relay's form, coil type and termination, a
PTC's lead spacing, a buzzer's termination. Columns: letter (the plan-47
letter that draws it: D inductors, E fuses, F PTC resettable fuses, G
relays, H buzzers), type, company, series, mounting, package,
supplier_package, size, detail, parts, example (the most-stocked part),
datasheets (distinct URLs of the line's rows), existing
(footprints.json ids of the same package code: a hint for the reuse check,
which is made against the datasheet's drawing)."""
import collections
import csv
import json
import re
import sys

LETTER = {'inductor': 'D', 'fuse': 'E', 'ptc': 'F', 'relay': 'G', 'buzzer': 'H'}
STATES = ('add',) if '--add' in sys.argv else ('candidate', 'reserve')
rows = [r for r in csv.DictReader(open(sys.argv[1], encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE)
        if r['state'] in STATES]
ids = [f['id'] for f in json.load(open(sys.argv[2], encoding='utf-8'))['footprints']]


def existing(package, supplier):
    text = package + ' ' + supplier
    found = []
    m = re.search(r'\b(01005|0201|0402|0603|0805|0806|1008|1206|1210|1812|2010|2410|2512|2920)\b', text)
    if m:
        found += [i for i in ids if re.search(r'(^|-)' + m.group(1) + r'($|-)', i)]
    m = re.match(r'(\d+)-(DIP|SOP|SOIC|SSOP|SIP|VSSOP|SON|TSON)', package) or re.match(r'(\d+)-(DIP|SOP|SOIC|SSOP|SIP|VSSOP|SON|TSON)', supplier)
    if m:
        n, fam = m.groups()
        found += [i for i in ids if re.match(r'(P|C)?' + fam + r'-?' + n + r'($|-)', i)]
    return ','.join(sorted(set(found))[:8])


lines = collections.defaultdict(list)
for r in rows:
    relay = r['type'] == 'relay'
    det = (r['form'] + '; ' + r['detail']) if relay else (r['detail'] if r['type'] in ('ptc', 'buzzer') else '')
    lines[(LETTER[r['type']], r['type'], r['company'], r['series'] if relay else '', r['mounting'], r['package'],
           r['supplier_package'], r['size'], det)].append(r)
print('# letter\ttype\tcompany\tseries\tmounting\tpackage\tsupplier_package\tsize\tdetail\tparts\texample\tdatasheets\texisting')
for k in sorted(lines):
    rs = sorted(lines[k], key=lambda r: -int(r['stock']))
    print(*k, len(rs), rs[0]['mpn'], len({r['url'] for r in rs}), existing(k[5], k[6]), sep='\t')
