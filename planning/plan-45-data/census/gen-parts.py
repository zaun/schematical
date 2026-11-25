#!/usr/bin/env python3
"""plan-45-G § 3: write the terminal-block and JST parts into Parts/connector.json and Parts/datasheets.json.

usage: python3 -I gen-parts.py --generics-only
       python3 -I gen-parts.py --maker phoenix --maker weidmuller     the generics and those makers' `add` rows
       python3 -I gen-parts.py                                        the generics and every maker
       python3 -I gen-parts.py --check [same options]                 exit 1 when the files differ from what the run would write

connector.json is split into its device blocks (their `    {` ... `    }` lines); every block whose id starts with
`tb-`, `jst-`, `generic-terminal-` or `generic-jst-` is dropped, every other block is kept byte for byte, and the
generated blocks (sorted by id) are appended. datasheets.json: the entries this run names replace the same ids,
and the new ones are appended (sorted by id) before the closing lines; no other entry changes. Footprints are not
written here: `node scripts/gen-terminals.js --write` writes the ones the parts name.

One device per (variant, rated voltage, rated current) of the `add` rows of candidates.tsv (§ 3.2); the 88
generics (a device per terminal pitch and per generic JST series, 11 position counts each, the footprint of
generic-reps.tsv's representative variant).
"""
import collections
import csv
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
PARTS = os.path.join(ROOT, 'schematical', 'schematical', 'Parts')
CONNECTOR = os.path.join(PARTS, 'connector.json')
DATASHEETS = os.path.join(PARTS, 'datasheets.json')

MAKER_SLUG = {'Phoenix Contact': 'phoenix', 'Weidmuller': 'weidmuller', 'Wurth Elektronik': 'wurth', 'JST': 'jst'}
MAKER_NAME = {'Phoenix Contact': 'Phoenix Contact', 'Weidmuller': 'Weidmüller', 'Wurth Elektronik': 'Würth Elektronik', 'JST': 'JST'}
CLAMP = {'screw': 'Screw', 'spring': 'Spring', 'push-in': 'Push-in', 'pluggable': 'Plug-in header'}
PACKAGE = {('tht', 'vertical'): 'Through hole vertical', ('tht', 'horizontal'): 'Through hole right angle',
           ('tht', 'angled'): 'Through hole angled', ('smd', 'vertical'): 'SMD vertical', ('smd', 'horizontal'): 'SMD right angle'}
GENERATED = ('tb-', 'jst-', 'generic-terminal-', 'generic-jst-')
PITCHES = ['3.50', '3.81', '5.00', '5.08', '7.50', '7.62']


def number(text):
    """'17.5' -> 17.5, '400.0' -> 400, '' -> None."""
    if text in ('', None):
        return None
    v = float(text)
    if v == 0:
        return None
    return int(v) if v == int(v) else v


def jnum(v):
    return json.dumps(v)


def read(path):
    return list(csv.DictReader(open(path), delimiter='\t'))


def footprint_id(letter, label, n):
    return f"{'' if letter == 'E' else 'TB-'}{label}-{n:02d}"


def pinout_json(n):
    pins = ','.join(f'{{"number":{i},"name":"{i}","role":"inout"}}' for i in range(1, n + 1))
    return f'"p{n:02d}":[{pins}]'


def part_json(p):
    return json.dumps(p, ensure_ascii=False, separators=(',', ':'))


def device_block(d):
    """The device as the four-space block of connector.json (the keys one per line, the pinouts and parts compact)."""
    lines = ['    {']
    lines.append(f'      "id": {json.dumps(d["id"])},')
    lines.append(f'      "function": {json.dumps(d["function"], ensure_ascii=False)},')
    lines.append(f'      "description": {json.dumps(d["description"], ensure_ascii=False)},')
    lines.append(f'      "drawer": {json.dumps(d["drawer"], ensure_ascii=False)},')
    lines.append(f'      "note": {json.dumps(d["note"], ensure_ascii=False)},')
    lines.append(f'      "{d["mode"]}": {json.dumps(d["kind_value"])},')
    lines.append(f'      "pitch": {jnum(d["pitch"])},')
    if d.get('volts') is not None:
        lines.append(f'      "ratedVolts": {jnum(d["volts"])},')
    if d.get('amps') is not None:
        lines.append(f'      "ratedAmps": {jnum(d["amps"])},')
    lines.append('      "pinouts": {' + ','.join(pinout_json(n) for n in sorted(d['pins'])) + '},')
    lines.append('      "parts": [')
    parts = sorted(d['parts'], key=lambda p: p['name'])
    for i, p in enumerate(parts):
        lines.append('        ' + part_json(p) + (',' if i < len(parts) - 1 else ''))
    lines.append('      ]')
    lines.append('    }')
    return lines


def build(makers):
    # footprints-needed.txt: letter, variant, ..., label (column 11)
    label, letter = {}, {}
    for line in open(os.path.join(DATA, 'footprints-needed.txt')):
        if line.startswith('#') or not line.strip():
            continue
        f = line.rstrip('\n').split('\t')
        letter[f[1]], label[f[1]] = f[0], f[10]
    reps = {r['variant']: r for r in read(os.path.join(DATA, 'generic-reps.tsv'))}
    cand = [r for r in read(os.path.join(DATA, 'candidates.tsv')) if r['state'] == 'add']

    # the datasheet of every placed file: id = the file's lower-case stem, url = the shortest maker URL it was fetched under
    urls = collections.defaultdict(set)
    for folder in ('datasheets/connector-terminal/phoenix', 'datasheets/connector-terminal/weidmuller',
                   'datasheets/connector-terminal/wurth', 'datasheets/connector-jst'):
        for r in read(os.path.join(ROOT, folder, 'datasheets.tsv')):
            urls[r['path']].add(r['url'])
    sheets = {}
    devices = {}
    for r in cand:
        if makers is not None and MAKER_SLUG[r['maker']] not in makers:
            continue
        v, lab = r['variant'], label[r['variant']]
        n = int(r['positions'])
        volts, amps = number(r['volts']), number(r['amps'])
        jst = r['group'] == 'jst'
        sid = os.path.splitext(os.path.basename(r['pdf']))[0].lower()
        url = min(urls[r['pdf']], key=lambda u: (len(u), u))
        if sheets.setdefault(sid, (url, r['pdf']))[1] != r['pdf']:
            raise SystemExit(f'datasheet id {sid} names two files')
        key = (v, volts, amps)
        d = devices.get(key)
        if d is None:
            tok = lambda x: 'nr' if x is None else str(x)
            pitch = float(r['pitch'])
            orient = r['orientation']
            if jst:
                series = r['series']
                drawer = ['Connector', 'JST', series.replace('/', '-')]
                function = 'Top entry' if orient == 'vertical' else 'Side entry'
                desc = f"JST {series} {pitch:g} mm board header, {function.lower()}, {'through hole' if r['mounting'] == 'tht' else 'surface mount'}"
                mode, kind_value = 'jst', series.split('/')[0]
                did = f"{lab.lower()}-{tok(amps)}a-{tok(volts)}v"
            else:
                drawer = ['Connector', 'Terminal block', f'{pitch:.2f} mm']
                function = f"{CLAMP[r['class']]}, {orient}"
                what = ('board-side header of a plug-in terminal block' if r['class'] == 'pluggable'
                        else f"PCB terminal block, {r['class']} clamp")
                desc = f"{MAKER_NAME[r['maker']]} {r['series']} {pitch:.2f} mm {what}, {orient}"
                mode, kind_value = 'terminal', r['class']
                did = f"tb-{lab.lower()}-{tok(amps)}a-{tok(volts)}v"
            d = devices[key] = {'id': did, 'function': function, 'description': desc, 'drawer': drawer,
                                'mode': mode, 'kind_value': kind_value, 'pitch': pitch if pitch != int(pitch) else int(pitch),
                                'volts': volts, 'amps': amps, 'pins': set(), 'parts': [], 'files': set(), 'maker': r['maker'],
                                'series': r['series']}
        d['pins'].add(n)
        d['files'].add(os.path.basename(r['pdf']))
        d['parts'].append({'name': r['mpn'], 'manufacturer': MAKER_NAME[r['maker']], 'datasheet': sid,
                           'package': PACKAGE[(r['mounting'], orient if orient in ('vertical', 'horizontal', 'angled') else 'vertical')],
                           'pinout': f'p{n:02d}', 'footprint': footprint_id(letter[v], lab, n)})
    for d in devices.values():
        folder = 'connector-jst' if d['mode'] == 'jst' else f"connector-terminal/{MAKER_SLUG[d['maker']]}"
        files = sorted(d['files'])
        d['note'] = (f"{MAKER_NAME[d['maker']]} {d['series']}, datasheets/{folder}/" +
                     (files[0] if len(files) == 1 else '<item>.pdf (one per part)'))
        d['note'] += '; rating from the datasheet' if (d['volts'] is not None or d['amps'] is not None) else '; the datasheet prints no rating'
    out = list(devices.values())
    # the generics (always written): the representative variant's footprint at 2..12 positions (generic-reps.tsv)
    allrows = [r for r in read(os.path.join(DATA, 'candidates.tsv')) if r['state'] == 'add']
    for p in PITCHES:
        variant = next(r['variant'] for r in reps.values() if r['pitch'] == p and not r['variant'].startswith('JST'))
        sample = next(r for r in allrows if r['variant'] == variant)
        out.append(generic('terminal', p, variant, label[variant], letter[variant], sample))
    for series, pitch in (('XH', '2.50'), ('PH', '2.00')):
        variant = next(r['variant'] for r in reps.values() if r['pitch'] == pitch and r['variant'].startswith('JST'))
        sample = next(r for r in allrows if r['variant'] == variant)
        out.append(generic('jst', pitch, variant, label[variant], letter[variant], sample, series))
    out.sort(key=lambda d: d['id'])
    return out, sheets


def generic(mode, pitch, variant, lab, let, sample, series=None):
    pitch_f = float(pitch)
    parts = []
    for n in range(2, 13):
        name = f'GENERIC-TERMINAL-{pitch}-{n:02d}' if mode == 'terminal' else f'GENERIC-JST-{series}-{n:02d}'
        parts.append({'name': name, 'manufacturer': 'Generic', 'package': PACKAGE[(sample['mounting'], sample['orientation'])],
                      'pinout': f'p{n:02d}', 'footprint': footprint_id(let, lab, n)})
    base = {'function': 'Generic', 'pins': set(range(2, 13)), 'parts': parts, 'volts': None, 'amps': None}
    if mode == 'terminal':
        return dict(base, id=f'generic-terminal-{pitch}', description=f'Generic {pitch} mm PCB terminal block, 2 to 12 positions',
                    drawer=['Connector', 'Terminal block', f'{pitch} mm'], mode='terminal', kind_value=sample['class'],
                    pitch=pitch_f if pitch_f != int(pitch_f) else int(pitch_f),
                    note=f'generic; the footprint of {MAKER_NAME[sample["maker"]]} {sample["series"]} ({lab}); plan-45-G')
    return dict(base, id=f'generic-jst-{series.lower()}', description=f'Generic JST {series} {pitch_f:g} mm board header, 2 to 12 positions',
                drawer=['Connector', 'JST', series], mode='jst', kind_value=series, pitch=pitch_f,
                note=f'generic; the footprint of JST {series} ({lab}); plan-45-G')


def connector_text(devices, old):
    lines = old.split('\n')
    open_at = next(i for i, l in enumerate(lines) if l == '  "devices": [')
    end = len(lines) - 1
    while lines[end] != '  ]':
        end -= 1
    blocks, cur = [], None
    for l in lines[open_at + 1:end]:
        if l == '    {':
            cur = [l]
        elif cur is not None:
            cur.append(l)
            if l in ('    }', '    },'):
                blocks.append(cur)
                cur = None
    if cur is not None:
        raise SystemExit('connector.json: an unterminated device block')
    kept = []
    for b in blocks:
        did = next(re.match(r'      "id": "([^"]+)",', l).group(1) for l in b if l.startswith('      "id": '))
        if not did.startswith(GENERATED):
            kept.append([b[0]] + b[1:-1] + ['    }'])
    kept += [device_block(d) for d in devices]
    body = []
    for i, b in enumerate(kept):
        body += b[:-1] + ['    }' + (',' if i < len(kept) - 1 else '')]
    return '\n'.join(lines[:open_at + 1] + body + lines[end:])


def datasheets_text(sheets, old):
    lines = old.split('\n')
    end = len(lines) - 1
    while lines[end] != '  ]':
        end -= 1
    open_at = next(i for i, l in enumerate(lines) if l == '  "datasheets": [')
    entries = [l.rstrip(',') for l in lines[open_at + 1:end]]
    # the file's own spacing: {"id": "...", "url": "..."}
    new = {sid: '    {"id": ' + json.dumps(sid) + ', "url": ' + json.dumps(url) + '}' for sid, (url, _) in sheets.items()}
    kept = [e for e in entries if json.loads(e.strip())['id'] not in new]
    kept += [new[sid] for sid in sorted(new)]
    return '\n'.join(lines[:open_at + 1] + [e + (',' if i < len(kept) - 1 else '') for i, e in enumerate(kept)] + lines[end:])


def main():
    argv = sys.argv[1:]
    makers = None
    if '--generics-only' in argv:
        makers = set()
    if '--maker' in argv:
        makers = {argv[i + 1] for i, a in enumerate(argv) if a == '--maker'}
    devices, sheets = build(makers)
    old_c, old_d = open(CONNECTOR).read(), open(DATASHEETS).read()
    new_c, new_d = connector_text(devices, old_c), datasheets_text(sheets, old_d)
    json.loads(new_c)
    json.loads(new_d)
    if '--check' in argv:
        bad = [n for n, a, b in (('connector.json', old_c, new_c), ('datasheets.json', old_d, new_d)) if a != b]
        for n in bad:
            print(f'{n} differs from what gen-parts.py would write')
        sys.exit(1 if bad else 0)
    open(CONNECTOR, 'w').write(new_c)
    open(DATASHEETS, 'w').write(new_d)
    nparts = sum(len(d['parts']) for d in devices)
    print(f'{len(devices)} devices, {nparts} parts, {len(sheets)} datasheet entries')


main()
