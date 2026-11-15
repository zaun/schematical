#!/usr/bin/env python3
"""plan-42 census: footprints-needed.txt from candidates.tsv.

usage: python3 -I footprint-lines.py <candidates.tsv> > footprints-needed.txt

One line per (maker, DigiKey Package / Case, Supplier Device Package) of the
picked rows (`candidate`; `add` after plan-42-B), with the families it
covers, the number of parts, the part with the most stock as the example and
its datasheet link. The package drawing in that datasheet is what
plan-42-D / E / F draw the footprint from; DigiKey's package names only group the
parts. Lines starting with `#` are the header. Tab-separated: letter, maker,
mounting, package, supplier package, families, parts, example part, example
datasheet, existing footprints with the same outline (candidates only: D / E / F
accept one when its body, pitch and pad count equal the datasheet drawing's,
else draw a new footprint).

Letters (the plan-42 letter that draws the line): D = through-hole and leaded
surface-mount packages (TO-92, TO-220, TO-263, DPAK, SOT-223, SOT-89, SOT-23,
SC-70, SOIC, MSOP, TSSOP, HTSSOP, ...); leadless and chip-scale packages (SON,
DFN, WSON, X2SON, QFN, BGA, WLCSP, DSBGA): E for Texas Instruments and
Microchip Technology, F for Analog Devices.
"""
import collections
import csv
import json
import os
import re
import sys

# (DigiKey Package / Case regex, footprint id regex): first match wins.
PACKAGE_IDS = [
    (r'^TO-226|^TO-92', r'^TO-?92-3'),
    (r'^TO-220-3', r'^TO-220(AB)?-3$'),
    (r'^TO-263-3|^D2PAK.*3', r'^(D2PAK-3|TO-263-2-TAB)$'),
    (r'^TO-252-3|^DPAK', r'^DPAK-3$'),
    (r'^TO-261-4|SOT-223', r'^SOT-?223'),
    (r'^TO-243|SOT-89', r'^SOT-?89-3$'),
    (r'^TO-236-3|SOT-23-3', r'^SOT23-3$'),
    (r'^SC-74A, SOT-753|SOT-23-5', r'^SOT23-5$'),
    (r'^SOT-23-6', r'^SOT23-6$'),
    (r'^SOT-23-8', r'^SOT23-8'),
    (r'^5-TSSOP, SC-70-5|SC-70-5', r'^SC70-5$'),
    (r'^6-TSSOP, SC-88|SC-70-6', r'^SC70-6$'),
    (r'^SOT-563', r'^SOT563-6$'),
    (r'^8-SOIC', r'^SOIC-8$'),
    (r'^8-TSSOP, 8-MSOP|^8-MSOP', r'^MSOP-?8'),
]

FOOTPRINTS = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..',
                          'schematical', 'schematical', 'Parts', 'footprints.json')
IDS = [f['id'] for f in json.load(open(FOOTPRINTS))['footprints']]

LEADLESS = re.compile(r'SON|DFN|QFN|LGA|BGA|WLCSP|CSP|DSBGA|Die|Flip Chip|PowerPAD.*QFN|uMAX.*TDFN', re.I)


def letter(maker, mounting, package, supplier):
    if mounting == 'Through Hole' or not (LEADLESS.search(package) or LEADLESS.search(supplier)):
        return 'D'
    return 'F' if maker == 'Analog Devices' else 'E'


def existing(package):
    for pkg, ids in PACKAGE_IDS:
        if re.search(pkg, package):
            return [i for i in IDS if re.search(ids, i)]
    return []


groups = collections.OrderedDict()
with open(sys.argv[1], newline='') as fh:
    for r in csv.DictReader(fh, delimiter='\t'):
        if r['reason'] not in ('add', 'candidate'):
            continue
        key = (r['maker'], r['mounting'], r['package'], r['supplier_package'])
        g = groups.setdefault(key, {'n': 0, 'best': r, 'families': set()})
        g['n'] += 1
        g['families'].add(r['family'])
        if int(r['stock']) > int(g['best']['stock']):
            g['best'] = r

print('# letter\tmaker\tmounting\tpackage\tsupplier package\tfamilies\tparts\texample part\texample datasheet\texisting footprints with the same outline')
for key in sorted(groups, key=lambda k: (letter(*k),) + k):
    g = groups[key]
    print('\t'.join([letter(*key)] + list(key) + [
        ','.join(sorted(g['families'])), str(g['n']), g['best']['mpn'], g['best']['datasheet_url'],
        ','.join(existing(key[2]))]))
