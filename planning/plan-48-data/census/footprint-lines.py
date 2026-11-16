#!/usr/bin/env python3
"""plan-48 census: footprints-needed.txt, one line per distinct package of
the download set (candidate + reserve rows of candidates.tsv).
usage: footprint-lines.py <candidates.tsv> <footprints.json> [--add] > footprints-needed.txt
(--add: the `add` rows, plan-48-A's final pick, instead of the download set.)
Key: (company, mounting, Package / Case, Supplier Device Package).
Columns: letter (the plan-48 letter that draws it: C leaded packages - DIP,
SOIC, SOP, SSOP, TSSOP, MSOP / VSSOP, SOT-23, TSOT, SC-70, TO-263; D
leadless and ball packages - DFN, SON, QFN, LFCSP, LGA, WLP / WLCSP / BGA),
company, mounting, package, supplier_package, pins, types (timer, opamp),
candidates (rows in the pick at planning time), parts (candidates +
reserve), example (the most-stocked part), datasheets (distinct final PDFs
of the line's rows), existing (footprints.json ids of the same package
family and pin count: a hint for the reuse check, which is made against
the datasheet's drawing; empty = no footprint of that family and pin count
exists, a certain gap)."""
import collections
import csv
import json
import re
import sys

STATES = ('add',) if '--add' in sys.argv else ('candidate', 'reserve')
rows = [r for r in csv.DictReader(open(sys.argv[1], encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE)
        if r['state'] in STATES]
ids = [f['id'] for f in json.load(open(sys.argv[2], encoding='utf-8'))['footprints']]

# (pattern on "package | supplier package", footprint id prefixes of the family)
FAMILIES = [
    (r'DIP', ['PDIP', 'DIP']),
    (r'SOT-23-\d Thin|TSOT', ['SOT23-THIN', 'TSOT23', 'TSOT']),
    (r'SC-74A|SOT-753|SOT-23', ['SOT23']),
    (r'SC-70|SOT-353|SC-88|SOT-363', ['SC70', 'SOT353', 'SOT363']),
    (r'MSOP|VSSOP|uMAX|MiniSO', ['VSSOP', 'MSOP', 'UMAX']),
    (r'TVSOP', ['TVSOP']),
    (r'TSSOP', ['TSSOP', 'HTSSOP']),
    (r'SSOP', ['SSOP', 'QSOP']),
    (r'SOIC \(0\.209|SOIC \(0\.295|-SOP\b', ['SOP', 'SOIC']),
    (r'SOIC|-SO\b', ['SOIC', 'SO']),
    (r'TO-263', ['TO263', 'DDPAK', 'TO-263']),
    (r'DFN|SON|LFCSP-[WU]D|TDFN', ['DFN', 'SON', 'WSON', 'USON', 'X1SON', 'X2SON', 'VSON', 'LFCSP', 'TDFN']),
    (r'QFN|LFCSP', ['QFN', 'WQFN', 'UQFN', 'LFCSP']),
    (r'LGA', ['LGA', 'UTLGA']),
    (r'BGA|WLP|WLCSP|UCSP|uSMD', ['DSBGA', 'WLP', 'WLCSP', 'BGA', 'UCSP']),
]
LEADLESS = re.compile(r'DFN|SON|QFN|LFCSP|LGA|BGA|WLP|WLCSP|UCSP|uSMD')


def pins(package, supplier):
    m = re.match(r'(\d+)-', package)
    if m:
        return m.group(1)
    m = re.search(r'(?:SOT-23|TSOT-23|TSOT|SC-70)-(\d+)', supplier + ' ' + package)
    if m:
        return m.group(1)
    if 'SC-74A' in package:
        return '5'
    if 'DDPAK-7' in supplier:
        return '7'
    return ''


def existing(package, supplier, n):
    text = package + ' | ' + supplier
    for pat, prefixes in FAMILIES:
        if re.search(pat, text):
            return ','.join(sorted(i for i in ids if any(re.match(re.escape(p) + r'-?' + n + r'($|-)', i) or
                                                         re.match(re.escape(p) + r'-(THIN-)?' + n + r'($|-)', i)
                                                         for p in prefixes))[:10])
    return ''


lines = collections.defaultdict(list)
for r in rows:
    lines[(r['company'], r['mounting'], r['package'], r['supplier_package'])].append(r)
print('# letter\tcompany\tmounting\tpackage\tsupplier_package\tpins\ttypes\tcandidates\tparts\texample\tdatasheets\texisting')
out = []
for k, rs in lines.items():
    rs.sort(key=lambda r: -int(r['stock']))
    n = pins(k[2], k[3])
    letter = 'D' if LEADLESS.search(k[2] + ' ' + k[3]) else 'C'
    out.append((letter,) + k + (n, ','.join(sorted({r['type'] for r in rs})),
                                sum(r['state'] in ('candidate', 'add') for r in rs), len(rs), rs[0]['mpn'],
                                len({r['final'] for r in rs}), existing(k[2], k[3], n)))
for line in sorted(out):
    print(*line, sep='\t')
