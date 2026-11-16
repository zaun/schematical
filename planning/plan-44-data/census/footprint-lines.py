#!/usr/bin/env python3
"""plan-44 census: footprints-needed.txt from candidates.tsv.

usage: python3 -I footprint-lines.py <candidates.tsv> > footprints-needed.txt

One line per (maker, DigiKey Package / Case, Supplier Device Package, Size /
Dimension) of the download-set rows (`candidate` and `reserve`; the `add` rows
once plan-44-B has set any), with the classes it covers, the number of parts, the part with the
most stock as the example and its datasheet link. The package drawing in that
datasheet is what plan-44-C / D draw the footprint from; DigiKey's package
names only group the parts. Lines starting with `#` are the header.
Tab-separated: letter, maker, mounting, package, supplier package, size,
classes, parts, example part, example datasheet, existing footprints whose id
holds the same body size code (e.g. `3225` for 3.2 x 2.5 mm; candidates only:
C / D accept one when its pad centres, pad sizes and pad count equal the
datasheet drawing's, else draw a new footprint), label (the footprint id
without its maker suffix, `label()`).

Letters: C = surface-mount crystals (kHz and MHz); D = through-hole crystals
and every ceramic resonator.
"""
import collections
import csv
import json
import os
import re
import sys

FOOTPRINTS = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..',
                          'schematical', 'schematical', 'Parts', 'footprints.json')
IDS = [f['id'] for f in json.load(open(FOOTPRINTS))['footprints']]


def letter(r):
    return 'D' if r['mounting'] == 'Through Hole' or r['class'] == 'resonator' else 'C'


def code(size):
    """'3.20mm x 2.50mm' -> '3225' (millimetres x 10, long side first); a side of 10 mm
    or more is spelled `118X55` (11.8 x 5.5 mm)."""
    m = re.search(r'\(([\d.]+)mm x ([\d.]+)mm\)', size or '')
    if not m:
        return ''
    a, b = (round(float(x) * 10) for x in m.groups())
    a, b = max(a, b), min(a, b)
    return f'{a}X{b}' if a >= 100 else f'{a:02d}{b:02d}'


def label(r):
    """The footprint label of a row: `<XTAL|CRES>-<body>-<pads>`, the id of a
    footprint without its maker (and variant) suffix. C and D name the ids
    `<label>-<MAKER>`; plan-44-F's generic parts and plan-44-B's pick refer to
    footprints by this label."""
    prefix = 'CRES' if r['class'] == 'resonator' else 'XTAL'
    package, size = r['package'], r['size']
    m = re.match(r'(\d)-', package)
    pads = m.group(1) if m else {'HC-49/US - 3 Lead': '3', 'HC-49/US - 4 Lead': '4',
                                 '1512 (3731 Metric)': '3'}.get(package, '2')
    if package.startswith('HC-49/US'):
        body = 'HC49US'
    elif package == 'HC-49/U':
        body = 'HC49U'
    elif package == 'HC-46/X':
        body = 'HC46X'
    elif package.startswith('Cylindrical'):
        m = re.search(r'\(([\d.]+)mm x ([\d.]+)mm\)', size)
        body = f'CYL-{round(float(m.group(1)))}X{round(float(m.group(2)))}' if m else 'CYL'
    elif 'SOJ' in package:
        m = re.search(r'([\d.]+)mm pitch', package)
        body = f'SOJ-{float(m.group(1)):g}' if m else 'SOJ'
    else:
        body = code(size) or 'X'
    if r['mounting'] == 'Surface Mount' and (body.startswith(('HC4', 'CYL'))):
        body += '-SMD'   # a surface-mount can and a through-hole one are different footprints
    return f'{prefix}-{body}-{pads}'


def main():
    every = list(csv.DictReader(open(sys.argv[1]), delimiter='\t'))
    # After plan-44-B the rows that were added are `add`; before it, the download set.
    wanted = ('add',) if any(r['state'] == 'add' for r in every) else ('candidate', 'reserve')
    rows = [r for r in every if r['state'] in wanted]
    groups = collections.defaultdict(list)
    for r in rows:
        groups[(letter(r), r['maker'], r['mounting'], r['package'], r['supplier_package'], r['size'])].append(r)
    print('# letter\tmaker\tmounting\tpackage\tsupplier package\tsize\tclasses\tparts\texample part\t'
          'example datasheet\texisting footprints with the same body size code\tlabel')
    for key in sorted(groups):
        g = groups[key]
        ex = max(g, key=lambda r: int(r['stock']))
        c = code(key[5])
        same = [i for i in IDS if c and re.search(r'(?<!\d)' + c + r'(?!\d)', i)]
        print('\t'.join([key[0], key[1], key[2], key[3], key[4], key[5],
                         ','.join(sorted({r['class'] for r in g})), str(len(g)), ex['mpn'],
                         ex['datasheet'], ' '.join(same), label(ex)]))


main()
