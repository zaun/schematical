"""usage: apply.py <spec.json>   (from the worktree root)
spec: {"datasheets": [{"id","url"}], "footprints": [ {...} ],
       "parts": {"<kind>": {"<device id>": [part, ...]}},
       "devices": {"<kind>": [device, ...]}}
Adds to Parts/datasheets.json (sorted by id), footprints.json (appended), <kind>.json (parts sorted by name
when the device's list is sorted; devices appended). Refuses duplicates."""
import json, sys
P = 'schematical/schematical/Parts/'
spec = json.load(open(sys.argv[1]))

def dump_fp(f): return json.dumps(f, separators=(',', ':'), ensure_ascii=False)

if spec.get('datasheets'):
    p = P + 'datasheets.json'
    d = json.load(open(p))
    ids = {x['id'] for x in d['datasheets']}
    for x in spec['datasheets']:
        assert x['id'] not in ids, x['id']
        assert x['url'].startswith('https://'), x
        d['datasheets'].append(x); ids.add(x['id'])
    d['datasheets'].sort(key=lambda x: x['id'])
    open(p, 'w').write('{\n  "format": "schematical-datasheets",\n  "version": 1,\n  "datasheets": [\n' +
                       ',\n'.join('    ' + json.dumps(x, ensure_ascii=False) for x in d['datasheets']) + '\n  ]\n}\n')
    print('datasheets +%d' % len(spec['datasheets']))

if spec.get('footprints'):
    p = P + 'footprints.json'
    s = open(p).read()
    d = json.loads(s)
    ids = {x['id'] for x in d['footprints']}
    tail = '\n  ]\n}\n'
    assert s.endswith(tail)
    body = s[:-len(tail)]
    for f in spec['footprints']:
        assert f['id'] not in ids, f['id']
        ids.add(f['id'])
        body += ',\n    ' + dump_fp(f)
    open(p, 'w').write(body + tail)
    json.load(open(p))
    print('footprints +%d' % len(spec['footprints']))

dsids = {x['id'] for x in json.load(open(P + 'datasheets.json'))['datasheets']}
fpids = {x['id'] for x in json.load(open(P + 'footprints.json'))['footprints']}
kinds = set(spec.get('parts', {})) | set(spec.get('devices', {}))
allnames = None
for kind in sorted(kinds):
    p = P + kind + '.json'
    s = open(p).read()
    d = json.loads(s)
    style = None
    for kw in ({'indent': 1}, {'indent': 1, 'ensure_ascii': False}, {'indent': 2}, {'indent': 2, 'ensure_ascii': False}):
        for nl in ('\n', ''):
            if style is None and json.dumps(d, **kw) + nl == s: style = (kw, nl)
    assert style, 'no round-trip style for ' + kind
    names = {q['name'] for dev in d['devices'] for q in dev['parts']}
    n = 0
    for dev in spec.get('devices', {}).get(kind, []):
        assert dev['id'] not in {x['id'] for x in d['devices']}, dev['id']
        d['devices'].append(dev)
        for q in dev['parts']:
            assert q['name'] not in names, q['name']; names.add(q['name']); n += 1
            assert q.get('datasheet') in dsids and q.get('footprint') in fpids, q
    for did, parts in spec.get('parts', {}).get(kind, {}).items():
        dev = [x for x in d['devices'] if x['id'] == did]
        assert len(dev) == 1, did
        dev = dev[0]
        was = [q['name'] for q in dev['parts']]
        for q in parts:
            assert q['name'] not in names, q['name']; names.add(q['name']); n += 1
            assert q['datasheet'] in dsids, q['datasheet']
            assert q['footprint'] in fpids, q['footprint']
            if isinstance(dev.get('pinouts'), dict): assert q['pinout'] in dev['pinouts'], (q['name'], q['pinout'])
            dev['parts'].append(q)
        if was == sorted(was): dev['parts'].sort(key=lambda q: q['name'])
    open(p, 'w').write(json.dumps(d, **style[0]) + style[1])
    print('%s +%d parts' % (kind, n))
