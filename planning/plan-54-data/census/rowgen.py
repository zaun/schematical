#!/usr/bin/env python3
"""plan-54-H § 3.3: the row generator for module and card-edge sockets.

usage: python3 -I rowgen.py <rowgen-params.json> > footprints-<L>.json
       python3 -I rowgen.py --list <rowgen-params.json>        the ids it would write, one per line

The parameter file is `{"rows": [<row>, ...]}`; one row is one footprint *variant* (a datasheet drawing with a
pitch, rows and key) and lists the position counts `positions` it serves. For every count N the generator writes an
`explicit` footprint `<stem>-<N>` (plus the optional suffix of a position, `"ids": {"288": "-B"}`).

Row keys (all mm; a number, or a string expression over `N` (the key's positions), `P` (the footprint's pins), `m0`
and `m1` (pads on side 0 and side 1), `L0` and `L1` (first-to-last pad centre distance of each side), `pitch`):

  stem         the id stem (`EDGE-SAMTEC-MEC8`); the id is `<stem>-<N>`
  positions    [N, ...]
  pins         the footprint's `pins` (default N; N + 1 when a SHIELD pin follows the contacts)
  description  text; `{N}` is replaced by the count
  note         text (the drawing, PDF and page; every number read off it); `{N}` replaced
  numbering    `halves` (pins 1..ceil(N/2) on side 0, the rest on side 1), `odd-even` (odd pins on side 0, even on
               side 1), `single` (every contact on side 0); the contacts are pins 1..N
  pitch        pad pitch along a side (mm)
  dir          `ltr` or `rtl` per side, `"ltr,rtl"`: pin order of that side runs left to right or right to left
  row_y        y of side 0, then of side 1 (`"-2.1,2.1"`); one number for `single`
  row_layer    `same` or `opposite` per side (`"same,opposite"`): an `opposite` side's pads get `"opposite": true`
  edge_mount   1 writes `"edgeMount": true`
  pad_w, pad_h pad size (x by y); `"0.5,0.6"` gives side 0 and side 1 their own
  shape        `rect` or `round`; drill 0 (surface mount) or the through-hole drill
  stagger      y offset added to every second pad of a side (staggered rows), default 0; `"a,b"` per side
  x_off        x shift of a side, default 0; `"a,b"` per side (a row offset by half a pitch)
  gaps         `{"<after pad index, 1-based>": <extra mm>, ...}` per side as `[{...}, {...}]` or one object for both:
               extra distance after that pad of the side (a key notch)
  extras       a list of `explicit` pads (pegs, tabs, holes, shield tabs) in the footprint's coordinates; `x`, `y`,
               `pin` may be expressions; every other key as in `docs/parts-format.md`
  body_w, body_h  the housing outline (expressions)

Pad `i` (0-based) of a side with `m` pads sits at x = -(span / 2) + i * pitch + (the gaps before it), `span` = (m - 1) *
pitch + all gaps of the side, so the pads of a side are centred on x = 0; `rtl` mirrors the side. One generator row
therefore describes every position count of a variant; `rowgen-check.py` compares the generated pads with the
dimensions the drawing prints.
"""
import ast
import json
import math
import operator
import sys

OPS = {ast.Add: operator.add, ast.Sub: operator.sub, ast.Mult: operator.mul, ast.Div: operator.truediv,
       ast.FloorDiv: operator.floordiv, ast.Mod: operator.mod, ast.Pow: operator.pow}
FUNCS = {'min': min, 'max': max, 'abs': abs, 'round': round, 'ceil': math.ceil, 'floor': math.floor, 'int': int}


def ev(node, env):
    if isinstance(node, ast.Expression):
        return ev(node.body, env)
    if isinstance(node, ast.Constant) and isinstance(node.value, (int, float)):
        return node.value
    if isinstance(node, ast.Name) and node.id in env:
        return env[node.id]
    if isinstance(node, ast.UnaryOp) and isinstance(node.op, (ast.USub, ast.UAdd)):
        v = ev(node.operand, env)
        return -v if isinstance(node.op, ast.USub) else v
    if isinstance(node, ast.BinOp) and type(node.op) in OPS:
        return OPS[type(node.op)](ev(node.left, env), ev(node.right, env))
    if isinstance(node, ast.Call) and isinstance(node.func, ast.Name) and node.func.id in FUNCS and not node.keywords:
        return FUNCS[node.func.id](*[ev(a, env) for a in node.args])
    raise ValueError(f'unsupported expression {ast.dump(node)}')


def num(v, env):
    if isinstance(v, (int, float)):
        return v
    return ev(ast.parse(str(v), mode='eval'), env)


def per_side(v, default=None):
    """'a,b' / [a, b] / a  ->  (a, b)"""
    if v is None:
        return (default, default)
    if isinstance(v, str) and ',' in v:
        a, b = [x.strip() for x in v.split(',')]
        return (a, b)
    if isinstance(v, (list, tuple)):
        return (v[0], v[1] if len(v) > 1 else v[0])
    return (v, v)


def r3(x):
    x = round(float(x), 3)
    return 0.0 if x == 0 else x


def side_of(numbering, N, p):
    """(side, index) of contact pin p (1-based)."""
    m0 = (N + 1) // 2
    if numbering == 'single':
        return 0, p - 1
    if numbering == 'odd-even':
        return (0, (p - 1) // 2) if p % 2 else (1, p // 2 - 1)
    if numbering == 'halves':
        return (0, p - 1) if p <= m0 else (1, p - m0 - 1)
    raise ValueError(f'unknown numbering {numbering}')


def sides_count(numbering, N):
    if numbering == 'single':
        return N, 0
    m0 = (N + 1) // 2
    return m0, N - m0


def side_x(i, m, pitch, gaps, rtl):
    """x of pad i (0-based) of a side with m pads (not mirrored for rtl yet)."""
    total = (m - 1) * pitch + sum(gaps.values())
    x = -total / 2 + i * pitch + sum(g for k, g in gaps.items() if k <= i)   # gap after pad k (1-based): pads with index >= k shift
    return -x if rtl else x


def footprint(row, N):
    numbering = row.get('numbering', 'halves')
    pitch = float(row['pitch'])
    m0, m1 = sides_count(numbering, N)
    env = {'N': N, 'pitch': pitch, 'm0': m0, 'm1': m1}
    env['P'] = num(row.get('pins', 'N'), env)
    gaps_raw = row.get('gaps') or {}
    gaps_sides = gaps_raw if isinstance(gaps_raw, list) else [gaps_raw, gaps_raw]
    gaps = [{int(k): float(v) for k, v in g.items()} for g in gaps_sides]
    gaps = [{k: v for k, v in g.items() if k < (m0, m1)[s]} for s, g in enumerate(gaps)]
    dirs = per_side(row.get('dir', 'ltr'))
    ys = [num(y, env) for y in per_side(row.get('row_y', 0))]
    layers = per_side(row.get('row_layer', 'same'))
    pw = [num(x, env) for x in per_side(row['pad_w'])]
    ph = [num(x, env) for x in per_side(row['pad_h'])]
    shape = row.get('shape', 'rect')
    drill = float(row.get('drill', 0))
    stagger = [float(v) for v in per_side(row.get('stagger', 0), 0)]   # `"a,b"`: per side
    x_off = [float(v) for v in per_side(row.get('x_off', 0), 0)]       # `"a,b"`: x shift of a side (offset rows)
    spans = []
    for s, m in enumerate((m0, m1)):
        spans.append(((m - 1) * pitch + sum(gaps[s].values())) if m > 0 else 0)
    env['L0'], env['L1'] = spans
    pads = []
    for p in range(1, N + 1):
        s, i = side_of(numbering, N, p)
        m = (m0, m1)[s]
        x = side_x(i, m, pitch, gaps[s], dirs[s] == 'rtl') + x_off[s]
        y = ys[s] + (stagger[s] if i % 2 else 0)
        pad = {'pin': p, 'x': r3(x), 'y': r3(y), 'width': r3(pw[s]), 'height': r3(ph[s]), 'shape': shape}
        if drill:
            pad['drill'] = drill
        if layers[s] == 'opposite':
            pad['opposite'] = True
        pads.append(pad)
    for ex in row.get('extras', []):
        e = dict(ex)
        for k in ('x', 'y', 'pin', 'width', 'height', 'drill', 'slotLength'):
            if k in e:
                e[k] = r3(num(e[k], env)) if k != 'pin' else int(num(e[k], env))
        pads.append(e)
    fp = {'id': f"{row['stem']}-{N}{(row.get('ids') or {}).get(str(N), '')}",
          'description': row.get('description', row['stem']).replace('{N}', str(N)),
          'note': row.get('note', '').replace('{N}', str(N)),
          'style': 'explicit', 'pins': int(env['P'])}
    if not fp['note']:
        del fp['note']
    fp['body'] = {'width': r3(num(row['body_w'], env)), 'height': r3(num(row['body_h'], env))}
    if row.get('edge_mount'):
        fp['edgeMount'] = True
    fp['pads'] = pads
    return fp


def build(params):
    out = []
    for row in params['rows']:
        for N in row['positions']:
            out.append(footprint(row, int(N)))
    ids = [f['id'] for f in out]
    dup = sorted({i for i in ids if ids.count(i) > 1})
    if dup:
        raise SystemExit('rowgen: duplicate ids ' + ', '.join(dup))
    return out


def dump(fps):
    lines = ',\n'.join('    ' + json.dumps(f, ensure_ascii=False, separators=(',', ':')) for f in fps)
    return '{\n  "format": "schematical-footprints",\n  "version": 1,\n  "footprints": [\n' + lines + '\n  ]\n}\n'


def main(argv):
    if argv and argv[0] == '--list':
        for f in build(json.load(open(argv[1]))):
            print(f['id'])
        return 0
    sys.stdout.write(dump(build(json.load(open(argv[0])))))
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
