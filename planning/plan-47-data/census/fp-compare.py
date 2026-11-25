#!/usr/bin/env python3
"""plan-47-D section 3.1: does an existing footprint equal a drawing's land pattern?

usage: python3 -I fp-compare.py <footprint id> <pad table.tsv>

<pad table> is a tab separated file with a header `pad x y w h shape drill` (mm, origin at the
body centre, x right, y down, drill empty or 0 for surface mount; shape rect or round). Prints
`same` (exit 0) when the footprint's pads equal the table's (centre, size, shape, drill, each
within 0.02 mm), else one line per difference (exit 1). The expanded pads come from
`footprint_check --print` (FOOTPRINT_CHECK, default /tmp/logic-p47-i/footprint_check: build
the backend there, `cmake --build /tmp/logic-p47-i --target footprint_check`).
"""
import csv
import os
import re
import subprocess
import sys

TOL = 0.02


def expanded(fid):
    exe = os.environ.get('FOOTPRINT_CHECK', '/tmp/logic-p47-i/footprint_check')
    out = subprocess.run([exe, '--print'], capture_output=True, text=True).stdout
    for line in out.splitlines():
        if line.startswith(fid + '|'):
            pads = {}
            for p in re.search(r'\|pads=(.*)$', line).group(1).split(';'):
                m = re.match(r'([^:]+):(-?[\d.]+),(-?[\d.]+),([\d.]+)x([\d.]+),(\w+)(?:,d([\d.]+))?', p)
                pads[m.group(1)] = (float(m.group(2)), float(m.group(3)), float(m.group(4)), float(m.group(5)),
                                    m.group(6), float(m.group(7) or 0))
            return pads
    return None


def main(fid, table):
    have = expanded(fid)
    if have is None:
        print('no footprint', fid)
        return 2
    want = {}
    for r in csv.DictReader(open(table, encoding='utf-8'), delimiter='\t'):
        want[r['pad']] = (float(r['x']), float(r['y']), float(r['w']), float(r['h']), r['shape'],
                          float(r['drill'] or 0))
    diffs = []
    for pad in sorted(set(have) | set(want), key=lambda p: (len(p), p)):
        if pad not in have or pad not in want:
            diffs.append(f"pad {pad}: only in the {'footprint' if pad in have else 'drawing'}")
            continue
        h, w = have[pad], want[pad]
        names = ('x', 'y', 'w', 'h')
        for i, n in enumerate(names):
            if abs(h[i] - w[i]) > TOL:
                diffs.append(f"pad {pad}: {n} {h[i]} against {w[i]}")
        if h[4] != w[4]:
            diffs.append(f"pad {pad}: shape {h[4]} against {w[4]}")
        if abs(h[5] - w[5]) > TOL:
            diffs.append(f"pad {pad}: drill {h[5]} against {w[5]}")
    print('same' if not diffs else '\n'.join(diffs))
    return 1 if diffs else 0


if __name__ == '__main__':
    sys.exit(main(*sys.argv[1:3]))
