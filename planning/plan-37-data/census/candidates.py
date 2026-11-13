#!/usr/bin/env python3
# plan-37 census: builds candidates.tsv from the planning listing (raw/*.jsonl).
# usage: python3 -I candidates.py <rawdir> <Parts dir> <out.tsv>
# One row per (maker, part number) the listing returned (in stock, Active or
# Last Time Buy at DigiKey). Each row gets the catalog device it belongs to,
# its logic family, the kind it goes into and a scope verdict:
#   in        a new register device (register kind), an existing dff
#             register device (74_174/175/273/374/574, 40175B) from onsemi or
#             Nexperia (dff kind), or a missing part of the existing 74_670
#             register file (mux kind, model regfile)
#   out:<why> not a register in scope (1-bit flip-flops, ECL, a TI part of a
#             dff device, a part name some part file already has)
# Phase A of the plan fills the datasheet columns; this script only sorts.
import json, re, sys
from pathlib import Path

raw, parts_dir, out_path = Path(sys.argv[1]), Path(sys.argv[2]), Path(sys.argv[3])
TI, ON, NX = 'Texas Instruments', 'onsemi', 'Nexperia USA Inc.'

# The new register devices (owner's answer: 173 class, clock-enable, bus-
# interface / inverting, wide 16/20/22-bit, the 670 register file) by the
# number in the part name -> catalog device id.
REGISTER = {
    '173': '74_173', '4076': '4076B', '14076': '4076B',
    '377': '74_377', '378': '74_378', '379': '74_379', '396': '74_396',
    '534': '74_534', '564': '74_564', '575': '74_575', '576': '74_576',
    '577': '74_577', '821': '74_821', '823': '74_823', '825': '74_825',
    '29821': '74_29821', '874': '74_874', '11374': '74_11374',
    '40174': '40174B', '14174': '40174B',
    '16374': '74_16374', '162374': '74_162374', '16821': '74_16821',
    '16823': '74_16823', '162823': '74_162823', '16820': '74_16820',
    '16721': '74_16721', '162721': '74_162721', '16722': '74_16722',
}
# The 74x670 is already the mux kind's 74_670 (model regfile, plan-21).
MUX = {'670': '74_670'}
# dff devices whose onsemi / Nexperia parts are in (owner's answer).
DFF = {'174': '74_174', '175': '74_175', '273': '74_273', '374': '74_374',
       '574': '74_574', '40175': '40175B', '14175': '40175B'}
# Military numbers in the listing, mapped from the DigiKey description.
MIL = {'5962-8682501EA': ('173', 'HC'), '5962-87807012A': ('377', 'HC'),
       '5962-8976901RA': ('377', 'HCT'), '5962-8992501EA': ('378', 'LS'),
       '5962-9314801Q2A': ('377', 'ABT'), '5962-9314801QRA': ('377', 'ABT'),
       '8400102RA': ('576', 'ALS'), 'JM38510/32504BRA': ('377', 'LS'),
       'JM38510/32504BSA': ('377', 'LS'),
       # found by plan-37-A's full TI listing (16- and 20-pin pieces)
       'JM38510/36101BEA': ('173', 'TTL'), 'JM38510/30109BEA': ('109', 'LS'),
       'JM38510/37201BEA': ('174', 'ALS'), '5962-9222203MRA': ('574', 'FCT'),
       '5962-9221804MRA': ('374', 'FCT'), '5962-9221806M2A': ('374', 'FCT')}
# Rows settled while planning (the datasheet was read then).
PLANNING_OUT = {
    # sn74als575a.pdf covers SN74AS575 (NT package) in its text, but its
    # PACKAGING INFORMATION table lists no SN74AS575 orderable, and
    # ti.com/product/SN74AS575 answers 404.
    'SN74AS575NT': 'out:not-in-ordering-table',
}
SUFFIX_FAMILY = {'ALVCH': 'ALVC', 'LVCH': 'LVC', 'AUCH': 'AUC', 'LVTH': 'LVT',
                 'ABTH': 'ABT', 'AHCH': 'AHC'}

PAT = re.compile(r'^(?:NLV(?=14)|NLV|NL)?(?:SNJ54|SN54|SN74|CD54|CD74|CY74|MM74|MC74|MC54|M74|MC|CLV|CD|HEF|HEC|74|54|)'
                 r'([A-Z]*?)(\d[GT]\d+|\d{2,6})')


def device_and_family(maker, mpn):
    if mpn.startswith(('NC7S', 'NC7SV', 'NL17SZ')):
        return 'single:' + mpn, ''
    if mpn in MIL:
        num, fam = MIL[mpn]
        return num, fam
    m = PAT.match(mpn)
    if not m:
        return None, None
    fam, num = m.group(1), m.group(2)
    if re.match(r'\d[GT]', num):
        return 'single:' + num, fam
    if mpn.startswith(('CD4', 'HEF4', 'HEC4', 'MC14', 'NLV14')):
        return num, 'C'
    if mpn.startswith('CLV'):
        fam = 'LVT'
    if mpn.startswith(('CY74FCT', '74FCT')):
        fam = 'FCT'
    if fam == '' and mpn.startswith('SN'):
        fam = 'TTL'
    return num, SUFFIX_FAMILY.get(fam, fam)


def grade(p):
    q = p.get('Qualification') or ''
    g = p.get('Grade') or ''
    return 'automotive' if 'AEC' in q or g == 'Automotive' else 'military' if g == 'Military' or 'MIL' in q else ''


rows = {}
for f in sorted(raw.glob('*.jsonl')):
    for line in f.open():
        d = json.loads(line)
        rows[(d['maker'], d['mpn'])] = d

existing = set()
for f in sorted(parts_dir.glob('*.json')):
    data = json.load(f.open())
    for dv in data.get('devices', []):
        existing.update(p['name'] for p in dv['parts'])
cols = ['mpn', 'maker', 'device', 'family', 'kind', 'scope', 'stock', 'status', 'grade',
        'dk_package', 'dk_supplier_package', 'bits', 'elements', 'output', 'function',
        'datasheet_url', 'pdf', 'ordering', 'verdict', 'footprint']
with out_path.open('w') as out:
    out.write('\t'.join(cols) + '\n')
    for (maker, mpn), d in sorted(rows.items()):
        p = d['params']
        num, fam = device_and_family(maker, mpn)
        dev, kind, scope = '', '', ''
        if num in REGISTER:
            dev, kind, scope = REGISTER[num], 'register', 'in'
        elif num in MUX:
            dev, kind, scope = MUX[num], 'mux', 'in'
        elif num in DFF:
            dev, kind = DFF[num], 'dff'
            scope = 'out:ti-existing-device' if maker == TI else 'in'
        elif num and num.startswith('single:'):
            scope = 'out:single-dual-or-translating-gate'
        elif num is None:
            scope = 'out:no-device-number'
        elif num.startswith('10') and maker == ON and mpn.startswith(('MC10', 'MC100', 'NB')):
            scope = 'out:ecl'
        elif num in ('72', '73', '74', '107', '109', '112', '013', '027', '4013', '4027',
                     '14013', '14027', '76'):
            scope = 'out:one-bit-flip-flop'
        else:
            scope = 'out:keyword-noise'
        if mpn.startswith(('MC10', 'MC100', 'NB4', 'NB7')):
            scope, dev, kind = 'out:ecl', '', ''
        if scope == 'in' and mpn in existing:
            scope = 'out:already-in-catalog'
        if scope == 'in' and mpn in PLANNING_OUT:
            scope = PLANNING_OUT[mpn]
        out.write('\t'.join(str(x) for x in [
            mpn, maker, dev, fam or '', kind, scope, d['stock'], d['status'], grade(p),
            p.get('Package / Case', ''), p.get('Supplier Device Package', ''),
            p.get('Number of Bits per Element', ''), p.get('Number of Elements', ''),
            p.get('Output Type', ''), p.get('Function', ''), d.get('datasheet') or '',
            '', '', '', '']) + '\n')
