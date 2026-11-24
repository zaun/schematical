#!/usr/bin/env python3
"""plan-55-H § 3: write Parts/powerswitch.json from the `add` rows.

usage: python3 -I gen-parts.py --family ideal --family load [--dry-run]

Every `add` row of verdicts.tsv of the given families becomes a part: its pinout, switches, analog list and facts from its
specs.tsv row (plan-55-C), the footprint from the footprint maps (fp-check.lookup: footprint-map.tsv and its
footprint-map-*.tsv), the datasheet id from datasheets/powerswitch/datasheets.tsv, its flags from verdicts.tsv.
Devices are one per (company, PDF, type, switches, analog); a device holds the pinouts of its packages (`p<N>`, `p<N>-<k>`
for two assignments of N pins). Devices of other families already in the file are kept; same-id devices are replaced.
The full run also adds each used PDF's entry to Parts/datasheets.json (id only if absent) and appends the ready footprint
objects (footprints-F.json / footprints-G.json) the parts use to Parts/footprints.json (id only if absent). Prints
`<d> devices, <p> parts`.
"""
import collections
import importlib.util
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import common  # noqa: E402

PARTS = os.path.join(common.ROOT, 'schematical', 'schematical', 'Parts')
OUT = os.path.join(PARTS, 'powerswitch.json')
LIMITS = {'vinMin': 100, 'vinMax': 100, 'iout': 100, 'ilimit': 100, 'ron': 10, 'iq': 1}
FACT_COLS = [('vinMin', 'vin_min'), ('vinMax', 'vin_max'), ('iout', 'iout'), ('ilimit', 'ilimit'), ('ron', 'ron'), ('iq', 'iq')]
SHORT = {'ti': 'ti', 'analog-devices': 'adi', 'monolithic-power-systems': 'mps', 'onsemi': 'onsemi', 'microchip': 'mchp',
         'diodes': 'diodes'}
MAKER = {'ti': 'Texas Instruments', 'analog-devices': 'Analog Devices', 'monolithic-power-systems': 'Monolithic Power Systems',
         'onsemi': 'onsemi', 'microchip': 'Microchip Technology', 'diodes': 'Diodes Incorporated'}
DRAWER = {'load': 'Load Switches', 'limited': 'Current-Limited', 'ideal-diode': 'Ideal Diodes'}
DESCRIPTION = {'load': 'Load switch', 'limited': 'Current-limited power switch', 'ideal-diode': 'Ideal diode'}
COUNT_NAME = {1: 'Single', 2: 'Dual', 3: 'Triple', 4: 'Quad'}


def load_module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


fpcheck = load_module('fpcheck', os.path.join(HERE, 'fp-check.py'))
fpmerge = load_module('fpmerge', os.path.join(HERE, 'fp-merge.py'))


def kebab(text):
    return re.sub(r'[^a-z0-9]+', '-', text.lower()).strip('-')


def number(text):
    if text in ('', None):
        return None
    v = float(text)
    if v == 0:
        return None
    v = float(f'{v:.6g}')
    return int(v) if v == int(v) else v


PACKAGE_RULES = [
    (r'^TSOT', 'TSOT-23'), (r'^SOT-23-THIN', 'TSOT-23'), (r'^SOT-?2[3568]\b', 'SOT-23'), (r'^SOT-23', 'SOT-23'),
    (r'^SC-70', 'SC-70'), (r'^SO$', 'SOIC'), (r'^SOT-583', 'SOT-583'), (r'^SuperSOT', 'SuperSOT'),
    (r'^([UVWT])-?(DFN|QFN)\d+', r'\1-\2'), (r'^MicroFET', 'MicroFET'),
]


def package_family(supplier):
    """The part's `package`: the family, no pin count, size or material (docs/parts-format.md)."""
    s = re.sub(r'[®™]', '', supplier)
    s = re.sub(r'\s*\(.*?\)', '', s).strip()
    s = re.sub(r'^\d+-', '', s)
    s = re.sub(r'-(EP|HR)$', '', s)
    for rx, fam in PACKAGE_RULES:
        m = re.match(rx, s)
        if m:
            return m.expand(fam)
    return s


def pins_json(pins):
    out = []
    for num, name, role in pins:
        pin = {'name': name, 'role': role}
        pin = {'ball': num, **pin} if isinstance(num, str) else {'number': num, **pin}
        out.append(pin)
    return out


def write(devices):
    doc = {'format': 'schematical-parts', 'version': 1, 'kind': 'powerswitch', 'devices': devices}
    with open(OUT, 'w') as f:
        json.dump(doc, f, ensure_ascii=False, indent=1)
        f.write('\n')


def add_datasheets(sheets):
    path = os.path.join(PARTS, 'datasheets.json')
    text = open(path).read()
    have = set(re.findall(r'"id": "([^"]+)"', text))
    new = [(i, u) for i, u in sorted(sheets.items()) if i not in have]
    if not new:
        return 0
    doc = json.loads(text)
    doc['datasheets'].extend({'id': i, 'url': u} for i, u in new)
    doc['datasheets'].sort(key=lambda d: d['id'])
    lines = ',\n'.join('    ' + json.dumps(d, ensure_ascii=False) for d in doc['datasheets'])
    with open(path, 'w') as f:
        f.write('{\n  "format": "schematical-datasheets",\n  "version": 1,\n  "datasheets": [\n' + lines + '\n  ]\n}\n')
    return len(new)


def merge_footprints(used):
    target = os.path.join(PARTS, 'footprints.json')
    paths = [os.path.join(common.DATA, f'footprints-{L}.json') for L in 'FG']
    paths = [p for p in paths if os.path.exists(p)]
    have = set(re.findall(r'"id":"([^"]+)"', open(target).read()))
    need = {i for i in used if i not in have}
    return fpmerge.merge(target, paths, only=need) if need else 0


def main():
    args = sys.argv[1:]
    fams = {args[i + 1] for i, a in enumerate(args) if a == '--family'}
    dry = '--dry-run' in args
    specs = collections.defaultdict(list)
    for s in common.read_tsv(common.SPECS):
        specs[common.spec_key(s)].append(s)
    cand = {(r['family'], r['mpn']): r for r in common.read_tsv(common.CANDIDATES)}
    placed = {r['file']: r for r in common.read_tsv(common.PLACED)}
    fmap = fpcheck.load_map([])
    devices = collections.OrderedDict()
    names = collections.Counter()
    sheets = {}
    used_fp = set()
    n_parts = 0
    for v in common.read_tsv(common.VERDICTS):
        if v['verdict'] != 'add' or (fams and v['family'] not in fams):
            continue
        c = cand[(v['family'], v['mpn'])]
        spec = specs[common.spec_key(c)][int(v['spec'].rpartition('#')[2])]
        pins = json.loads(spec['pinout'])
        switches = json.loads(spec['switches'])
        analog = json.loads(spec['analog'] or '[]')
        company = v['company']
        stem = os.path.splitext(os.path.basename(c['pdf']))[0]
        dkey = (company, c['pdf'], spec['type'], json.dumps(switches, sort_keys=True), json.dumps(analog))
        if dkey not in devices:
            base = f"psw-{SHORT[company]}-{kebab(stem)}"
            used = {d['id'] for d in devices.values()}
            dev_id, k = base, 2
            while dev_id in used:
                dev_id, k = f'{base}-{k}', k + 1
            count = len(switches)
            function = ((COUNT_NAME.get(count, str(count)) + (' input' if spec['type'] == 'ideal-diode' else ' channel')))
            devices[dkey] = {'id': dev_id, 'function': function,
                             'description': f"{DESCRIPTION[spec['type']]}, {count} {'input' if spec['type'] == 'ideal-diode' else 'channel'}{'s' if count > 1 else ''}",
                             'drawer': ['Power', 'Power Switches', DRAWER[spec['type']]], 'pinouts': {},
                             'type': spec['type'], 'switches': switches, 'analog': analog, 'parts': []}
        d = devices[dkey]
        sig = json.dumps(pins)
        pn = d.setdefault('_names', {})
        if sig not in pn:
            k = sum(1 for x in pn.values() if x.startswith(f'p{len(pins)}'))
            pn[sig] = f'p{len(pins)}' if k == 0 else f'p{len(pins)}-{k + 1}'
            d['pinouts'][pn[sig]] = pins_json(pins)
        letter = 'F' if company == 'ti' else 'G'
        m = fpcheck.lookup(fmap, letter, company, c['package'], c['supplier_package'], len(pins), os.path.basename(c['pdf']), c['mpn'])
        if m is None or not m['footprint']:
            sys.exit(f"gen-parts: {c['mpn']} has no footprint-map row ({company} | {c['package']} | {c['supplier_package']} | {len(pins)} pins)")
        used_fp.add(m['footprint'])
        sid = f"{SHORT[company]}-{kebab(stem)}"
        sheets[sid] = re.sub(r'^http://', 'https://', placed[c['pdf']]['url'])
        part = {'name': c['mpn'], 'manufacturer': MAKER[company], 'package': package_family(c['supplier_package']),
                'pinout': pn[sig], 'footprint': m['footprint'], 'datasheet': sid}
        for key, col in FACT_COLS:
            val = number(spec[col])
            if val is not None:
                if not 0 < val <= LIMITS[key]:
                    sys.exit(f"gen-parts: {c['mpn']}: \"{key}\" {val} is outside (0, {LIMITS[key]}]")
                part[key] = val
        if v['flags']:
            part['flags'] = v['flags'].split(',')
        names[c['mpn']] += 1
        d['parts'].append(part)
        n_parts += 1
    dup = [n for n, k in names.items() if k > 1]
    if dup:
        sys.exit(f'gen-parts: part names listed twice: {dup[:10]}')
    new = []
    for d in devices.values():
        d.pop('_names', None)
        d['parts'].sort(key=lambda p: p['name'])
        new.append(d)
    existing = []
    if os.path.exists(OUT):
        existing = [d for d in json.load(open(OUT))['devices'] if d['id'] not in {x['id'] for x in new}]
    out = sorted(existing + new, key=lambda d: (d['drawer'], d['function'], d['id']))
    ids = [d['id'] for d in out]
    assert len(ids) == len(set(ids)), 'duplicate device ids'
    if dry:
        print(f'gen-parts: dry run: {len(new)} new devices, {n_parts} parts; {len(sheets)} datasheets, {len(used_fp)} footprints')
        return
    write(out)
    added = add_datasheets(sheets)
    merged = merge_footprints(used_fp)
    total = sum(len(d['parts']) for d in out)
    print(f'gen-parts: {OUT}: {len(out)} devices, {total} parts ({len(new)} devices, {n_parts} parts this run); '
          f'{len(sheets)} datasheets used, {added} added to datasheets.json; {merged} footprints appended to footprints.json')


if __name__ == '__main__':
    main()
