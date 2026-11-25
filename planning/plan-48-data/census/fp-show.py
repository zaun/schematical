#!/usr/bin/env python3
"""plan-48-C/D: print a footprint's pads (centre x, y, width, height, shape).
usage: python3 -I fp-show.py <id>... [--file <footprints-X.json>]
Looks the id up in footprints-C.json, footprints-D.json and
Parts/footprints.json. Expands `dual`, `quad`, `grid` and `explicit` the way
docs/parts-format.md § Footprints defines them (millimetres, origin at the
body centre, y down, pin 1 top of the left side)."""
import json
import os
import string
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
FILES = [os.path.join(ROOT, 'schematical', 'schematical', 'Parts', 'footprints.json'),
         os.path.join(DATA, 'footprints-C.json'), os.path.join(DATA, 'footprints-D.json')]
ROWS = [c for c in string.ascii_uppercase if c not in 'IOQSXZ']


def find(fid, files):
    for p in files:
        if os.path.exists(p):
            for o in json.load(open(p))['footprints']:
                if o['id'] == fid:
                    return o
    return None


def pads(o):
    st = o['style']
    out = []
    if st == 'dual':
        n = o['pins']
        per = o.get('perSide', n // 2)
        slots = o.get('slots') or list(range(1, n + 1))
        for i, s in enumerate(slots):
            if s == 0:
                continue
            if s <= per:
                x, y = -o['span'] / 2, (s - 1 - (per - 1) / 2) * o['pitch']
            else:
                x, y = o['span'] / 2, ((2 * per - s) - (per - 1) / 2) * o['pitch']
            out.append((str(i + 1), x, y, o['pad']['width'], o['pad']['height'], o['pad']['shape']))
        if 'centre' in o:
            c = o['centre']
            pin = '0' if 0 in slots else 'NULL'
            out.append((pin if pin != '0' else str(slots.index(0) + 1), 0.0, 0.0, c['width'], c['height'], c['shape']))
    elif st == 'quad':
        l, b, r, t = o['sides']
        slots = o.get('slots') or list(range(1, o['pins'] + 1))
        pos = {}
        for k in range(l):
            pos[1 + k] = (-o['spanX'] / 2, (k - (l - 1) / 2) * o['pitch'], False)
        for k in range(b):
            pos[1 + l + k] = ((k - (b - 1) / 2) * o['pitch'], o['spanY'] / 2, True)
        for k in range(r):
            pos[1 + l + b + k] = (o['spanX'] / 2, ((r - 1) / 2 - k) * o['pitch'], False)
        for k in range(t):
            pos[1 + l + b + r + k] = (((t - 1) / 2 - k) * o['pitch'], -o['spanY'] / 2, True)
        for i, s in enumerate(slots):
            if s == 0:
                continue
            x, y, turned = pos[s]
            w, h = (o['pad']['height'], o['pad']['width']) if turned else (o['pad']['width'], o['pad']['height'])
            out.append((str(i + 1), x, y, w, h, o['pad']['shape']))
        if 'centre' in o:
            c = o['centre']
            out.append((str(slots.index(0) + 1) if 0 in slots else 'centre', 0.0, 0.0, c['width'], c['height'], c['shape']))
    elif st == 'grid':
        for b in o['balls']:
            row = ROWS.index(b[0])
            col = int(b[1:])
            out.append((b, (col - 1 - (o['columns'] - 1) / 2) * o['pitch'], (row - (o['rows'] - 1) / 2) * o['pitch'],
                        o['pad']['width'], o['pad']['height'], o['pad']['shape']))
    else:
        for p in o['pads']:
            out.append((str(p.get('pin', p.get('ball'))), p['x'], p['y'], p['width'], p['height'], p['shape']))
    return out


def main(argv):
    files = FILES
    if '--file' in argv:
        i = argv.index('--file')
        files = [argv[i + 1]] + FILES
        argv = argv[:i] + argv[i + 2:]
    for fid in argv:
        o = find(fid, files)
        if o is None:
            print(fid, ': not found')
            continue
        print('%s  %s  body %sx%s  %s' % (fid, o['style'], o['body']['width'], o['body']['height'], o.get('description', '')))
        for k in ('pins', 'pitch', 'span', 'spanX', 'spanY', 'perSide', 'sides', 'slots', 'pad', 'centre', 'drill', 'note'):
            if k in o:
                print('   %s: %s' % (k, json.dumps(o[k])))
        for p in pads(o):
            print('   pad %-6s x %8.3f y %8.3f  %.3f x %.3f %s' % p)


if __name__ == '__main__':
    main(sys.argv[1:])
