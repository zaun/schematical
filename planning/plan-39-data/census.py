#!/usr/bin/env python3 -I
"""plan-39 census: every number the plan quotes, from the part files.
Run: python3 -I planning/plan-39-data/census.py   (from the repo root)"""
import json, glob, os, collections

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
PARTS = os.path.join(ROOT, 'schematical', 'schematical', 'Parts')
IN_SCOPE = ['arith', 'buffer', 'converter', 'counter', 'cpu', 'decoder', 'dff', 'eeprom', 'jkff',
            'latch', 'mcu', 'mux', 'oscillator', 'pio', 'register', 'rtc', 'serial', 'shift', 'sram']
SIGNAL = ('input', 'output', 'inout')

def devices(kind):
    return sorted(json.load(open(os.path.join(PARTS, kind + '.json')))['devices'], key=lambda d: d['id'])

kinds = sorted(os.path.basename(f)[:-5] for f in glob.glob(os.path.join(PARTS, '*.json'))
               if os.path.basename(f) not in ('datasheets.json', 'footprints.json'))
out_of_scope = [k for k in kinds if k not in IN_SCOPE]

print('== all kinds: devices pinouts parts')
tot = collections.Counter()
for k in kinds:
    ds = devices(k)
    row = (len(ds), sum(len(d['pinouts']) for d in ds), sum(len(d.get('parts', [])) for d in ds))
    tot.update(dict(zip(('devices', 'pinouts', 'parts'), row)))
    print(k, *row)
print('TOTAL', dict(tot))

print('== in scope: kind devices pinouts signal-pins parts')
t = collections.Counter()
for k in IN_SCOPE:
    ds = devices(k)
    sp = sum(1 for d in ds for p in d['pinouts'].values() for q in p if q['role'] in SIGNAL)
    row = (len(ds), sum(len(d['pinouts']) for d in ds), sp, sum(len(d.get('parts', [])) for d in ds))
    t.update(dict(zip(('devices', 'pinouts', 'signal', 'parts'), row)))
    print(k, *row)
print('IN-SCOPE TOTAL', dict(t))

print('== out of scope kinds and their signal-pin names (none can be active low)')
for k in out_of_scope:
    names = sorted({q['name'] for d in devices(k) for p in d['pinouts'].values() for q in p if q['role'] in SIGNAL})
    print(k, len(names), names[:40])

print('== pins with a trailing # in the name, by kind (kind: pins, devices)')
for k in kinds:
    pins = sum(1 for d in devices(k) for p in d['pinouts'].values() for q in p if '#' in q['name'])
    devs = sum(1 for d in devices(k) if any('#' in q['name'] for p in d['pinouts'].values() for q in p))
    if pins: print(k, pins, devs)
print('names with # not at the end, or # and / together:',
      sum(1 for k in kinds for d in devices(k) for p in d['pinouts'].values() for q in p
          if '#' in q['name'] and (not q['name'].endswith('#') or '/' in q['name'])))

segs = collections.Counter(len(q['name'].split('/')) for k in kinds for d in devices(k)
                           for p in d['pinouts'].values() for q in p)
print('alternatives per name (n: pins):', sorted(segs.items()))
print('signal pins whose name has more than one alternative (in scope):',
      sum(1 for k in IN_SCOPE for d in devices(k) for p in d['pinouts'].values() for q in p
          if q['role'] in SIGNAL and '/' in q['name']))
print('non-signal pins whose name has # (cannot be flagged):',
      sum(1 for k in kinds for d in devices(k) for p in d['pinouts'].values() for q in p
          if q['role'] not in SIGNAL and '#' in q['name']))

print('== pins already one-per-line in the files (pin objects vs lines holding "role")')
for f in sorted(glob.glob(os.path.join(PARTS, '*.json'))):
    if os.path.basename(f) in ('datasheets.json', 'footprints.json'): continue
    text = open(f).read()
    lines = sum(1 for l in text.split('\n') if '"role"' in l)
    objs = text.count('"role"')
    if lines != objs: print('MULTI-PIN LINES', os.path.basename(f), lines, objs)
print('(no MULTI-PIN LINES above = every pin object is on its own line)')

print('== pinouts whose parts name no datasheet / pinouts no part uses')
for k in IN_SCOPE:
    for d in devices(k):
        by = collections.defaultdict(list)
        for p in d.get('parts', []): by[p['pinout']].append(p)
        for n in d['pinouts']:
            if not by.get(n): print('NO PART', k, d['id'], n)
            elif not any(p.get('datasheet') for p in by[n]): print('NO DATASHEET', k, d['id'], n)

print('== devices with >1 pinout, and of those, ones whose signal-pin name sets differ')
m = df = 0
for k in IN_SCOPE:
    for d in devices(k):
        if len(d['pinouts']) > 1:
            m += 1
            s = {frozenset(q['name'] for q in p if q['role'] in SIGNAL) for p in d['pinouts'].values()}
            df += len(s) > 1
print(m, df)

print('== sub-plan splits: converter and mcu by device (sorted by id), cumulative pinouts')
for k in ('converter', 'mcu'):
    ds = devices(k); total = sum(len(d['pinouts']) for d in ds); acc = 0
    for i, d in enumerate(ds, 1):
        acc += len(d['pinouts'])
        if acc * 2 >= total:
            print(k, 'split after device #%d %s: first half %d pinouts, rest %d, of %d devices' %
                  (i, d['id'], acc, total - acc, len(ds))); break

print('== in-scope pinouts by where their datasheet comes from')
pdfs = {os.path.basename(p)[:-4].lower() for p in glob.glob(os.path.join(ROOT, 'datasheets', '*', '*.pdf'))}
print('local PDFs (datasheets/<group>/<id>.pdf):', len(glob.glob(os.path.join(ROOT, 'datasheets', '*', '*.pdf'))))
src = collections.Counter(); url_ids = set()
for k in IN_SCOPE:
    for d in devices(k):
        by = collections.defaultdict(list)
        for p in d.get('parts', []): by[p['pinout']].append(p)
        for n in d['pinouts']:
            ids = [p['datasheet'] for p in by.get(n, []) if p.get('datasheet')]
            if not by.get(n): src['no part'] += 1
            elif not ids: src['no datasheet (local PDF from the docs table)'] += 1
            elif any(i.lower() in pdfs for i in ids): src['a local PDF'] += 1
            else:
                src['URL only'] += 1; url_ids.add(ids[0])
print(dict(src), 'sum', sum(src.values()), '; distinct first-listed URL-only ids:', len(url_ids))

print('== parts whose golden line changes when the # pins are flagged')
cnt = collections.Counter()
for k in IN_SCOPE:
    for d in devices(k):
        hp = {n for n, p in d['pinouts'].items() if any('#' in q['name'] for q in p)}
        cnt[k] += sum(1 for p in d.get('parts', []) if p['pinout'] in hp)
print({k: v for k, v in cnt.items() if v}, 'total', sum(cnt.values()))

print('== sram/eeprom pins named by chipEnable/outputEnable/writeEnable/byteEnable whose name does not end in #')
bad = []
for k in ('sram', 'eeprom'):
    for d in devices(k):
        names = set()
        for key in ('chipEnable', 'outputEnable', 'writeEnable', 'byteEnable'):
            v = d.get(key, [])
            names |= set([v] if isinstance(v, str) else v)
        bad += [(k, d['id'], n) for n in names if not n.endswith('#')]
print(len(bad), bad[:5])
