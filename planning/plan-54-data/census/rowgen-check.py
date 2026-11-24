#!/usr/bin/env python3
"""plan-54-H § 3.3: the generated pads against the dimensions the drawings print.

usage: python3 -I rowgen-check.py <rowgen-params.json> <rowgen-ref.tsv> [--tol 0.02]

rowgen-ref.tsv (tab separated, header `id what value`): `id` a generated footprint id, `what` one of
  span0 / span1     first-to-last pad centre distance of side 0 / side 1 (side 0 = pins 1..; side 1 = the rest,
                    both read from the generated pads by their y: the pads at the first pad's y are side 0)
  rows              distance between the two rows of pads (side 1 y minus side 0 y)
  pad<pin>.x / .y   centre of the contact pin
  peg<k>.x / .y     centre of the k-th (1-based) pad of the row's `extras`
  pins              the footprint's pin count
and `value` the drawing's dimension (mm). Prints each difference above the tolerance and
`<n> dimensions, <d> differ`. For every variant the file should hold at least two position counts.
"""
import json
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import rowgen  # noqa: E402


def main(argv):
    tol = 0.02
    if '--tol' in argv:
        i = argv.index('--tol')
        tol = float(argv[i + 1])
        argv = argv[:i] + argv[i + 2:]
    params = json.load(open(argv[0]))
    fps = {f['id']: f for f in rowgen.build(params)}
    n_contacts = {}
    for row in params['rows']:
        for N in row['positions']:
            n_contacts[f"{row['stem']}-{N}{(row.get('ids') or {}).get(str(N), '')}"] = (int(N), len(row.get('extras', [])))
    n = bad = 0
    for line in open(argv[1]).read().splitlines()[1:]:
        if not line.strip():
            continue
        fid, what, value = line.split('\t')
        n += 1
        fp = fps.get(fid)
        if fp is None:
            print('unknown footprint', fid)
            bad += 1
            continue
        N, ex = n_contacts[fid]
        contacts = [p for p in fp['pads'] if p['pin'] >= 1 and p['pin'] <= N]
        pegs = fp['pads'][len(contacts):]
        got = None
        if what == 'pins':
            got = fp['pins']
        elif what in ('span0', 'span1'):
            y0 = contacts[0]['y']
            side0 = [p for p in contacts if abs(p['y'] - y0) < 1e-6]
            side1 = [p for p in contacts if abs(p['y'] - y0) >= 1e-6]
            side = side0 if what == 'span0' else side1
            got = (max(p['x'] for p in side) - min(p['x'] for p in side)) if side else 0
        elif what == 'rows':
            ys = sorted({p['y'] for p in contacts})
            got = ys[-1] - ys[0]
        elif what.startswith('pad'):
            pin, axis = what[3:].split('.')
            p = next(p for p in contacts if p['pin'] == int(pin))
            got = p[axis]
        elif what.startswith('peg'):
            k, axis = what[3:].split('.')
            got = pegs[int(k) - 1][axis]
        else:
            print('unknown quantity', what)
            bad += 1
            continue
        if abs(got - float(value)) > tol:
            print(f'{fid} {what}: generated {got:.3f}, drawing {float(value):.3f}')
            bad += 1
    print(f'{n} dimensions, {bad} differ')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
