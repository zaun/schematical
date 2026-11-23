#!/usr/bin/env python3
"""plan-47-K / L section 3: write Parts/inductor.json, fuse.json, relay.json and buzzer.json.

usage: python3 -I gen-parts.py [--candidates <candidates.tsv>] [--check] <type>...
       (type: inductor, fuse, ptc, relay, buzzer; `fuse` and `ptc` both write fuse.json: name both)

Every `add` row of the candidates file (default candidates.tsv; candidates-provisional.tsv when
candidates.tsv has no `add` row yet) of the named types becomes a part of its kind's file:
the package label, pinout and the plan-47-C columns from its specs.tsv row, the footprint from
footprint-map.tsv (and footprint-map-*.tsv), the datasheet id from the family's datasheets.tsv,
the facts from the row's DigiKey parameters (raw/*.jsonl.gz) checked in plan-47-B / C. Each run
rewrites its files whole and byte-stable (devices sorted by id, parts by name). It also adds
each used PDF's entry to Parts/datasheets.json (id = the placed file's stem in lower case, plus
`-<maker>` on a clash with an existing id of another URL; only if the id is absent).

--check regenerates in memory and reports how many written files differ, then checks every
part: its footprint exists, its pinout's pin count equals the footprint's pads, a relay's pole
count equals its DigiKey contact form's. Prints `<n> differences` and each problem.

Drawer paths (the drawer regroup of 2026-11-23): inductors Discrete / Inductors / <class> / <band>;
fuses Discrete / Fuses / <class>; PTCs Discrete / Resettable fuses / <class>; relays Switches &
Relays / Relay / <Signal|Power|Reed|Solid state|Automotive>; buzzers Displays & Indicators / Buzzers.
"""
import collections
import csv
import glob
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.append(HERE)
import specs47 as sp  # noqa: E402

DATA = sp.DATA
ROOT = os.path.dirname(os.path.dirname(DATA))
PARTS = os.path.join(ROOT, 'schematical', 'schematical', 'Parts')
FAMILY = {'inductor': 'inductor', 'fuse': 'fuse', 'ptc': 'fuse', 'relay': 'relay', 'buzzer': 'buzzer'}
FILE = {'inductor': 'inductor', 'fuse': 'fuse', 'ptc': 'fuse', 'relay': 'relay', 'buzzer': 'buzzer'}
MAKER = {'bourns': 'Bourns', 'murata': 'Murata Electronics', 'wurth': 'Würth Elektronik', 'littelfuse': 'Littelfuse',
         'eaton': 'Eaton', 'schurter': 'SCHURTER', 'belfuse': 'Bel Fuse', 'te': 'TE Connectivity',
         'franjobaim': 'Franjobaim', 'panasonic': 'Panasonic', 'samesky': 'Same Sky', 'pui': 'PUI Audio',
         'mallory': 'Mallory Sonalert'}
LETTER = {'inductor': 'D', 'fuse': 'E', 'ptc': 'F', 'relay': 'G', 'buzzer': 'H'}
CLASS_DRAWER = {'smd': 'Surface mount', 'tht': 'Through hole', 'cartridge': 'Cartridge'}
RELAY_DRAWER = {'signal': 'Signal', 'power': 'Power', 'reed': 'Reed', 'ssr': 'Solid state', 'auto': 'Automotive'}
RELAY_DESC = {'signal': 'Signal relay', 'power': 'Power relay', 'reed': 'Reed relay', 'ssr': 'Solid-state relay',
              'auto': 'Automotive relay'}

IND_BANDS = [(1e-3, '< 10 nH'), (0.1, '10 nH – 100 nH'), (1, '100 nH – 1 µH'), (10, '1 µH – 10 µH'),
             (100, '10 µH – 100 µH'), (1000, '100 µH – 1 mH'), (float('inf'), '≥ 1 mH')]
IND_LIMITS = [0.01, 0.1, 1, 10, 100, 1000]           # µH, lower bound inclusive
FUSE_LIMITS = [0.5, 1, 2, 5, 10]                     # A
FUSE_NAMES = ['< 500 mA', '500 mA – 1 A', '1 A – 2 A', '2 A – 5 A', '5 A – 10 A', '≥ 10 A']
PTC_LIMITS = [0.2, 0.5, 1, 2, 5]
PTC_NAMES = ['< 200 mA', '200 mA – 500 mA', '500 mA – 1 A', '1 A – 2 A', '2 A – 5 A', '≥ 5 A']
IND_NAMES = ['< 10 nH', '10 nH – 100 nH', '100 nH – 1 µH', '1 µH – 10 µH', '10 µH – 100 µH', '100 µH – 1 mH', '≥ 1 mH']


def kebab(text):
    return re.sub(r'[^a-z0-9]+', '-', text.lower()).strip('-')


def num(v):
    """A fact as a JSON number with six significant digits (an int when whole)."""
    v = float(f'{v:.6g}')
    return int(v) if v == int(v) else v


def band(value, limits, names):
    i = 0
    while i < len(limits) and value >= limits[i] * (1 - 1e-9):
        i += 1
    return names[i]


def read_candidates(path):
    rows = sp.read(path)
    return rows


def maps():
    """(line key + series_split) -> footprint row, from footprint-map.tsv and footprint-map-*.tsv."""
    out = {}
    for p in [os.path.join(DATA, 'footprint-map.tsv')] + sorted(glob.glob(os.path.join(DATA, 'footprint-map-*.tsv'))):
        if os.path.exists(p):
            for r in sp.read(p):
                if r['footprint']:
                    out[tuple(r[k] for k in ['letter', 'type', 'company', 'series', 'mounting', 'package',
                                             'supplier_package', 'size', 'detail']) + (r.get('series_split', ''),)] = r
    return out


_PARTS = None


def part_footprints():
    """footprint-parts-*.tsv (mpn, footprint): a part whose footprint differs from its line's."""
    global _PARTS
    if _PARTS is None:
        _PARTS = {}
        for p in sorted(glob.glob(os.path.join(DATA, 'footprint-parts-*.tsv'))):
            for t in sp.read(p):
                _PARTS[t['mpn']] = t['footprint']
    return _PARTS


def footprint_of(fmap, r):
    relay = r['type'] == 'relay'
    det = (r['form'] + '; ' + r['detail']) if relay else (r['detail'] if r['type'] in ('ptc', 'buzzer') else '')
    key = (LETTER[r['type']], r['type'], r['company'], r['series'] if relay else '', r['mounting'], r['package'],
           r['supplier_package'], r['size'], det)
    part = part_footprints().get(r['mpn'])
    if part:
        return {'footprint': part}
    return fmap.get(key + (r['mpn'],)) or fmap.get(key + (r['series'],)) or fmap.get(key + ('',))


def datasheet_ids(rows):
    """{pdf path: (id, url)} for the PDFs the rows use; ids follow the module docstring."""
    path = os.path.join(PARTS, 'datasheets.json')
    existing = {d['id']: d['url'] for d in json.load(open(path, encoding='utf-8'))['datasheets']}
    urls = {}
    for fam in ('inductor', 'fuse', 'relay', 'buzzer'):
        p = os.path.join(ROOT, 'datasheets', fam, 'datasheets.tsv')
        if os.path.exists(p):
            for t in sp.read(p):
                urls[t['path']] = t['url']
    out = {}
    taken = dict(existing)
    for r in sorted(rows, key=lambda r: r['pdf']):
        pdf = r['pdf']
        if pdf in out:
            continue
        stem = kebab(os.path.splitext(os.path.basename(pdf))[0])
        url = urls[pdf]
        i = stem
        if i in taken and taken[i] != url:
            i = f"{stem}-{r['company']}"
        n = 2
        while i in taken and taken[i] != url:
            i = f"{stem}-{r['company']}-{n}"
            n += 1
        taken[i] = url
        out[pdf] = (i, url)
    return out, existing


def pinout_parts(pinout):
    """[(designator, name)] of a specs pinout `1=1,2=2`."""
    return [tuple(x.split('=', 1)) for x in pinout.split(',')]


def pinout_key(pinout):
    return '-'.join(kebab(n) for _, n in pinout_parts(pinout))


def role_of(kind, name):
    if name == 'NC':
        return 'nc'
    if kind == 'inductor' or kind == 'fuse':
        return 'inout'
    if kind == 'buzzer':
        return 'nc' if name in ('F', 'NC') else 'input'
    # relay
    if name.startswith(('A', 'S', 'R', 'K')) and not name.startswith('NC'):
        return 'input' if re.fullmatch(r'(A|K|A\d|K\d|S\d?|R\d?|C)', name) else 'inout'
    return 'inout'


def pinout_json(kind, pinout, relay_coil_names=()):
    pins = []
    for d, n in pinout_parts(pinout):
        if kind == 'relay':
            role = 'nc' if n == 'NC' else ('input' if n in relay_coil_names else 'inout')
        else:
            role = role_of(kind, n)
        pins.append({'number': int(d) if d.isdigit() else d, 'name': n, 'role': role})
    return pins


def coil_names(coils):
    names = set()
    for c in [x for x in coils.split(',') if x]:
        names.update(c.split('-'))
    return names


def contact_entries(contacts):
    out = []
    for pole in [c for c in contacts.split(';') if c]:
        f = pole.split('/')
        coil = None
        if f[-1].startswith('@'):
            coil = int(f[-1][1:])
            f = f[:-1]
        e = {'common': f[0]}
        if f[1]:
            e['no'] = f[1]
        if f[2]:
            e['nc'] = f[2]
        if coil is not None:
            e['coil'] = coil
        out.append(e)
    return out


def arrangement(entries, latching, channels):
    """The drawer leaf of a relay device (plan-47-L section 3.1)."""
    per = collections.defaultdict(list)
    for e in entries:
        per[e.get('coil', 1)].append(e)
    labels = []
    for ch in sorted(per):
        poles = per[ch]
        kinds = ['C' if ('no' in p and 'nc' in p) else ('NO' if 'no' in p else 'NC') for p in poles]
        n = len(poles)
        if all(k == 'C' for k in kinds):
            lab = {1: 'SPDT', 2: 'DPDT'}.get(n, f'{n}PDT')
        elif all(k == 'NO' for k in kinds):
            lab = {1: 'SPST-NO', 2: 'DPST-NO'}.get(n, f'{n}PST-NO')
        elif all(k == 'NC' for k in kinds):
            lab = {1: 'SPST-NC', 2: 'DPST-NC'}.get(n, f'{n}PST-NC')
        else:
            c = kinds.count('C')
            parts = []
            if c:
                parts.append(f'{c} C')
            if kinds.count('NO'):
                parts.append(f"{kinds.count('NO')} NO")
            if kinds.count('NC'):
                parts.append(f"{kinds.count('NC')} NC")
            lab = ' + '.join(parts)
        labels.append(lab)
    if len(labels) > 1:
        lab = f'{len(labels)} × {labels[0]}' if len(set(labels)) == 1 else ' + '.join(labels)
    else:
        lab = labels[0]
    return lab + (' latching' if latching else '')


def first_number(text):
    m = re.search(r'([0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?)', text or '')
    return float(m.group(1)) if m else None


class Gen:
    def __init__(self, cand_path, types):
        self.rows = [r for r in read_candidates(cand_path) if r['state'] == 'add' and r['type'] in types]
        self.specs = sp.load_specs()
        self.params = sp.params_by_mpn()
        self.fmap = maps()
        self.ds, self.existing_ds = datasheet_ids(self.rows)
        self.errors = []

    def err(self, r, text):
        self.errors.append(f"{r['mpn']}: {text}")

    def common(self, r):
        s = self.specs.get(sp.key(r))
        if s is None:
            self.err(r, 'no specs row')
            return None, None, None
        fp = footprint_of(self.fmap, r)
        if fp is None:
            self.err(r, 'no footprint in the map')
            return s, None, None
        return s, fp['footprint'], fp

    def flags(self, r):
        f = []
        if r['grade'] == 'AEC-Q200':
            f.append('automotive')
        if r['grade'] == 'Military':
            f.append('military')
        return f

    # ---- inductor ----
    def inductor(self):
        devs = {}
        for r in self.rows:
            if r['type'] != 'inductor':
                continue
            s, fp, _ = self.common(r)
            if not fp:
                continue
            p = self.params[r['mpn']]
            uh = sp.parse(p.get('Inductance'))
            if uh is None:
                self.err(r, 'no inductance')
                continue
            part = {'name': r['mpn'], 'manufacturer': MAKER[r['company']], 'datasheet': self.ds[r['pdf']][0],
                    'package': s['label'], 'pinout': pinout_key(s['pinout']), 'footprint': fp,
                    'henries': num(uh * 1e-6)}
            if p.get('Tolerance', '-') not in ('', '-'):
                part['tolerance'] = p['Tolerance']
            for key, name in (('amps', 'Current Rating (Amps)'), ('isat', 'Current - Saturation (Isat)')):
                v = sp.parse(p.get(name))
                if v:
                    part[key] = num(v)
            v = sp.parse(p.get('DC Resistance (DCR)'))
            if v:
                part['dcr'] = num(v)
            if p.get('Shielding', '-').lower() in ('shielded', 'semi-shielded', 'unshielded'):
                part['shielding'] = p['Shielding'].lower()
            if self.flags(r):
                part['flags'] = self.flags(r)
            cls = 'smd' if r['class'] == 'smd' else 'tht'
            band_name = band(uh, IND_LIMITS, IND_NAMES)
            key = (cls, pinout_key(s['pinout']), band_name)
            d = devs.setdefault(key, {'s': s, 'parts': [], 'pdfs': {}})
            d['parts'].append(part)
            d['pdfs'].setdefault(r['series'], (r['pdf'], s['page']))
        out = []
        for (cls, pk, bn), d in sorted(devs.items()):
            out.append({
                'id': f"ind_{cls}_{pk}_{kebab(bn)}", 'function': bn, 'description': 'Fixed inductor',
                'drawer': ['Discrete', 'Inductors', CLASS_DRAWER[cls]],
                'note': self.note(d), 'pinouts': {pk: pinout_json('inductor', d['s']['pinout'])},
                'terminals': ['1', '2'], 'parts': sorted(d['parts'], key=lambda p: p['name'])})
        return out

    def note(self, d, extra=''):
        items = sorted({f"{ser} {os.path.basename(pdf)} p.{page}" for ser, (pdf, page) in d['pdfs'].items()})
        text = 'Facts, pinouts and ordering codes from the maker datasheets: ' + '; '.join(items[:12])
        if len(items) > 12:
            text += f'; and {len(items) - 12} more'
        return text + '.' + extra

    # ---- fuse and ptc ----
    def fuses(self, ptc):
        devs = {}
        typ = 'ptc' if ptc else 'fuse'
        for r in self.rows:
            if r['type'] != typ:
                continue
            s, fp, _ = self.common(r)
            if not fp:
                continue
            p = self.params[r['mpn']]
            part = {'name': r['mpn'], 'manufacturer': MAKER[r['company']], 'datasheet': self.ds[r['pdf']][0],
                    'package': s['label'], 'pinout': pinout_key(s['pinout']), 'footprint': fp}
            if not ptc:
                amps = sp.parse(p.get('Current Rating (Amps)'))
                if amps is None:
                    self.err(r, 'no current')
                    continue
                speed = {'Fast Blow': 'fast', 'Slow Blow': 'slow', 'Medium Blow': 'medium'}.get(p.get('Response Time', ''))
                if speed is None and s['speed'] in ('fast', 'medium', 'slow'):
                    speed = s['speed']
                if speed is None:
                    self.err(r, 'no response speed')
                    continue
                part['amps'] = num(amps)
                part['speed'] = speed
                ac, dc = sp.parse(p.get('Voltage Rating - AC')), sp.parse(p.get('Voltage Rating - DC'))
                if not ac and not dc:
                    self.err(r, 'no voltage rating')
                    continue
                if ac:
                    part['voltsAc'] = num(ac)
                if dc:
                    part['voltsDc'] = num(dc)
                br = sp.parse(p.get('Breaking Capacity @ Rated Voltage'))
                if br:
                    part['breaking'] = num(br)
                value = amps
                names, limits = FUSE_NAMES, FUSE_LIMITS
            else:
                hold, trip = sp.parse(p.get('Current - Hold (Ih) (Max)')), sp.parse(p.get('Current - Trip (It)'))
                volts = first_number(p.get('Voltage - Max'))
                if not hold or not trip or not volts or trip <= hold:
                    self.err(r, 'PTC hold, trip or voltage missing or trip not above hold')
                    continue
                part.update({'hold': num(hold), 'trip': num(trip), 'volts': num(volts)})
                imax = sp.parse(p.get('Current - Max'))
                if imax:
                    part['imax'] = num(imax)
                value = hold
                names, limits = PTC_NAMES, PTC_LIMITS
            if self.flags(r):
                part['flags'] = self.flags(r)
            cls = r['class']
            bn = band(value, limits, names)
            key = (cls, pinout_key(s['pinout']), bn)
            d = devs.setdefault(key, {'s': s, 'parts': [], 'pdfs': {}, 'makers': set()})
            d['parts'].append(part)
            d['pdfs'].setdefault(r['series'] or r['company'], (r['pdf'], s['page']))
            d['makers'].add(r['company'])
        out = []
        for (cls, pk, bn), d in sorted(devs.items()):
            extra = ''
            if cls == 'cartridge':
                extra = (' Cartridge fuses sit on PCB fuse clips: the footprint is the maker\'s clip pair '
                         '(plan-47-A section 6).')
            out.append({
                'id': f"{'ptc' if ptc else 'fuse'}_{ {'smd': 'smd', 'tht': 'tht', 'cartridge': 'cart'}[cls] }_{pk}_{kebab(bn)}",
                'function': bn, 'description': 'PTC resettable fuse' if ptc else 'Fuse',
                'drawer': ['Discrete', 'Resettable fuses' if ptc else 'Fuses', CLASS_DRAWER[cls]],
                'note': self.note(d, extra), 'pinouts': {pk: pinout_json('fuse', d['s']['pinout'])},
                'type': typ, 'terminals': ['1', '2'], 'parts': sorted(d['parts'], key=lambda p: p['name'])})
        return out

    # ---- relay ----
    def relays(self):
        devs = {}
        for r in self.rows:
            if r['type'] != 'relay':
                continue
            s, fp, _ = self.common(r)
            if not fp:
                continue
            p = self.params[r['mpn']]
            coil_pairs = [c.split('-') for c in s['coils'].split(',') if c]
            entries = contact_entries(s['contacts'])
            latching = s['latching'] == '1'
            pol = s['polarized'] == '1'
            part = {'name': r['mpn'], 'manufacturer': MAKER[r['company']], 'datasheet': self.ds[r['pdf']][0],
                    'package': s['label'], 'pinout': pinout_key(s['pinout']), 'footprint': fp}
            solid = s['rtype'] == 'solid-state'
            if not solid:
                v = sp.parse(p.get('Coil Voltage'))
                if v is None:
                    self.err(r, 'no coil voltage')
                    continue
                part['coilVolts'] = num(v)
                for pair in s['coil_ohms'].split('|'):
                    if '=' in pair and abs(float(pair.split('=')[0]) - v) < 1e-9:
                        part['coilOhms'] = num(float(pair.split('=')[1]))
            amps = sp.parse(p.get('Load Current' if solid else 'Contact Rating (Current)'))
            if amps is None:
                self.err(r, 'no contact current')
                continue
            part['contactAmps'] = num(amps)
            sw = p.get('Voltage - Load' if solid else 'Switching Voltage', '')
            dcs = [float(x) for x in re.findall(r'([0-9.]+)\s*V\s*DC', sw, re.I)] if not solid else \
                [float(x) for x in re.findall(r'([0-9.]+)\s*V', sw)]
            if dcs:
                part['contactVolts'] = num(max(dcs))
            for key, name in (('operateMs', 'Operate Time'), ('releaseMs', 'Release Time')):
                v = sp.parse(p.get(name))
                if v:
                    part[key] = num(v)
            if solid:
                v = sp.parse(p.get('On-State Resistance (Max)'))
                if v:
                    part['onOhms'] = num(v)
            if self.flags(r):
                part['flags'] = self.flags(r)
            channels = 1 if latching else len(coil_pairs)
            func = arrangement(entries, latching, channels)
            key = (r['class'], s['rtype'], func, s['coils'], s['polarized'], s['contacts'], s['pinout'])
            d = devs.setdefault(key, {'s': s, 'parts': [], 'pdfs': {}, 'entries': entries, 'pairs': coil_pairs,
                                      'latching': latching, 'pol': pol, 'func': func, 'cls': r['class']})
            d['parts'].append(part)
            d['pdfs'].setdefault(r['series'], (r['pdf'], s['page']))
        out = []
        used = collections.Counter()
        for key, d in sorted(devs.items(), key=lambda kv: (kv[0][0], kv[0][2], kv[0][6], kv[0][5], kv[0][3])):
            s = d['s']
            pk = pinout_key(s['pinout'])
            base = f"relay_{d['cls']}_{kebab(d['func'])}_{pk}"
            used[base] += 1
            did = base if used[base] == 1 else f'{base}_{used[base]}'
            coils = []
            for a, b in d['pairs']:
                c = {'a': a, 'b': b}
                if d['pol'] and not d['latching'] or (d['pol'] and d['latching'] and len(d['pairs']) == 2):
                    c['polarized'] = True
                if s['rtype'] == 'solid-state':
                    c['polarized'] = True
                coils.append(c)
            dev = {'id': did, 'function': d['func'], 'description': RELAY_DESC[d['cls']],
                   'drawer': ['Switches & Relays', 'Relay', RELAY_DRAWER[d['cls']]], 'note': self.note(d),
                   'pinouts': {pk: pinout_json('relay', s['pinout'], coil_names(s['coils']))},
                   'type': s['rtype'], 'coils': coils}
            if d['latching']:
                dev['latching'] = True
            dev['contacts'] = d['entries']
            dev['parts'] = sorted(d['parts'], key=lambda p: p['name'])
            out.append(dev)
        return out

    # ---- buzzer ----
    def buzzers(self):
        devs = {}
        for r in self.rows:
            if r['type'] != 'buzzer':
                continue
            s, fp, _ = self.common(r)
            if not fp:
                continue
            p = self.params[r['mpn']]
            hz = float(s['hz']) if s['hz'] not in ('', '-') else sp.parse(r['value'])
            if not hz or hz < 1:
                self.err(r, 'no frequency')
                continue
            part = {'name': r['mpn'], 'manufacturer': MAKER[r['company']], 'datasheet': self.ds[r['pdf']][0],
                    'package': s['label'], 'pinout': pinout_key(s['pinout']).replace('f', 'f'), 'footprint': fp,
                    'hz': int(round(hz))}
            v = float(s['volts']) if s['volts'] not in ('', '-') else sp.parse(p.get('Voltage - Rated'))
            if v:
                part['volts'] = num(v)
            if s['vrange'] not in ('', '-'):
                lo, hi = (float(x) for x in s['vrange'].split('..'))
                part['voltsMin'], part['voltsMax'] = num(lo), num(hi)
            ma = sp.parse(p.get('Current - Supply'))
            if ma:
                part['ma'] = num(ma * 1000)
            db = first_number(p.get('Sound Pressure Level (SPL)'))
            if s['db'] not in ('', '-'):
                db = float(s['db'])
            if db:
                part['db'] = num(db)
            if self.flags(r):
                part['flags'] = self.flags(r)
            key = (s['driver'], s['technology'], pinout_key(s['pinout']))
            d = devs.setdefault(key, {'s': s, 'parts': [], 'pdfs': {}})
            d['parts'].append(part)
            d['pdfs'].setdefault(r['series'] or r['company'], (r['pdf'], s['page']))
        out = []
        for (drv, tech, pk), d in sorted(devs.items()):
            out.append({
                'id': f'buzzer_{drv}_{tech}_{pk}',
                'function': 'Indicator (built-in driver)' if drv == 'indicator' else 'Transducer (external drive)',
                'description': 'Piezo buzzer' if tech == 'piezo' else 'Magnetic buzzer',
                'drawer': ['Displays & Indicators', 'Buzzers'], 'note': self.note(d),
                'pinouts': {pk: pinout_json('buzzer', d['s']['pinout'])},
                'driver': drv, 'technology': tech, 'positive': '+', 'negative': '-',
                'parts': sorted(d['parts'], key=lambda p: p['name'])})
        return out


def render(kind, devices):
    return json.dumps({'format': 'schematical-parts', 'version': 1, 'kind': kind, 'devices': devices},
                      ensure_ascii=False, indent=1) + '\n'


def main(argv):
    cand = os.path.join(DATA, 'candidates.tsv')
    if '--candidates' in argv:
        i = argv.index('--candidates')
        cand = argv[i + 1]
        argv = argv[:i] + argv[i + 2:]
    elif not any(r['state'] == 'add' for r in sp.read(cand)):
        cand = os.path.join(DATA, 'candidates-provisional.tsv')
    check = '--check' in argv
    types = [a for a in argv if not a.startswith('--')]
    if not types or any(t not in sp.TYPES for t in types):
        print(__doc__)
        return 2
    g = Gen(cand, types)
    files = {}
    if 'inductor' in types:
        files['inductor'] = ('inductor', g.inductor())
    if 'fuse' in types or 'ptc' in types:
        devs = (g.fuses(False) if 'fuse' in types else []) + (g.fuses(True) if 'ptc' in types else [])
        files['fuse'] = ('fuse', sorted(devs, key=lambda d: d['id']))
    if 'relay' in types:
        files['relay'] = ('relay', g.relays())
    if 'buzzer' in types:
        files['buzzer'] = ('buzzer', g.buzzers())
    for e in g.errors:
        print('error:', e)
    if g.errors:
        print(f'{len(g.errors)} rows cannot be written')
        return 1
    diffs = 0
    for name, (kind, devs) in files.items():
        text = render(kind, devs)
        path = os.path.join(PARTS, name + '.json')
        if check:
            if not os.path.exists(path) or open(path, encoding='utf-8').read() != text:
                diffs += 1
                print('differs:', path)
        else:
            open(path, 'w', encoding='utf-8').write(text)
            print(f"{name}.json: {sum(len(d['parts']) for d in devs)} parts in {len(devs)} devices")
    if not check:
        # the datasheet entries of the PDFs used
        path = os.path.join(PARTS, 'datasheets.json')
        text = open(path, encoding='utf-8').read()
        have = set(re.findall(r'"id": ?"([^"]+)"', text))
        used = {}
        for name, (kind, devs) in files.items():
            for d in devs:
                for p in d['parts']:
                    used[p['datasheet']] = None
        new = [(i, u) for pdf, (i, u) in sorted(g.ds.items()) if i in used and i not in have]
        if new:
            end = text.rstrip().rfind(']')
            head = text[:end].rstrip()
            add = ',\n'.join('    ' + json.dumps({'id': i, 'url': u}, ensure_ascii=False, separators=(', ', ': ')) for i, u in new)
            open(path, 'w', encoding='utf-8').write(head + ',\n' + add + '\n  ]\n}\n')
        print(f'datasheets.json: {len(new)} entries added')
    else:
        print(f'{diffs} differences')
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
