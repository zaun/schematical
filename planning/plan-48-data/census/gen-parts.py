#!/usr/bin/env python3
"""plan-48-G § 3: write Parts/timer.json and Parts/opamp.json.
usage: python3 -I gen-parts.py --generics [--type timer|opamp]
       python3 -I gen-parts.py [--type timer|opamp]
       python3 -I gen-parts.py --verify

--generics writes the files with only the generic device (GENERIC-555 /
GENERIC-OPAMP). Without it every `add` row of candidates.tsv becomes a part
(§ 3.1): the package label and pinout from its specs.tsv row, the footprint
from footprint-map.tsv, the datasheet id from the folder's datasheets.tsv,
the facts from the spec row and DigiKey's parameters of the listing. The
generic device is always the file's last. The full run adds each used PDF's
entry to Parts/datasheets.json (an id only if absent) and prints the
footprint ids the parts use that are not in Parts/footprints.json yet (they
are in footprints-C.json / footprints-D.json: fp-merge.py --only <ids>).
--verify: every `add` row of the pick is a part, every part an `add` row or a
generic."""
import collections
import csv
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import common  # noqa: E402

DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
PARTS = os.path.join(ROOT, 'schematical', 'schematical', 'Parts')
MANUFACTURER = {'ti': 'Texas Instruments', 'adi': 'Analog Devices', 'st': 'STMicroelectronics',
                'renesas': 'Renesas Electronics'}
SCHEMES = {
    '555-8': ['GND', 'TRIG', 'OUT', 'RESET', 'CONT', 'THRES', 'DISCH', 'VCC'],
    '556-14': ['DISCH1', 'THRES1', 'CONT1', 'RESET1', 'OUT1', 'TRIG1', 'GND', 'TRIG2', 'OUT2', 'RESET2',
               'CONT2', 'THRES2', 'DISCH2', 'VCC'],
    'single-8': ['NC', 'IN-', 'IN+', 'V-', 'NC', 'OUT', 'V+', 'NC'],
    'dual-8': ['OUT1', 'IN1-', 'IN1+', 'V-', 'IN2+', 'IN2-', 'OUT2', 'V+'],
    'quad-14': ['OUT1', 'IN1-', 'IN1+', 'V+', 'IN2+', 'IN2-', 'OUT2', 'OUT3', 'IN3-', 'IN3+', 'V-', 'IN4+',
                'IN4-', 'OUT4'],
    'single-5a': ['OUT', 'V-', 'IN+', 'IN-', 'V+'],
    'single-5b': ['IN+', 'V-', 'IN-', 'OUT', 'V+'],
}
BALL = re.compile(r'^[ABCDEFGHJKLMNPRTUVWY][1-9][0-9]?$')
CLASS_TEXT = {'gp': ('general', 'general purpose', 'General purpose'), 'jfet': ('jfet', 'JFET input', 'JFET input'),
              'cmos': ('cmos', 'CMOS', 'CMOS')}
NAMES = {1: 'single', 2: 'dual', 4: 'quad'}
TITLES = {1: 'Single', 2: 'Dual', 4: 'Quad'}
FILES = {'timer': os.path.join(PARTS, 'timer.json'), 'opamp': os.path.join(PARTS, 'opamp.json')}


def kebab(text):
    return re.sub(r'[^a-z0-9]+', '-', text.lower()).strip('-')


def read(path):
    return list(csv.DictReader(open(path, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE))


def num(v):
    v = float('%.6g' % v)
    return int(v) if v == int(v) else v


# ---- facts --------------------------------------------------------------------

def parse_facts(text):
    out = {}
    for item in text.split(';'):
        if item.strip():
            k, _, v = item.partition('=')
            out[k.strip()] = v.strip()
    return out


def unit_value(text, table):
    m = re.match(r'^\s*([\d.]+)\s*([^\d\s]*)', text or '')
    if not m or m.group(2) not in table:
        return None
    return float(m.group(1)) * table[m.group(2)]


def gbw_hz(text):
    return unit_value((text or '').replace('µ', 'u'), {'Hz': 1, 'kHz': 1e3, 'MHz': 1e6, 'GHz': 1e9})


def slew_vus(text):
    t = (text or '').replace('µ', 'u').replace(' ', '')
    m = re.match(r'^([\d.]+)V/(us|ms|ns|s)$', t)
    if not m:
        return None
    return float(m.group(1)) * {'us': 1, 'ms': 1e-3, 'ns': 1e3, 's': 1e-6}[m.group(2)]


def vos_mv(text):
    return unit_value((text or '').replace('µ', 'u').replace(' ', ''),
                      {'V': 1000, 'mV': 1, 'uV': 1e-3, 'nV': 1e-6})


# ---- pinouts --------------------------------------------------------------------

def pin_names(spec):
    """(names in pin order, ball designators or None) of a specs.tsv row."""
    if spec['scheme'] in SCHEMES:
        return list(SCHEMES[spec['scheme']]), None
    items = [i.split('=', 1) for i in spec['pinout'].split(',')]
    designators = [d for d, _ in items]
    names = [n for _, n in items]
    if all(d.isdigit() for d in designators):
        assert designators == [str(i) for i in range(1, len(items) + 1)], spec['pinout']
        return names, None
    assert all(BALL.match(d) for d in designators), spec['pinout']
    return names, designators


def role_of(name):
    if name in ('VCC', 'V+') or name == 'VCC2':
        return 'power'
    if name in ('GND', 'V-', 'EP', 'GND2'):
        return 'ground'
    if re.match(r'^(OUT\d?|DISCH\d?|FLAG)$', name):
        return 'output'
    if name == 'NC':
        return 'nc'
    return 'input'


def pins_json(names, balls):
    out = []
    for k, name in enumerate(names):
        pin = {'ball': balls[k]} if balls else {'number': k + 1}
        pin.update({'name': name, 'role': role_of(name)})
        out.append(pin)
    return out


# ---- footprints -----------------------------------------------------------------

def load_map():
    fmap = collections.defaultdict(list)
    for r in read(os.path.join(DATA, 'footprint-map.tsv')):
        fmap[(r['company'], r['package'])].append(r)
    return fmap


def footprint_of(fmap, row):
    cands = [m for m in fmap.get((row['company'], row['package']), []) if m['supplier'] in (row['supplier_package'], '*')]
    exact = [m for m in cands if m['supplier'] == row['supplier_package']] or cands
    own = [m for m in exact if m['pdf'] == row['pdf']]
    use = own or [m for m in exact if m['pdf'] in ('*', '')]
    ids = sorted({m['footprint'] for m in use})
    if len(ids) != 1:
        sys.exit('gen-parts: %s: footprint map gives %s (%s | %s | %s)' % (row['mpn'], ids, row['company'],
                                                                             row['package'], row['supplier_package']))
    return ids[0]


# ---- the generics ------------------------------------------------------------------

def generic_device(typ):
    if typ == 'timer':
        return {
            'id': 'timer-generic', 'function': 'Generic', 'description': 'Generic 555 timer',
            'drawer': ['Analog', 'Timers'],
            'pinouts': {'dip-8': pins_json(SCHEMES['555-8'], None)},
            'process': 'bipolar', 'maxHz': 500000, 'tpd': 100, 'vcc': [4.5, 16],
            'timers': [section('')],
            'parts': [{'name': 'GENERIC-555', 'manufacturer': 'Generic', 'package': 'DIP', 'pinout': 'dip-8',
                       'footprint': 'PDIP-8', 'footprintChoices': [{'package': 'DIP', 'footprint': 'PDIP-8'},
                                                                   {'package': 'SOIC', 'footprint': 'SOIC-8'}]}],
        }
    return {
        'id': 'opamp-generic', 'function': 'Generic', 'description': 'Generic dual op-amp',
        'drawer': ['Analog', 'Op-amps'],
        'pinouts': {'dip-8': pins_json(SCHEMES['dual-8'], None)},
        'type': 'general', 'gbwHz': 1000000, 'vcc': [3, 32],
        'amplifiers': amplifiers(2),
        'parts': [{'name': 'GENERIC-OPAMP', 'manufacturer': 'Generic', 'package': 'DIP', 'pinout': 'dip-8',
                   'footprint': 'PDIP-8', 'footprintChoices': [{'package': 'DIP', 'footprint': 'PDIP-8'},
                                                               {'package': 'SOIC', 'footprint': 'SOIC-8'}]}],
    }


def section(k, reset=True, control=True):
    s = {'trigger': 'TRIG' + k, 'threshold': 'THRES' + k, 'discharge': 'DISCH' + k, 'output': 'OUT' + k}
    if reset:
        s['reset'] = 'RESET' + k
    if control:
        s['control'] = 'CONT' + k
    return s


def amplifiers(n):
    if n == 1:
        return [{'plus': 'IN+', 'minus': 'IN-', 'output': 'OUT'}]
    return [{'plus': 'IN%d+' % k, 'minus': 'IN%d-' % k, 'output': 'OUT%d' % k} for k in range(1, n + 1)]


# ---- the pick -------------------------------------------------------------------------

def volts(text):
    lo, hi = text.split('-')
    return [num(float(lo)), num(float(hi))]


def build(typ):
    rows = [r for r in read(os.path.join(DATA, 'candidates.tsv')) if r['state'] == 'add' and r['type'] == typ]
    specs = {(s['type'], s['company'], s['pdf'], s['package'], s['supplier_package'], s['circuits']): s
             for s in read(os.path.join(DATA, 'specs.tsv'))}
    fmap = load_map()
    raw = {r['mpn']: r for r in common.read_raw(os.path.join(DATA, 'raw'))}
    sheets = {}
    folder_urls = {}
    for company in ('ti', 'adi', 'st', 'renesas'):
        path = os.path.join(ROOT, 'datasheets', typ, company, 'datasheets.tsv')
        if os.path.exists(path):
            for r in read(path):
                folder_urls[os.path.join('datasheets', typ, company, r['file'])] = r['url']
    names = collections.Counter(r['mpn'] for r in rows)
    clash = [n for n, c in names.items() if c > 1]
    if clash:
        sys.exit('gen-parts: part names listed twice: %s' % clash[:10])
    groups = collections.defaultdict(list)
    for r in rows:
        s = specs[(r['type'], r['company'], r['pdf'], r['package'], r['supplier_package'], r['circuits'])]
        facts = parse_facts(s['facts'])
        key = (r['company'], r['pdf'], r['circuits'])
        if typ == 'timer':
            key += (facts['process'], facts['maxHz'], facts['tpd'])
        else:
            key += (r['class'],)
        groups[key].append((r, s, facts))
    base_ids = collections.Counter()
    for key in groups:
        base_ids[(os.path.basename(key[1])[:-4], key[2])] += 1
    devices = []
    for key in sorted(groups, key=lambda k: (k[0], os.path.basename(k[1]), k[2]) + k[3:]):
        members = groups[key]
        company, pdf, circuits = key[0], key[1], int(key[2])
        sid = os.path.basename(pdf)[:-4].lower()
        sheets[sid] = re.sub(r'^http://', 'https://', folder_urls[pdf])
        ref = max(members, key=lambda m: (int(m[0]['stock'] or 0), m[0]['mpn']))
        dev_id = '%s-%d' % (sid, circuits)
        if base_ids[(os.path.basename(pdf)[:-4], key[2])] > 1:
            dev_id += '-' + (key[-1] if typ == 'opamp' else '%s-%s-%s' % key[3:])
        pinouts, pkeys = {}, {}
        for r, s, f in members:
            names_, balls = pin_names(s)
            pj = pins_json(names_, balls)
            base = '%s-%d' % (kebab(s['label']), len(names_))
            pk, suffix = base, ''
            while pk in pinouts and pinouts[pk] != pj:
                suffix = chr(ord(suffix or 'a') + 1) if suffix else 'b'
                pk = base + '-' + suffix
            pinouts[pk] = pj
            pkeys[s['pinout'] + '|' + s['scheme'] + '|' + s['package'] + '|' + s['supplier_package']] = pk
        device = {'id': dev_id}
        ref_facts = ref[2]
        circuits_n = circuits
        if typ == 'timer':
            device['function'] = 'Single (555)' if circuits_n == 1 else 'Dual (556)'
            device['description'] = '%s %stimer, %s' % (sid.upper(), 'dual ' if circuits_n == 2 else '', ref_facts['process'])
            device['drawer'] = ['Analog', 'Timers']
        else:
            kind, kind_text, function = CLASS_TEXT[key[-1]]
            device['function'] = function
            device['description'] = '%s %s op-amp, %s' % (sid.upper(), NAMES[circuits_n], kind_text)
            device['drawer'] = ['Analog', 'Op-amps', TITLES[circuits_n]]
        device['pinouts'] = pinouts
        if typ == 'timer':
            device['process'] = ref_facts['process']
            device['maxHz'] = int(float(ref_facts['maxHz']))
            device['tpd'] = num(float(ref_facts['tpd']))
            device['vcc'] = volts(ref_facts['vcc'])
            allnames = [set(p['name'] for p in pj) for pj in pinouts.values()]
            sections = []
            for k in (['1', '2'] if circuits_n == 2 else ['']):
                sections.append(section(k, all('RESET' + k in n for n in allnames), all('CONT' + k in n for n in allnames)))
            device['timers'] = sections
        else:
            device['type'] = CLASS_TEXT[key[-1]][0]
            p = raw[ref[0]['mpn']]['params']
            g, sl = gbw_hz(p.get('Gain Bandwidth Product')), slew_vus(p.get('Slew Rate'))
            if g:
                device['gbwHz'] = int(round(g))
            if sl:
                device['slewVus'] = num(sl)
            device['railIn'] = 'inputMax' in ref_facts
            device['railOut'] = 'Rail-to-Rail' in p.get('Output Type', '')
            device['vcc'] = volts(ref_facts['vcc'])
            if 'inputMax' in ref_facts:
                top = num(float(volts(ref_facts['vcc'])[1]) + float(ref_facts['inputMax']))
                if top <= 100:
                    device['inputMax'] = top
            device['amplifiers'] = amplifiers(circuits_n)
        vos = {}
        parts = []
        for r, s, f in sorted(members, key=lambda m: m[0]['mpn']):
            part = {'name': r['mpn'], 'manufacturer': MANUFACTURER[company], 'datasheet': sid, 'package': s['label'],
                    'pinout': pkeys[s['pinout'] + '|' + s['scheme'] + '|' + s['package'] + '|' + s['supplier_package']],
                    'footprint': footprint_of(fmap, dict(r, pdf=pdf))}
            if r['grade'] == 'AEC-Q100':
                part['flags'] = ['automotive']
            if volts(f['vcc']) != device['vcc']:
                part['vcc'] = volts(f['vcc'])
            if typ == 'opamp':
                top = None
                if 'inputMax' in f:
                    t = num(float(volts(f['vcc'])[1]) + float(f['inputMax']))
                    top = t if t <= 100 else None
                if 'vcc' in part or top != device.get('inputMax'):
                    if top is not None:
                        part.setdefault('vcc', volts(f['vcc']))
                        part['inputMax'] = top
                v = vos_mv(raw[r['mpn']]['params'].get('Voltage - Input Offset'))
                vos[r['mpn']] = v
            parts.append(part)
        if typ == 'opamp':
            values = {v for v in vos.values()}
            if len(values) == 1 and None not in values:
                device['vosMv'] = num(values.pop())
            else:
                for part in parts:
                    v = vos[part['name']]
                    if v:
                        part['vosMv'] = num(v)
        device['parts'] = parts
        devices.append(device)
    ids = [d['id'] for d in devices]
    assert len(ids) == len(set(ids)), 'duplicate device ids: %s' % [i for i in ids if ids.count(i) > 1]
    return devices, sheets, rows


def write(typ, devices):
    doc = {'format': 'schematical-parts', 'version': 1, 'kind': typ, 'devices': devices + [generic_device(typ)]}
    with open(FILES[typ], 'w', encoding='utf-8') as f:
        json.dump(doc, f, ensure_ascii=False, indent=1)
        f.write('\n')


def add_datasheets(sheets):
    path = os.path.join(PARTS, 'datasheets.json')
    text = open(path, encoding='utf-8').read()
    existing = dict(re.findall(r'"id": "([^"]+)", "url": "([^"]+)"', text))
    for i, u in sheets.items():
        if i in existing and existing[i] != u:
            print('gen-parts: datasheet id %s already has the URL %s (this plan: %s)' % (i, existing[i], u))
    new = [(i, u) for i, u in sorted(sheets.items()) if i not in existing]
    if not new:
        return 0
    doc = json.loads(text)
    doc['datasheets'].extend({'id': i, 'url': u} for i, u in new)
    doc['datasheets'].sort(key=lambda d: d['id'])
    lines = ',\n'.join('    ' + json.dumps(d, ensure_ascii=False) for d in doc['datasheets'])
    with open(path, 'w', encoding='utf-8') as f:
        f.write('{\n  "format": "schematical-datasheets",\n  "version": 1,\n  "datasheets": [\n' + lines + '\n  ]\n}\n')
    return len(new)


def verify():
    bad = 0
    for typ in ('timer', 'opamp'):
        rows = {r['mpn'] for r in read(os.path.join(DATA, 'candidates.tsv')) if r['state'] == 'add' and r['type'] == typ}
        doc = json.load(open(FILES[typ], encoding='utf-8'))
        parts = {p['name']: p for d in doc['devices'] for p in d['parts']}
        generic = {n for n, p in parts.items() if p['manufacturer'] == 'Generic'}
        for m in sorted(rows - set(parts)):
            print('add row without a part:', m)
            bad += 1
        for m in sorted(set(parts) - rows - generic):
            print('part without an add row:', m)
            bad += 1
        print('%s: %d add rows, %d parts (%d generic)' % (typ, len(rows), len(parts), len(generic)))
    return 1 if bad else 0


def main():
    args = sys.argv[1:]
    if '--verify' in args:
        sys.exit(verify())
    types = ['timer', 'opamp']
    if '--type' in args:
        types = [args[args.index('--type') + 1]]
    for typ in types:
        if '--generics' in args:
            write(typ, [])
            print('gen-parts: %s: 1 device, 1 part (the generic)' % FILES[typ])
            continue
        devices, sheets, rows = build(typ)
        write(typ, devices)
        added = add_datasheets(sheets)
        pc = sum(len(d['parts']) for d in devices) + 1
        fps = {p['footprint'] for d in devices for p in d['parts']}
        lib = open(os.path.join(PARTS, 'footprints.json'), encoding='utf-8').read()
        have = set(re.findall(r'"id": *"([^"]+)"', lib))
        newfp = sorted(fps - have)
        print('gen-parts: %s: %d devices, %d parts; %d datasheets used, %d added to datasheets.json; %d footprint ids '
              'not in footprints.json yet' % (FILES[typ], len(devices) + 1, pc, len(sheets), added, len(newfp)))
        print('NEWFP ' + ','.join(newfp))


if __name__ == '__main__':
    main()
