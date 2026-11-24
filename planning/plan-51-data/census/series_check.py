#!/usr/bin/env python3
"""plan-51-C: checks series.tsv against candidates.tsv.
usage: python3 -I series_check.py [--maker NAME] [--series 110,114] [--file series.tsv]
(prints `ok`, or one line per problem; --maker / --series check only those groups, and skip the generic coverage)

series.tsv: one row per (maker, series, family, mounting, spacing) group of the candidates, tab separated.
Columns (every cell filled; `-` where a column does not apply):
  maker series family mounting spacing   the group (spacing `0.3` / `0.6` for DIP, `-` for PLCC)
  pins      the group's pin counts, comma separated (equal to the candidates' set)
  rule      R1 R2 R3 R5 (plan-03-B § Pad rules)
  drill     through-hole diameter, mm (`0` for surface mount)
  pad_w     pad size along x, mm: left/right-column pads of a `dual`; left/right pads of a `quad` (round pad: the diameter)
  pad_h     pad size along y, mm
  span      DIP: the centre-to-centre distance of the two columns, mm
            PLCC: `N:x/y;N:x/y` per pin count, spanX/spanY, mm
  pitch     2.54 (DIP) or 1.27 (PLCC)
  sides     PLCC: `N:l,b,r,t;...` the slot counts per side (they sum to N); `-` for DIP
  body_w    DIP: the housing width, mm; PLCC: `N:w;...` per pin count
  body_len  DIP: `N:len;...` the housing length per pin count; PLCC: `N:h;...` per pin count
  pin1      `rect` (pin 1 square pad, through hole) or `none`
  generic   `yes` when the group is the footprint source of the generic sockets (§ 3.3) for all its pin counts,
            a list of its pin counts (`68`, `28,32,44`) when only those, else `-`; every shape is supplied once
            A pad is round when pad_w equals pad_h, else a rect (surface mount DIP gull wings, PLCC J-leads).
  drawing   `<pdf file name> p.<page>` (the PDF is in datasheets/ic-socket/<maker folder>/)
  note      a free text: what was assumed (`-` for none)
"""
import os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.join(HERE, '..')
ROOT = os.path.join(DATA, '..', '..')
COLS = ['maker', 'series', 'family', 'mounting', 'spacing', 'pins', 'rule', 'drill', 'pad_w', 'pad_h', 'span',
        'pitch', 'sides', 'body_w', 'body_len', 'pin1', 'generic', 'drawing', 'note']
FOLDER = {'Mill-Max': 'mill-max', 'Preci-Dip': 'preci-dip', 'Assmann': 'assmann'}
SHAPES = {('DIP', '0.3'): {8, 14, 16, 20, 24, 28}, ('DIP', '0.6'): {24, 28, 32, 40, 48},
          ('PLCC', '-'): {28, 32, 44, 68}}
SIDES_OK = {28: 28, 32: 32, 44: 44, 68: 68}


def read_tsv(path):
    lines = [l.rstrip('\n').split('\t') for l in open(path)]
    return lines[0], lines[1:]


def number(text, what, problems, positive=True):
    try:
        v = float(text)
    except ValueError:
        problems.append(f'{what}: "{text}" is not a number')
        return None
    if positive and v <= 0:
        problems.append(f'{what}: {text} must be above 0')
    return v


def per_pin(text, what, problems, parse):
    """`N:value;N:value` -> {N: parsed}."""
    out = {}
    for part in text.split(';'):
        if ':' not in part:
            problems.append(f'{what}: "{part}" is not N:value')
            continue
        n, v = part.split(':', 1)
        if not n.isdigit():
            problems.append(f'{what}: "{n}" is not a pin count')
            continue
        out[int(n)] = parse(v)
    return out


def main():
    args = sys.argv[1:]
    maker = args[args.index('--maker') + 1] if '--maker' in args else None
    path = args[args.index('--file') + 1] if '--file' in args else os.path.join(DATA, 'series.tsv')
    only = set(args[args.index('--series') + 1].split(',')) if '--series' in args else None
    partial = bool(maker or only)
    problems = []
    _, cands = read_tsv(os.path.join(DATA, 'candidates.tsv'))
    groups = {}
    for c in cands:
        if maker and c[0] != maker:
            continue
        if only and c[2] not in only:
            continue
        if c[11] == 'out':      # a candidate marked out (C phase 2) needs no row
            continue
        key = (c[0], c[2], c[3], c[6], c[5] or '-')
        groups.setdefault(key, set()).add(int(c[4]))
    header, rows = read_tsv(path)
    if header != COLS:
        print('header must be:', '\t'.join(COLS))
        sys.exit(1)
    seen = {}
    for n, r in enumerate(rows, 2):
        where = f'series.tsv line {n}'
        if len(r) != len(COLS):
            problems.append(f'{where}: {len(r)} cells, expected {len(COLS)}')
            continue
        row = dict(zip(COLS, r))
        if maker and row['maker'] != maker:
            continue
        if only and row['series'] not in only:
            continue
        key = (row['maker'], row['series'], row['family'], row['mounting'], row['spacing'])
        where = f'{where} {key}'
        if key in seen:
            problems.append(f'{where}: a second row for the group')
        seen[key] = row
        if key not in groups:
            problems.append(f'{where}: no such group in candidates.tsv')
        for col in COLS:
            if row[col] == '':
                problems.append(f'{where}: empty {col}')
        if any(row[c] == '' for c in COLS):
            continue
        fam, mount = row['family'], row['mounting']
        if row['rule'] not in ('R1', 'R2', 'R3', 'R5'):
            problems.append(f'{where}: unknown rule {row["rule"]}')
        if mount == 'Through Hole' and row['rule'] not in ('R1', 'R5'):
            problems.append(f'{where}: a through-hole group uses R1 or R5')
        if mount == 'Surface Mount' and row['rule'] not in ('R1', 'R2', 'R3'):
            problems.append(f'{where}: a surface-mount group uses R1, R2 or R3')
        pins = {int(p) for p in row['pins'].split(',') if p.isdigit()}
        if pins != groups.get(key, pins):
            problems.append(f'{where}: pins {sorted(pins)} differ from the candidates {sorted(groups[key])}')
        if not pins <= SHAPES.get((fam, row['spacing']), set()):
            problems.append(f'{where}: pins {sorted(pins)} outside fits.tsv')
        drill = number(row['drill'], f'{where} drill', problems, positive=False)
        pw = number(row['pad_w'], f'{where} pad_w', problems)
        ph = number(row['pad_h'], f'{where} pad_h', problems)
        number(row['pitch'], f'{where} pitch', problems)
        if drill is not None and mount == 'Through Hole' and (drill <= 0 or (pw and ph and drill >= min(pw, ph))):
            problems.append(f'{where}: a through-hole drill must be above 0 and below both pad sides')
        if drill is not None and mount == 'Surface Mount' and drill != 0:
            problems.append(f'{where}: a surface-mount drill is 0')
        if row['pin1'] not in ('rect', 'none'):
            problems.append(f'{where}: pin1 is rect or none')
        generic = row['generic']
        if generic not in ('yes', '-') and not (generic.replace(',', '').isdigit() and
                                                 {int(p) for p in generic.split(',')} <= pins):
            problems.append(f'{where}: generic is yes, -, or pin counts of the row')
        pdf, _, page = row['drawing'].partition(' p.')
        folder = FOLDER.get(row['maker'], '')
        if not os.path.isfile(os.path.join(ROOT, 'datasheets', 'ic-socket', folder, pdf)) or not page.strip():
            problems.append(f'{where}: drawing "{row["drawing"]}" is not "<file in datasheets/ic-socket/{folder}> p.<page>"')
        if fam == 'DIP':
            number(row['span'], f'{where} span', problems)
            number(row['body_w'], f'{where} body_w', problems)
            lens = per_pin(row['body_len'], f'{where} body_len', problems, lambda v: number(v, f'{where} body_len', problems))
            if set(lens) != pins:
                problems.append(f'{where}: body_len names {sorted(lens)}, pins are {sorted(pins)}')
            if row['sides'] != '-':
                problems.append(f'{where}: a DIP has no sides')
        else:
            spans = per_pin(row['span'], f'{where} span', problems, lambda v: v)
            sides = per_pin(row['sides'], f'{where} sides', problems, lambda v: [int(x) for x in v.split(',') if x.isdigit()])
            bw = per_pin(row['body_w'], f'{where} body_w', problems, lambda v: number(v, f'{where} body_w', problems))
            bl = per_pin(row['body_len'], f'{where} body_len', problems, lambda v: number(v, f'{where} body_len', problems))
            for label, d in (('span', spans), ('sides', sides), ('body_w', bw), ('body_len', bl)):
                if set(d) != pins:
                    problems.append(f'{where}: {label} names {sorted(d)}, pins are {sorted(pins)}')
            for npins, v in spans.items():
                xy = v.split('/')
                if len(xy) != 2 or any(number(t, f'{where} span {npins}', problems) is None for t in xy):
                    problems.append(f'{where}: span {npins}:{v} is not x/y')
            for npins, v in sides.items():
                if len(v) != 4 or sum(v) != npins:
                    problems.append(f'{where}: sides {npins}:{v} must be four counts summing to {npins}')
    for key in groups:
        if key not in seen:
            problems.append(f'no row for the group {key} ({sorted(groups[key])} pins)')
    if not partial:
        covered = {}
        for key, row in seen.items():
            if row['generic'] == '-':
                continue
            supplied = row['pins'].split(',') if row['generic'] == 'yes' else row['generic'].split(',')
            for p in supplied:
                if int(p) in covered.setdefault((row['family'], row['spacing']), set()):
                    problems.append(f'generic: {int(p)} pins of {(row["family"], row["spacing"])} supplied twice ({key})')
                covered[(row['family'], row['spacing'])].add(int(p))
        for shape, pins in SHAPES.items():
            if covered.get(shape, set()) != pins:
                problems.append(f'generic rows cover {sorted(covered.get(shape, set()))} of {shape}, need {sorted(pins)}')
    if problems:
        print('\n'.join(problems))
        sys.exit(1)
    print('ok')


main()
