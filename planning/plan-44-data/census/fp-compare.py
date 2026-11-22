#!/usr/bin/env python3
"""plan-44-C § 3.1: does an existing footprint's land pattern equal a drawing's?

usage: python3 -I fp-compare.py <footprint id> <pad table file | ->
       python3 -I fp-compare.py --pads <footprint id>         print the footprint's pads

The pad table has one pad per line: `pin x y width height shape` (mm, the
footprint's own axes: origin at the body centre, x right, y down; shape rect
or round; `#` starts a comment). Prints the differences against the library's
footprint (Parts/footprints.json; only the `dual` and `explicit` styles are
expanded) and `equal` when every pad of one has a pad of the other with the
same pin and shape and a centre, width and height within 0.02 mm. Exit 0 when
equal.
"""
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
LIBRARY = os.path.join(ROOT, 'schematical', 'schematical', 'Parts', 'footprints.json')
TOL = 0.02


def expand(fp):
    """[(pin, x, y, w, h, shape)] of a `dual` or `explicit` footprint."""
    if fp['style'] == 'explicit':
        return [(str(p.get('pin', p.get('ball'))), p['x'], p['y'], p['width'], p['height'], p['shape']) for p in fp['pads']]
    if fp['style'] != 'dual':
        raise SystemExit(f"fp-compare.py: style {fp['style']} is not expanded")
    pins = fp['pins']
    per = fp.get('perSide', pins // 2)
    slots = fp.get('slots') or list(range(1, pins + 1))
    pad = fp['pad']
    out = []
    for i, s in enumerate(slots):
        if s == 0:
            continue
        if s <= per:
            x, y = -fp['span'] / 2, (s - 1 - (per - 1) / 2) * fp['pitch']
        else:
            x, y = fp['span'] / 2, ((2 * per - s) - (per - 1) / 2) * fp['pitch']
        out.append((str(i + 1), x, y, pad['width'], pad['height'], pad['shape']))
    if fp.get('centre'):
        c = fp['centre']
        out.append(('0', 0.0, 0.0, c['width'], c['height'], c['shape']))
    return out


def library(fid):
    for fp in json.load(open(LIBRARY))['footprints']:
        if fp['id'] == fid:
            return fp
    raise SystemExit(f'fp-compare.py: no footprint {fid} in {LIBRARY}')


def read_table(path):
    out = []
    for line in (sys.stdin if path == '-' else open(path)):
        line = line.split('#')[0].strip()
        if line:
            pin, x, y, w, h, shape = line.split()
            out.append((pin, float(x), float(y), float(w), float(h), shape))
    return out


def diff(a, b):
    """Differences of footprint pads `a` against drawing pads `b`."""
    out = []
    left = list(b)
    for pa in a:
        match = [pb for pb in left if pb[0] == pa[0] and pb[5] == pa[5] and all(abs(pa[k] - pb[k]) <= TOL for k in (1, 2, 3, 4))]
        if match:
            left.remove(match[0])
        else:
            near = [pb for pb in left if pb[0] == pa[0]]
            out.append(f"footprint pad {pa[0]} {pa[1]:.3f},{pa[2]:.3f} {pa[3]:.3f}x{pa[4]:.3f} {pa[5]}: "
                       + (f"drawing has {near[0][1]:.3f},{near[0][2]:.3f} {near[0][3]:.3f}x{near[0][4]:.3f} {near[0][5]}" if near else 'no such pad in the drawing'))
    for pb in left:
        if not any(pa[0] == pb[0] for pa in a):
            out.append(f'drawing pad {pb[0]} has no pad in the footprint')
    return out


def main(argv):
    if argv[0] == '--pads':
        for p in expand(library(argv[1])):
            print(' '.join(str(round(v, 4)) if isinstance(v, float) else str(v) for v in p))
        return 0
    d = diff(expand(library(argv[0])), read_table(argv[1]))
    for line in d:
        print(line)
    print('equal' if not d else f'{len(d)} differences')
    return 1 if d else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
