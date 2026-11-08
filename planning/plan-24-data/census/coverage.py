#!/usr/bin/env python3
"""Cross-check parts.tsv against specs/ and datasheets/.

usage: coverage.py <GROUP> <datasheet dir name>   (e.g. ADC adc)
Checks: every add row's device has a spec; the spec's parts are exactly the device's add
rows; each part's package code is in the spec's packages with the same footprint; no spec
for a device without add rows; every add row's pdf is in datasheets/<dir>/; PDFs there
that no add row cites.
"""
import collections, csv, json, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
G, D = sys.argv[1], sys.argv[2]
rows = [r for r in csv.DictReader(open(os.path.join(DATA, 'parts.tsv')), delimiter='\t')
        if r['group'] == G]
add = [r for r in rows if r['decision'] == 'add']
by_dev = collections.defaultdict(list)
for r in add:
    by_dev[r['device']].append(r)
problems = 0
def bad(msg):
    global problems
    problems += 1
    print(msg)
specs = {}
for dev in by_dev:
    f = os.path.join(DATA, 'specs', dev + '.json')
    if not os.path.exists(f):
        bad(f'{dev}: no spec file'); continue
    specs[dev] = json.load(open(f))
for dev, rs in by_dev.items():
    sp = specs.get(dev)
    if not sp:
        continue
    names = {p[0] for p in sp.get('parts', [])}
    want = {r['name'] for r in rs}
    if names != want:
        bad(f'{dev}: spec parts differ: missing {sorted(want - names)}, extra {sorted(names - want)}')
    for r in rs:
        pk = sp['packages'].get(r['code'])
        if not pk:
            bad(f'{dev}: {r["name"]} package {r["code"]} not in spec packages')
        elif pk.get('footprint') != r['footprint']:
            bad(f'{dev}: {r["name"]} footprint {r["footprint"]} vs spec {pk.get("footprint")}')
        if sp['device'].get('function') != G:
            bad(f'{dev}: function {sp["device"].get("function")} not {G}')
# specs of this group with no add rows
for f in os.listdir(os.path.join(DATA, 'specs')):
    if not f.endswith('.json'):
        continue
    sp = json.load(open(os.path.join(DATA, 'specs', f)))
    if sp['device'].get('function') == G and f[:-5] not in by_dev:
        bad(f'{f[:-5]}: spec but no add rows')
pdfdir = os.path.join(ROOT, 'datasheets', D)
have = {f for f in os.listdir(pdfdir) if f.endswith('.pdf')}
cited = {r['pdf'] for r in add}
for p in sorted(cited - have):
    bad(f'{p}: cited by add rows, not in datasheets/{D}/')
for p in sorted(have - cited):
    print(f'UNCITED {p}')
print(f'{G}: {len(rows)} rows, {len(add)} add, {len(by_dev)} devices, {len(have)} PDFs, '
      f'{len(cited)} cited; {problems} problems', file=sys.stderr)
sys.exit(1 if problems else 0)
