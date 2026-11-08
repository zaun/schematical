#!/usr/bin/env python3
"""Write planning/plan-24-data/parts.tsv from the matched rows and device lists.

usage: build.py <rows-ADC.tsv> <devices-ADC.tsv> [<rows-DAC.tsv> <devices-DAC.tsv> ...]
Columns: group name device package code pins footprint flags status pdf decision reason
"""
import csv, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
IDS = os.path.join(DATA, 'footprint-ids.tsv')
FAMILY = {'PDIP': 'DIP', 'CDIP': 'DIP', 'CDIP SB': 'DIP', 'MSOP': 'VSSOP', 'SC70': 'SC-70'}
CERAMIC = {'CDIP', 'CDIP SB', 'CFP', 'LCCC', 'CPGA'}
fid = {}
for l in open(IDS):
    if l.startswith('#') or not l.strip():
        continue
    p = l.rstrip('\n').split('\t')
    fid[p[0]] = p[1]

def flags(r):
    f = []
    n = r['name']
    if r['poa_status'] == 'Obsolete':
        f.append('obsolete')
    if n.startswith('5962-') or n.startswith('V62/') and False:
        f.append('military')
    if n.endswith('Q1'):
        f.append('automotive')
    if n.endswith('EP') or n.startswith('V62/'):
        f.append('enhanced')
    if r['pkgtype'] in CERAMIC:
        f.append('ceramic')
    return ','.join(f)

# devices the spec writers found unaddable after reading the datasheet (plan-24-A Corrections)
DEVICE_SKIP = {
    'ADS114S18': 'short-form datasheet only (ads124s18.pdf, 7 pages: no pinout, registers or timing)',
    'ADS124S18': 'short-form datasheet only (ads124s18.pdf, 7 pages: no pinout, registers or timing)',
    'ADS5403': 'out of scope (interface: LVDS outputs; DigiKey lists Parallel)',
    'ADS5440': 'out of scope (interface: LVDS outputs; DigiKey lists Parallel)',
    'ADS5444': 'out of scope (interface: LVDS outputs; DigiKey lists Parallel)',
    'ADS5463': 'out of scope (interface: LVDS outputs; DigiKey lists Parallel)',
    'ADS54RF63': 'out of scope (interface: LVDS outputs; DigiKey lists Parallel)',
    'ADS5474': 'out of scope (interface: LVDS outputs; DigiKey lists Parallel)',
}

out = []
args = sys.argv[1:]
for rows_f, dev_f in zip(args[0::2], args[1::2]):
    device = {}
    for l in open(dev_f):
        cl, dev, pdfs, codes, parts = l.rstrip('\n').split('\t')
        for n in parts.split():
            device[n] = dev
    for r in csv.DictReader(open(rows_f), delimiter='\t'):
        m = r['match']
        code = r['code']
        fp = fid.get(f"{code}-{r['pins']}", '') if code else ''
        if m in ('own', 'other') and device.get(r['name']) in DEVICE_SKIP:
            dec, why = 'skip', DEVICE_SKIP[device[r['name']]]
        elif m in ('own', 'other'):
            dec, why = 'add', ('ordering table of ' + r['pdf'] if m == 'other' else '')
        elif m == 'status':
            dec, why = 'skip', f"status {r['status']} (only Active and Last Time Buy are added)"
        elif m == 'out' and r['interface'] == 'highspeed':
            dec, why = 'skip', f"out of scope (high-speed outputs: {r.get('dk_interface') or 'JESD204/LVDS'})"
        elif m == 'out':
            dec, why = 'skip', f"out of scope (no SPI, I2C or parallel interface: {r['interface'].split(':', 1)[1] or 'none listed'})"
        elif r['pdf']:
            dec, why = 'skip', f"not in the ordering table of {r['pdf']} (its package is not documented)"
        else:
            dec, why = 'skip', 'no TI datasheet (ti.com has no product page; DigiKey links a Rochester Electronics copy)'
        out.append([r['group'], r['name'], device.get(r['name'], '') if dec == 'add' or r['name'] in device else '',
                    FAMILY.get(r['pkgtype'], r['pkgtype']) if dec == 'add' else '',
                    code if dec == 'add' else '', r['pins'] if dec == 'add' else '',
                    fp if dec == 'add' else '', flags(r) if dec == 'add' else '',
                    r['status'], r['pdf'] if dec == 'add' else '', dec, why])
w = csv.writer(open(os.path.join(DATA, 'parts.tsv'), 'w'), delimiter='\t', lineterminator='\n')
w.writerow(['group', 'name', 'device', 'package', 'code', 'pins', 'footprint', 'flags', 'status',
            'pdf', 'decision', 'reason'])
w.writerows(out)
print(len(out), 'rows;', sum(1 for o in out if o[10] == 'add'), 'add')
