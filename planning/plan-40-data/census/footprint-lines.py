#!/usr/bin/env python3
"""plan-40 census: footprints-needed.txt from candidates.tsv.

usage: python3 -I footprint-lines.py <candidates.tsv> > footprints-needed.txt

One line per (maker, DigiKey Package / Case) of the `candidate` rows, with the
family groups and supplier package names it covers, the number of parts, the
part with the most stock as the example and its datasheet link. The package drawing in that
datasheet is what plan-40-C..E draw the footprint from; DigiKey's package
names only group the parts. Lines starting with `#` are the header.
Tab-separated: letter, maker, mounting, package, groups, supplier packages, parts,
example part, example datasheet, existing footprints with the same outline.
"""
import collections
import csv
import json
import os
import re
import sys

# DigiKey Package / Case (regex) -> the footprint ids of the library that name the
# same JEDEC / EIAJ outline (regex on the id). First match wins. These are
# candidates only: plan-40-C..E accept one when its body and pad pitch equal the
# datasheet drawing's (fp-map.py), else draw a new footprint.
PACKAGE_IDS = [
    (r'TO-226|TO-92', r'^TO-?92'),
    (r'TO-225|TO-126', r'^TO-?126'),
    (r'TO-220-3 Full Pack|TO-220.*Full', r'TO-?220(F|-FULL)|ITO-?220-3|FULL-PACK'),
    (r'TO-220', r'^TO-220(AB)?-3'),
    (r'TO-247|TO-274', r'^TO-247-3'),
    (r'TO-3P', r'^TO-3P'),
    (r'TO-251|IPAK|TO-262|I2PAK', r'^(IPAK|I2PAK|TO-25|TO-262)'),
    (r'TO-252|DPAK', r'^(SLIM|THIN)?DPAK|^TO-252'),
    (r'TO-263|D2PAK', r'D2PAK|^TO-263'),
    (r'TO-261|SOT-223', r'^SOT-?223'),
    (r'TO-243|SOT-89', r'^SOT-?89'),
    (r'TO-236|SOT-23-3', r'^SOT-?23-3'),
    (r'SC-70, SOT-323', r'^SOT-?323'),
    (r'SC-89|SOT-490', r'^SC-?89'),
    (r'SC-75|SOT-416', r'^SC-?75'),
    (r'SOT-723', r'^SOT-?723'),
    (r'SOT-563|SOT-666', r'^SOT-?563'),
    (r'SC-88|SOT-363', r'^(SC-?88-?6|SOT-?363)'),
    (r'SC-74|SOT-457', r'^SC-?74'),
    (r'SOT-23-6|TSOT-23-6', r'^SOT-?23-6'),
    (r'^8-SOIC', r'^SOIC-8$'),
    (r'^16-SOIC', r'^SOIC-16$'),
]

GROUP = {
    'bjt': 'bjt', 'bjt-array': 'bjt-array', 'bjt-prebiased': 'bjt',
    'bjt-prebiased-array': 'bjt-array', 'mosfet': 'mosfet', 'mosfet-array': 'mosfet-array',
}

FOOTPRINTS = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..',
                          'schematical', 'schematical', 'Parts', 'footprints.json')
IDS = [f['id'] for f in json.load(open(FOOTPRINTS))['footprints']]


# Which footprint letter draws the line (plan-40-C through-hole, D small-signal
# surface mount, E power and leadless surface mount): through hole is C; a power
# or multi-pin package (a tab, a Power name, DFN/QFN with power pads, SOIC, TSSOP, DirectFET)
# is E; every other surface-mount package (SOT, SC, TO-236, TO-243, the 0402
# and the 3-6 lead mini DFN, LGA and BGA chips) is D.
E_PACKAGE = re.compile(r'TO-252|DPAK|TO-26|SOT-223|TO-261|TO-277|Power|DirectFET|SOIC|TSSOP \(|'
                       r'VFSOP|TSOP \(|SMD, Flat Leads|SSOT|SC-100|SOT-669|^10-|^12-|^16-|^32-|^8-')


def letter(mounting, package):
    if mounting == 'Through Hole':
        return 'C'
    return 'E' if E_PACKAGE.search(package) else 'D'


def existing(package):
    for pkg, ids in PACKAGE_IDS:
        if re.search(pkg, package):
            return [i for i in IDS if re.search(ids, i)]
    return []


groups = collections.OrderedDict()
with open(sys.argv[1], newline='') as fh:
    for r in csv.DictReader(fh, delimiter='\t'):
        if r['reason'] != 'candidate':
            continue
        key = (r['maker'], r['mounting'], r['package'])
        g = groups.setdefault(key, {'n': 0, 'best': r, 'groups': set(), 'supplier': set()})
        g['n'] += 1
        g['groups'].add(GROUP[r['family']])
        g['supplier'].add(r['supplier_package'])
        if int(r['stock']) > int(g['best']['stock']):
            g['best'] = r

print('# letter\tmaker\tmounting\tpackage\tgroups\tsupplier packages\tparts\texample part\texample datasheet\texisting footprints with the same outline')
for key in sorted(groups):
    g = groups[key]
    print('\t'.join([letter(key[1], key[2])] + list(key) + [','.join(sorted(g['groups'])), ' | '.join(sorted(g['supplier'])),
                               str(g['n']), g['best']['mpn'], g['best']['datasheet_url'],
                               ','.join(existing(key[2]))]))
