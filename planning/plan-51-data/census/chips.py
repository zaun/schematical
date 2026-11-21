#!/usr/bin/env python3
"""plan-51: the chips a socket can hold.  usage: python3 -I chips.py [fits.tsv]

FITS maps a socket shape (family, pins, row spacing in inches) to the chip footprints of
Parts/footprints.json it holds; every chip footprint that names a pad per pin and sits in
a DIP or PLCC package is listed. Prints parts per footprint and the parts with an absent pin.
"""
import glob, json, os, sys

PARTS = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..',
                     'schematical', 'schematical', 'Parts')
FITS = {
    ('DIP', 8, '0.3'): ['PDIP-8', 'CDIP-8-JG'],
    ('DIP', 14, '0.3'): ['PDIP-14', 'CDIP-14'],
    ('DIP', 16, '0.3'): ['PDIP-16', 'CDIP-16'],
    ('DIP', 20, '0.3'): ['PDIP-20', 'CDIP-20'],
    ('DIP', 24, '0.3'): ['CDIP-24', 'CDIP-SB-24-JDN'],
    ('DIP', 28, '0.3'): ['PDIP-28-MC-28P3'],
    ('DIP', 24, '0.6'): ['CDIP-24-600'],
    ('DIP', 28, '0.6'): ['PDIP-28-600', 'CDIP-28-600'],
    ('DIP', 32, '0.6'): ['PDIP-32-600', 'CDIP-32-600'],
    ('DIP', 40, '0.6'): ['PDIP-40-600', 'CDIP-40-600'],
    ('DIP', 48, '0.6'): ['PDIP-48-600'],
    ('PLCC', 28, ''): ['PLCC-28'],
    ('PLCC', 32, ''): ['PLCC-32'],
    ('PLCC', 44, ''): ['PLCC-44', 'PLCC-44-MC-44J'],
    ('PLCC', 68, ''): ['PLCC-68'],
}

if __name__ == '__main__':
    fits = {f for ids in FITS.values() for f in ids}
    feet = {f['id']: f for f in json.load(open(os.path.join(PARTS, 'footprints.json')))['footprints']}
    for shape, ids in FITS.items():
        for i in ids:
            assert feet[i]['pins'] == shape[1], (shape, i)
    count, absent = {}, 0
    for path in sorted(glob.glob(os.path.join(PARTS, '*.json'))):
        if os.path.basename(path) in ('footprints.json', 'datasheets.json'):
            continue
        for dev in json.load(open(path))['devices']:
            for p in dev['parts']:
                if p.get('footprint') in fits:
                    count[p['footprint']] = count.get(p['footprint'], 0) + 1
                    absent += any(x.get('role') == 'absent' for x in dev['pinouts'][p['pinout']])
    print('chip footprints held by a socket:', len(fits), '; parts using them:', sum(count.values()),
          '; of them with an absent pin:', absent)
    for shape, ids in FITS.items():
        print(shape, [(i, count.get(i, 0)) for i in ids])
    if len(sys.argv) > 1:
        with open(sys.argv[1], 'w') as f:
            f.write('family\tpins\tspacing\tfits\n')
            for shape, ids in FITS.items():
                f.write('\t'.join([shape[0], str(shape[1]), shape[2], ','.join(ids)]) + '\n')
