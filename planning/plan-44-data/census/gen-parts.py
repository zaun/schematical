#!/usr/bin/env python3
"""plan-44-F § 3: write Parts/crystal.json and the new datasheets.json entries.

usage: python3 -I gen-parts.py --generics-only
       python3 -I gen-parts.py --class khz --class resonator
       python3 -I gen-parts.py                       every class
       python3 -I gen-parts.py --footprints <class>... print the footprint ids the run's parts and the generics use

The four generics are always the file's last two devices (`crystal-generic`,
`resonator-generic`); with --generics-only they are all the file holds. Each
other run writes one device per (class, frequency band) for the `add` rows of
the named classes (every class by default), byte-stable (devices sorted by
id, parts by name). Per row: the pinout (`pinout` of its specs.tsv row), the
package family (`label`), the footprint (footprint-map.tsv), the datasheet id
(the placed PDF's lower-case stem; `<maker>-<stem>` when that id is taken by
another URL), the facts (DigiKey's, checked against the datasheet by B), the
`automotive` flag for an AEC-Q200 row. The datasheets the run's parts name
are appended to Parts/datasheets.json (an id already there is kept).
"""
import collections
import csv
import json
import os
import re
import sys
import importlib.util

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.append(HERE)
import common  # noqa: E402
import specs  # noqa: E402


def load(name):
    spec = importlib.util.spec_from_file_location(name.replace('-', '_'), os.path.join(HERE, name + '.py'))
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


footprint = load('footprint-lines')
fpcheck = load('fp-check')

PARTS = os.path.join(common.ROOT, 'schematical', 'schematical', 'Parts')
OUT = os.path.join(PARTS, 'crystal.json')
DATASHEETS = os.path.join(PARTS, 'datasheets.json')
MAKER = {'Abracon': 'Abracon', 'ECS': 'ECS Inc.', 'Suntsu': 'Suntsu Electronics'}
DRAWER = {'khz': ['Crystal', 'kHz'], 'mhz': ['Crystal', 'MHz'], 'resonator': ['Crystal', 'Ceramic resonator']}
KEBAB = re.compile(r'[^a-z0-9]+')

# § 3.2: label -> package family, default first
GENERICS = [
    ('GENERIC-CRYSTAL-32768', 'crystal', 32768, 'x1-x2',
     [('XTAL-3215-2', 'SMD'), ('XTAL-2012-2', 'SMD'), ('XTAL-1610-2', 'SMD'), ('XTAL-CYL-2X6-2', 'Cylinder')]),
    ('GENERIC-CRYSTAL-8M', 'crystal', 8000000, 'x1-x2',
     [('XTAL-HC49US-2', 'HC-49/US'), ('XTAL-3225-2', 'SMD'), ('XTAL-5032-2', 'SMD'), ('XTAL-6035-2', 'SMD')]),
    ('GENERIC-CRYSTAL-16M', 'crystal', 16000000, 'x1-x2',
     [('XTAL-HC49US-2', 'HC-49/US'), ('XTAL-3225-2', 'SMD'), ('XTAL-5032-2', 'SMD'), ('XTAL-6035-2', 'SMD')]),
    ('GENERIC-RESONATOR-8M', 'resonator', 8000000, 'x1-gnd-x2',
     [('CRES-3731-3', 'SMD'), ('CRES-6030-3', 'SMD'), ('CRES-3213-3', 'SMD'), ('CRES-2520-3', 'SMD')]),
]


def kebab(text):
    return KEBAB.sub('-', text.lower()).strip('-')


def number(text):
    """A DigiKey fact as a JSON number (None when absent or 0)."""
    if text in ('', None):
        return None
    v = float(text)
    if v == 0:
        return None
    v = float(f'{v:.6g}')
    return int(v) if v == int(v) else v


def pinout_key(names):
    return '-'.join(n.lower() for n in names)


def pins_json(names):
    out = []
    for k, name in enumerate(names):
        out.append({'number': k + 1, 'name': name, 'role': 'inout' if name in ('X1', 'X2') else 'nc'})
    return out


def khz_text(hz):
    v = hz / 1000
    return (f'{v:.3f}'.rstrip('0').rstrip('.')) + ' kHz'


def band(cls, hz):
    if cls == 'khz':
        return khz_text(hz)
    if hz < 4e6:
        return 'Under 4 MHz'
    if hz < 8e6:
        return '4 MHz to 8 MHz'
    if hz < 16e6:
        return '8 MHz to 16 MHz'
    if cls == 'resonator':
        return '16 MHz and over'
    if hz < 32e6:
        return '16 MHz to 32 MHz'
    return '32 MHz and over'


def add_rows(classes):
    rows = [r for r in csv.DictReader(open(specs.CANDIDATES), delimiter='\t') if r['state'] == 'add']
    return [r for r in rows if r['class'] in classes]


def footprint_of(fmap, table, r):
    pads = len(specs.pinout_items(table[specs.key(r)]['pinout']))
    label = footprint.label(r, pads)
    m = fpcheck.lookup(fmap, footprint.letter(r), r['maker'], r['package'], r['supplier_package'], r['size'], label)
    if m is None or not m['footprint']:
        sys.exit(f"gen-parts: {r['mpn']} has no footprint-map row ({r['maker']} | {r['package']} | "
                 f"{r['supplier_package']} | {r['size']} | {label})")
    return m['footprint'], label


def label_ids():
    """label -> the lowest footprint id (ASCII order) of the map rows with that label."""
    out = {}
    for r in csv.DictReader(open(os.path.join(common.DATA, 'footprint-map.tsv')), delimiter='\t'):
        if r['footprint']:
            out[r['label']] = min(out.get(r['label'], r['footprint']), r['footprint'])
    return out


def generic_devices():
    ids = label_ids()
    out = {'crystal': None, 'resonator': None}
    for name, typ, hz, pkey, choices in GENERICS:
        missing = [l for l, _ in choices if l not in ids]
        if missing:
            sys.exit(f'gen-parts: no footprint-map row for the labels {missing}')
        if out[typ] is None:
            names = ['X1', 'X2'] if pkey == 'x1-x2' else ['X1', 'GND', 'X2']
            out[typ] = {
                'id': f'{typ}-generic', 'function': 'Generic',
                'description': 'Generic ' + ('quartz crystal' if typ == 'crystal' else 'ceramic resonator'),
                'drawer': ['Crystal'], 'pinouts': {pkey: pins_json(names)},
                'type': typ, 'terminals': ['X1', 'X2'], 'parts': [],
            }
        part = {'name': name, 'manufacturer': 'Generic', 'package': choices[0][1], 'pinout': pkey,
                'footprint': ids[choices[0][0]],
                'footprintChoices': [{'package': pk, 'footprint': ids[l]} for l, pk in choices],
                'hz': hz}
        if typ == 'resonator':
            part['capacitors'] = True
        out[typ]['parts'].append(part)
    return [out['crystal'], out['resonator']]


def datasheet_ids(rows, placed):
    """pdf path -> datasheets.json id, and id -> url."""
    existing = {}
    for d in json.load(open(DATASHEETS))['datasheets']:
        existing[d['id']] = d['url']
    ids = {}
    urls = {}
    for path in sorted({r['pdf'] for r in rows}):
        url = re.sub(r'^http://', 'https://', placed[path])
        stem = kebab(os.path.splitext(os.path.basename(path))[0])
        sid = stem
        maker = path.split('/')[2]
        if (sid in existing and existing[sid] != url) or (sid in urls and urls[sid] != url):
            sid = f'{maker}-{stem}'
        if (sid in existing and existing[sid] != url) or (sid in urls and urls[sid] != url):
            sys.exit(f'gen-parts: datasheet id {sid} is taken')
        ids[path] = sid
        urls[sid] = url
    return ids, urls


def build(classes):
    table = specs.load()
    fmap = fpcheck.load_map([os.path.join(common.DATA, 'footprint-map.tsv')])
    rows = add_rows(classes)
    placed = {}
    for r in csv.DictReader(open(common.PLACED), delimiter='\t'):
        placed.setdefault(r['path'], r['url'])
    sheet_ids, sheet_urls = datasheet_ids(rows, placed)
    names = collections.Counter(r['mpn'] for r in rows)
    clash = [n for n, c in names.items() if c > 1]
    if clash:
        sys.exit(f'gen-parts: part names listed twice: {clash[:10]}')
    devices = {}
    used = set()
    for r in rows:
        s = table[specs.key(r)]
        hz = int(float(r['hz']))
        cls = r['class']
        b = band(cls, hz)
        pin_names = [n for _, n in specs.pinout_items(s['pinout'])]
        pkey = pinout_key(pin_names)
        typ = 'resonator' if cls == 'resonator' else 'crystal'
        dkey = (cls, b)
        if dkey not in devices:
            devices[dkey] = {
                'id': f'{typ}-{cls}-{kebab(b)}' if cls != 'resonator' else f'resonator-{kebab(b)}',
                'function': b, 'description': ('Ceramic resonator, ' if cls == 'resonator' else
                                               'Quartz crystal, ') + b,
                'drawer': DRAWER[cls], 'note': set(), 'pinouts': {}, 'type': typ,
                'terminals': ['X1', 'X2'], 'parts': [],
            }
        d = devices[dkey]
        d['pinouts'][pkey] = pins_json(pin_names)
        fp, _ = footprint_of(fmap, table, r)
        used.add(fp)
        d['note'].add(f"{r['series']} {os.path.basename(r['pdf'])} p.{s['page']}")
        part = {'name': r['mpn'], 'manufacturer': MAKER[r['maker']], 'datasheet': sheet_ids[r['pdf']],
                'package': s['label'], 'pinout': pkey, 'footprint': fp, 'hz': hz}
        if r['load_pf'] not in ('', None):
            part['loadPf'] = number(r['load_pf'])
        for key, col in (('tolerancePpm', 'tol_ppm'), ('stabilityPpm', 'stab_ppm'), ('esrOhms', 'esr_ohm')):
            v = number(r[col])
            if v is not None:
                part[key] = v
        if s['capacitors'] == '1':
            part['capacitors'] = True
        if r['rating'] == 'AEC-Q200':
            part['flags'] = ['automotive']
        d['parts'].append(part)
    out = []
    for key in sorted(devices):
        d = devices[key]
        d['parts'].sort(key=lambda p: p['name'])
        d['note'] = ('Facts, pinouts and ordering codes from the maker datasheets: '
                     + '; '.join(sorted(d['note'])) + '. Pads that are not the two terminals are NC '
                     '(case or lid); the middle pad of a three-pad resonator is GND.')
        out.append(d)
    ids = [d['id'] for d in out]
    assert len(ids) == len(set(ids)), 'duplicate device ids'
    return out, sheet_urls, used, len(rows)


def write(devices):
    doc = {'format': 'schematical-parts', 'version': 1, 'kind': 'crystal', 'devices': devices}
    with open(OUT, 'w') as f:
        json.dump(doc, f, ensure_ascii=False, indent=1)
        f.write('\n')


def add_datasheets(urls):
    """Appends the entries whose id is absent (existing entries are never touched)."""
    text = open(DATASHEETS).read()
    have = set(re.findall(r'"id": "([^"]+)"', text))
    new = [(i, u) for i, u in sorted(urls.items()) if i not in have]
    if not new:
        return 0
    end = text.rstrip().rfind(']')
    head = text[:end].rstrip()
    lines = ',\n'.join('    ' + json.dumps({'id': i, 'url': u}, ensure_ascii=False) for i, u in new)
    with open(DATASHEETS, 'w') as f:
        f.write(head + ',\n' + lines + '\n  ]\n}\n')
    return len(new)


def main():
    argv = sys.argv[1:]
    classes = [argv[i + 1] for i, a in enumerate(argv) if a == '--class']
    generics_only = '--generics-only' in argv
    footprints_only = '--footprints' in argv
    classes = classes or ['khz', 'mhz', 'resonator']
    generics = generic_devices()
    gfp = {p['footprint'] for d in generics for p in d['parts']}
    gfp |= {c['footprint'] for d in generics for p in d['parts'] for c in p['footprintChoices']}
    if generics_only:
        if footprints_only:
            print(' '.join(sorted(gfp)))
            return
        write(generics)
        print(f'gen-parts: {OUT}: 2 devices, 4 parts (the generics); footprints {len(gfp)}')
        return
    devices, urls, used, n = build(classes)
    if footprints_only:
        print(' '.join(sorted(used | gfp)))
        return
    write(devices + generics)
    added = add_datasheets(urls)
    print(f'gen-parts: {OUT}: {len(devices) + 2} devices, {n + 4} parts; {len(urls)} datasheets used, '
          f'{added} added to datasheets.json; footprints used {len(used | gfp)}')


if __name__ == '__main__':
    main()
