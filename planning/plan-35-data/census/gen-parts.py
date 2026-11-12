#!/usr/bin/env python3
"""plan-35-E/F: charlcd device objects from specs.tsv + candidates.tsv.

usage: gen-parts.py [--check] [--maker <name>] <specs.tsv> <candidates.tsv>

  default   print one device per line, charlcd.json style
            ('    {"id": ...},'), sorted by device id
  --check   check every specs row: every key present, every named pin in
            the pin list, pin count = candidates.tsv `pins`, the pinout
            shape; print "<n> rows ok" or the problems (exit 1)
  --maker   only rows whose candidates.tsv maker is <name>

specs.tsv columns (tab-separated, one row per part, header line first):
  part        orderable part number, as candidates.tsv `part`
  device      device id (the part name without a trailing '#')
  controller  as the PDF prints it
  vcc         "min,max" V (the PDF's VDD row)
  timing      "tc,tpw,tas,tah,tdsw,th,tddr,tdhr" ns
  exec        "instruction,data,clearHome,powerOn" ns
  optics      "transflective", "transmissive" or "reflective"
  pins        pin names 1..N, ';'-separated, as printed
  rs rw       pin names
  e           "E", or "E1;E2"
  data        DB0..DB7 pin names, ';'-separated
  contrast    the V0 pin name
  backlight   "" or LEDs ';'-separated, each "anode,cathode,Color"
  note        the device note (PDF and pages used)

Pin roles from names: VSS / GND / FGND -> ground, VDD -> power, NC -> nc,
the 8 data pins -> inout (group data), anything else -> input.
"""
import csv
import json
import sys

KEYS = ['part', 'device', 'controller', 'vcc', 'timing', 'exec', 'optics', 'pins', 'rs', 'rw', 'e',
        'data', 'contrast', 'backlight', 'note']
TIMING = ['tc', 'tpw', 'tas', 'tah', 'tdsw', 'th', 'tddr', 'tdhr']
EXEC = ['instruction', 'data', 'clearHome', 'powerOn']
MAKER = {'Newhaven Display Intl': 'Newhaven Display', 'Midas Displays': 'Midas Displays',
         'Focus LCDs': 'Focus LCDs'}
PANEL = {'yellow-green': 'STN yellow-green', 'blue-negative': 'STN blue negative',
         'gray': 'STN gray', 'white': 'FSTN white'}
GROUND = {'VSS', 'GND', 'FGND'}


def num(s):
    v = float(s)
    return int(v) if v == int(v) else v


def role(name, data):
    u = name.upper()
    if name in data:
        return 'inout'
    if u in GROUND:
        return 'ground'
    if u == 'VDD':
        return 'power'
    if u == 'NC':
        return 'nc'
    return 'input'


def leds(s):
    return [x.split(',') for x in s.split(';')] if s.strip() else []


def problems(r, c):
    out = []
    for k in KEYS:
        if k != 'backlight' and not r.get(k, '').strip():
            out.append('missing ' + k)
    if out:
        return out
    pins = r['pins'].split(';')
    if len(pins) != int(c['pins']):
        out.append('%d pins, candidates.tsv says %s' % (len(pins), c['pins']))
    if len(r['timing'].split(',')) != 8:
        out.append('timing needs 8 values')
    if len(r['exec'].split(',')) != 4:
        out.append('exec needs 4 values')
    if len(r['vcc'].split(',')) != 2:
        out.append('vcc needs min,max')
    data = r['data'].split(';')
    if len(data) != 8:
        out.append('data needs 8 names')
    e = r['e'].split(';')
    if len(e) != (2 if c['size'] == '40x4' else 1):
        out.append('e: %d names for a %s' % (len(e), c['size']))
    named = [r['rs'], r['rw'], r['contrast']] + e + data
    for a in leds(r['backlight']):
        if len(a) != 3:
            out.append('backlight LED needs anode,cathode,Color: %r' % a)
            continue
        named += a[:2]
    for n in named:
        if pins.count(n) != 1:
            out.append('pin %r named %d times in the pin list' % (n, pins.count(n)))
    want = {'': 0, '-': 0}.get(c['backlight'], len(c['backlight'].split(',')))
    if len(leds(r['backlight'])) != want:
        out.append('backlight: %d LEDs, candidates.tsv says %r' % (len(leds(r['backlight'])), c['backlight']))
    if r['optics'] not in ('transflective', 'transmissive', 'reflective'):
        out.append('optics %r' % r['optics'])
    if r['device'] != r['part'].rstrip('#'):
        out.append('device id is not the part name without #')
    return out


def device(r, c):
    pins = r['pins'].split(';')
    data = r['data'].split(';')
    size = c['size']
    cols, rows = (int(x) for x in size.split('x'))
    pinout = []
    for i, n in enumerate(pins):
        p = {'number': i + 1, 'name': n, 'role': role(n, data)}
        if n in data:
            p['group'] = 'data'
        pinout.append(p)
    bl = leds(r['backlight'])
    if not bl:
        bdesc = 'no backlight'
    elif len(bl) == 3:
        bdesc = 'RGB LED backlight'
    else:
        bdesc = bl[0][2].lower() + ' LED backlight'
    e = r['e'].split(';')
    dev = {
        'id': r['device'],
        'function': size,
        'description': '%s character LCD module, %s, %s, %s' % (size, PANEL[c['panel']], r['optics'], bdesc),
        'drawer': ['Displays', 'Character'],
        'note': r['note'],
        'pinouts': {c['pinout']: pinout},
        'columns': cols,
        'rows': rows,
        'controller': r['controller'],
        'rom': 'A00',
        'panel': c['panel'],
        'bus': {'rs': r['rs'], 'rw': r['rw'], 'e': e[0] if len(e) == 1 else e, 'data': data},
        'contrast': r['contrast'],
    }
    if bl:
        objs = [{'anode': a, 'cathode': k, 'color': col} for a, k, col in bl]
        dev['backlight'] = objs[0] if len(objs) == 1 else objs
    dev['timing'] = dict(zip(TIMING, (num(x) for x in r['timing'].split(','))))
    dev['exec'] = dict(zip(EXEC, (num(x) for x in r['exec'].split(','))))
    dev['vcc'] = [num(x) for x in r['vcc'].split(',')]
    dev['parts'] = [{
        'name': r['part'],
        'manufacturer': MAKER[c['maker']],
        'datasheet': c['pdf'][:-4].lower(),
        'package': 'Module',
        'pinout': c['pinout'],
        'footprint': c['footprint'],
        'connectorFootprint': c['connector'],
    }]
    return dev


def main(argv):
    check = '--check' in argv
    argv = [a for a in argv if a != '--check']
    maker = None
    if '--maker' in argv:
        i = argv.index('--maker')
        maker = argv[i + 1]
        del argv[i:i + 2]
    specs = list(csv.DictReader(open(argv[0]), delimiter='\t', quoting=csv.QUOTE_NONE))
    cand = {c['part']: c for c in csv.DictReader(open(argv[1]), delimiter='\t', quoting=csv.QUOTE_NONE)
            if c['verdict'] == 'add'}
    bad = 0
    devs = []
    seen = set()
    for r in specs:
        c = cand.get(r['part'])
        if c is None:
            print('%s: not an add row of candidates.tsv' % r['part'])
            bad += 1
            continue
        if maker and c['maker'] != maker:
            continue
        if r['part'] in seen:
            print('%s: duplicate row' % r['part'])
            bad += 1
        seen.add(r['part'])
        p = problems(r, c)
        for x in p:
            print('%s: %s' % (r['part'], x))
        bad += len(p)
        if not p:
            devs.append(device(r, c))
    if check:
        print('%d rows%s' % (len(seen), ' ok' if not bad else ', %d problems' % bad))
        return 1 if bad else 0
    if bad:
        return 1
    for d in sorted(devs, key=lambda d: d['id']):
        print('    ' + json.dumps(d, ensure_ascii=False) + ',')
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
