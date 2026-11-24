#!/usr/bin/env python3
"""plan-54-M / N: the generator of the card-slot devices, parts and datasheet entries.

usage: python3 -I gen-parts.py [--family socket|module|edge]... [--maker <slug>]... [--check] [--examples]

Generates, from the `add` rows of candidates.tsv and their spec lines (specs.tsv), one `connector` device per footprint line
(footprint-lines.py: a maker's drawing / variant) and per spec line it serves, with one pinout `p<NN>` per pin count and
one part per row, into schematical/schematical/Parts/connector.json; and one entry per distinct datasheet into
datasheets.json. It is idempotent: it drops the blocks of `connector.json` whose `id` starts with
`card-<family>-<maker slug>-` for the (family, maker) pairs it is asked to generate (all pairs when none is named) and writes them
again, keeping every other block byte for byte; the new blocks go after the last block, sorted by id; `datasheets.json`
lines are inserted before the first existing id that sorts after them (an id of the pairs already present is kept).
--check writes nothing and exits 1 when either file differs from what would be written. --examples writes
planning/plan-54-data/examples.tsv (the most-stocked `add` row of each family, and of the straddle-mount kind).

Families: socket = memory / SIM card sockets (candidates family `card`), module, edge. Maker slugs: same-sky, amphenol, gct,
te, molex, samtec. The footprint of a part is `<stem>-<N>` (N its DigiKey positions) unless footprint-map-<letter>.tsv names
another id; pins that DigiKey leaves out (a SHIELD) or counts differently (`pins n, digikey m`) are in the pinout.
"""
import collections
import csv
import glob
import importlib.util
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import specs  # noqa: E402

PARTS_DIR = os.path.join(specs.ROOT, 'schematical', 'schematical', 'Parts')
CONNECTOR = os.path.join(PARTS_DIR, 'connector.json')
DATASHEETS = os.path.join(PARTS_DIR, 'datasheets.json')
FOOTPRINTS = os.path.join(PARTS_DIR, 'footprints.json')
FAMILY = {'socket': 'card', 'module': 'module', 'edge': 'edge'}
WORD = {'card': 'socket', 'module': 'module', 'edge': 'edge'}
SLUG = {'Same Sky': 'same-sky', 'Amphenol ICC': 'amphenol', 'GCT': 'gct', 'TE Connectivity': 'te', 'Molex': 'molex',
        'Samtec': 'samtec'}
MANUFACTURER = {'Same Sky': 'Same Sky', 'Amphenol ICC': 'Amphenol', 'GCT': 'GCT', 'TE Connectivity': 'TE Connectivity',
                'Molex': 'Molex', 'Samtec': 'Samtec'}
PACKAGE = {'Surface Mount, Right Angle': 'SMD right angle', 'Surface Mount': 'SMD vertical',
           'Through Hole': 'Through hole vertical', 'Through Hole, Right Angle': 'Through hole right angle',
           'Surface Mount, Through Hole': 'Hybrid vertical', 'Surface Mount, Right Angle; Through Hole': 'Hybrid right angle',
           'Board Edge, Straddle Mount': 'Board edge straddle'}
LEAF = {'SD': 'SD', 'microSD': 'microSD', 'microSD / UFS': 'microSD', 'SIM': 'SIM', 'Micro SIM': 'SIM', 'Nano SIM': 'SIM',
        'Mini SIM': 'SIM', 'SIM / SAM': 'SIM', 'Smart card': 'Smart card'}


def load_module(name):
    spec = importlib.util.spec_from_file_location(name.replace('-', '_'), os.path.join(specs.HERE, name + '.py'))
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


fplines = load_module('footprint-lines')


def footprint_map():
    out = {}
    for path in glob.glob(os.path.join(specs.DATA, 'footprint-map-*.tsv')) + glob.glob(os.path.join(specs.DATA, 'fp-parts', '*.map.tsv')):
        for r in specs.read_rows(path):
            if r.get('footprint'):
                out[(r['stem'], int(r['positions']))] = r['footprint']
    return out


def datasheet_id(pdf, taken, maker):
    base = os.path.splitext(os.path.basename(pdf))[0].lower()
    return re.sub(r'[^a-z0-9._-]', '-', base)


def pins_json(names, n):
    out = []
    half = (n + 1) // 2
    for i, name in enumerate(names, 1):
        pin = {'number': i, 'name': name, 'role': 'nc' if name == 'NC' else 'inout'}
        if n > 40 and i <= half:
            pin['side'] = 'in'
        out.append(pin)
    return out


def pitch_text(spec):
    ps = specs.parse_pitches(spec['pitch'])
    return ps[0] if ps else None


def collect(fams, makers):
    cand = specs.candidates()
    table = specs.load()
    ids = [f['id'] for f in json.load(open(FOOTPRINTS))['footprints']]
    fmap = footprint_map()
    devices = []
    sheets = {}   # id -> url
    sheet_of = {}  # pdf -> id
    for L in fplines.build(cand, table, ids):
        if (fams and L['family'] not in fams) or (makers and SLUG[L['maker']] not in makers):
            continue
        by_spec = collections.OrderedDict()
        for r in sorted(L['members'], key=lambda r: r['mpn']):
            v, _, _, spec = specs.verdict(r, table.get(specs.key(r)))
            assert v == 'ok', (r['mpn'], v)
            by_spec.setdefault(spec['scheme'] + '|' + spec['pinout'] + '|' + spec['card'] + '|' + spec['pitch'], []).append((r, spec))
        for k, (_, members) in enumerate(by_spec.items()):
            spec = members[0][1]
            maker_slug = SLUG[L['maker']]
            base = L['stem'].split('-', 2)[2].lower()
            did = f"card-{WORD[L['family']]}-{maker_slug}-{base}" + ('' if k == 0 else f'-v{k + 1}')
            card = spec['card']
            pitch = pitch_text(spec)
            if L['family'] == 'edge':
                function = f"{pitch:.2f} mm {card}" if pitch else card
                leaf = 'Card edge'
            elif L['family'] == 'module':
                function, leaf = card, 'Memory module'
            else:
                function, leaf = card, LEAF[card]
            counts = sorted({specs.pin_count(r, s) for r, s in members})
            pinouts = collections.OrderedDict()
            parts = []
            for r, s in members:
                n = specs.pin_count(r, s)
                names = specs.pin_names(s['pinout'], n)
                assert names is not None, (r['mpn'], s['pinout'], n)
                pinouts.setdefault(f'p{n}', pins_json(names, n))
                pdf_id = sheet_of.get(r['pdf'])
                if pdf_id is None:
                    pdf_id = datasheet_id(r['pdf'], sheets, r['maker'])
                    sheet_of[r['pdf']] = pdf_id
                    sheets.setdefault(pdf_id, {})[r['url']] = r['maker']
                fid = fmap.get((L['stem'], int(r['positions']))) or f"{L['stem']}-{r['positions']}"
                package = PACKAGE.get(r['mounting'], 'SMD vertical')
                parts.append({'name': r['mpn'], 'manufacturer': MANUFACTURER[L['maker']], 'datasheet': pdf_id,
                              'package': package, 'pinout': f'p{n}', 'footprint': fid})
            series = L['series'] if L['series'] not in ('', '-') else os.path.splitext(os.path.basename(L['pdf']))[0]
            contacts = f", {counts[0]} contacts" if len(counts) == 1 else ''
            dev = {'id': did, 'function': function,
                   'description': f"{MANUFACTURER[L['maker']]} {series} {card} card slot{contacts}",
                   'drawer': ['Connector', 'Card slot', leaf],
                   'note': f"{MANUFACTURER[L['maker']]} {series}, {os.path.basename(L['pdf'])}",
                   'card': card, 'cardPitch': pitch if (pitch and L['family'] != 'card') else None,
                   'pinouts': pinouts, 'parts': parts, 'family': L['family'], 'maker': L['maker']}
            devices.append(dev)
    return devices, sheets


def block(dev):
    lines = ['    {']
    lines.append(f'      "id": {json.dumps(dev["id"])},')
    lines.append(f'      "function": {json.dumps(dev["function"], ensure_ascii=False)},')
    lines.append(f'      "description": {json.dumps(dev["description"], ensure_ascii=False)},')
    lines.append(f'      "drawer": {json.dumps(dev["drawer"], ensure_ascii=False)},')
    lines.append(f'      "note": {json.dumps(dev["note"], ensure_ascii=False)},')
    lines.append(f'      "card": {json.dumps(dev["card"], ensure_ascii=False)},')
    if dev['cardPitch'] is not None:
        lines.append(f'      "cardPitch": {json.dumps(dev["cardPitch"])},')
    lines.append('      "pinouts": ' + json.dumps(dev['pinouts'], ensure_ascii=False, separators=(',', ':')) + ',')
    lines.append('      "parts": [')
    ps = [json.dumps(p, ensure_ascii=False, separators=(',', ':')) for p in dev['parts']]
    for i, p in enumerate(ps):
        lines.append('        ' + p + (',' if i < len(ps) - 1 else ''))
    lines.append('      ]')
    lines.append('    }')
    return '\n'.join(lines)


def split_blocks(text):
    """(head, [block text without trailing comma], tail) of connector.json."""
    lines = text.split('\n')
    start = next(i for i, l in enumerate(lines) if l.startswith('  "devices": ['))
    end = max(i for i, l in enumerate(lines) if l == '  ]')
    blocks, cur = [], None
    for l in lines[start + 1:end]:
        if l == '    {':
            cur = [l]
        elif l in ('    }', '    },'):
            cur.append('    }')
            blocks.append('\n'.join(cur))
            cur = None
        elif cur is not None:
            cur.append(l)
    return '\n'.join(lines[:start + 1]), blocks, '\n'.join(lines[end:])


def block_id(b):
    return re.search(r'"id": "([^"]+)"', b).group(1)


def new_datasheets(sheets, text):
    data = json.loads(text)['datasheets']
    have = {d['id']: d['url'] for d in data}
    lines = text.split('\n')
    first = next(i for i, l in enumerate(lines) if l.strip().startswith('{"id"'))
    last = max(i for i, l in enumerate(lines) if l.strip().startswith('{"id"'))
    entries = [l.rstrip(',') for l in lines[first:last + 1]]
    for sid, urls in sorted(sheets.items()):
        url = sorted(urls)[0]
        if len(urls) > 1:
            print(f'gen-parts: datasheet {sid} has {len(urls)} URLs: {sorted(urls)}', file=sys.stderr)
        final = sid
        if final in have and have[final] != url:
            final = sid + '-' + SLUG[sorted(urls.values())[0]]
        if final in have:
            continue
        have[final] = url
        entry = '    {"id": ' + json.dumps(final) + ', "url": ' + json.dumps(url) + '}'
        pos = next((i for i, e in enumerate(entries) if json.loads(e.strip())['id'] > final), len(entries))
        entries.insert(pos, entry)
    return '\n'.join(lines[:first]) + '\n' + ',\n'.join(entries) + '\n' + '\n'.join(lines[last + 1:])


def generate(cfams, wfams, makers):
    """(connector.json text, devices, sheets) for the families (candidates names `cfams`, device-id words `wfams`)."""
    devices, sheets = collect(cfams, makers)
    head, blocks, tail = split_blocks(open(CONNECTOR).read())
    fams = wfams or list(WORD.values())
    ms = makers or list(SLUG.values())
    prefixes = tuple(f'card-{f}-{m}-' for f in fams for m in ms)
    kept = [b for b in blocks if not block_id(b).startswith(prefixes)]
    gen = [block(d) for d in sorted(devices, key=lambda d: d['id'])]
    return head + '\n' + ',\n'.join(kept + gen) + '\n' + tail, devices, sheets


def main():
    argv = sys.argv[1:]
    cfams, wfams, makers = [], [], []
    i = 0
    check = '--check' in argv
    examples = '--examples' in argv
    while i < len(argv):
        if argv[i] == '--family':
            cfams.append(FAMILY[argv[i + 1]])
            wfams.append(WORD[FAMILY[argv[i + 1]]])
            i += 2
            continue
        if argv[i] == '--maker':
            makers.append(argv[i + 1])
            i += 2
            continue
        i += 1
    conn, devices, sheets = generate(cfams, wfams, makers)
    ds = new_datasheets(sheets, open(DATASHEETS).read())
    if examples:
        write_examples(devices)
        return 0
    if check:
        same = conn == open(CONNECTOR).read() and ds == open(DATASHEETS).read()
        print('gen-parts: up to date' if same else 'gen-parts: connector.json or datasheets.json differ')
        return 0 if same else 1
    open(CONNECTOR, 'w').write(conn)
    open(DATASHEETS, 'w').write(ds)
    print(f'gen-parts: {len(devices)} devices, {sum(len(d["parts"]) for d in devices)} parts, {len(sheets)} datasheets')
    return 0


def write_examples(devices):
    cand = {r['mpn']: r for r in specs.candidates() if r['state'] == 'add'}
    best = {}
    for d in devices:
        for p in d['parts']:
            r = cand[p['name']]
            for key in ((d['family'],) + (('straddle',) if r['mounting'] == 'Board Edge, Straddle Mount' else ())):
                if key not in best or int(r['stock'] or 0) > int(best[key][0]['stock'] or 0):
                    best[key] = (r, d, p)
    with open(os.path.join(specs.DATA, 'examples.tsv'), 'w') as f:
        f.write('mpn\tfamily\tpositions\tdrawer\tfootprint\n')
        for key in sorted(best):
            r, d, p = best[key]
            f.write(f"{r['mpn']}\t{key}\t{r['positions']}\t{'/'.join(d['drawer'])}\t{p['footprint']}\n")
    print('examples:', ', '.join(f"{best[k][0]['mpn']} ({k})" for k in sorted(best)))


if __name__ == '__main__':
    sys.exit(main())
