"""usage: devadd.py <spec.json>   (from the repository root)
For rtc.json and converter.json (json.dump indent 1 layout, not byte-stable under a reload):
appends each device of spec["devices"][<kind>] as text at the end of the kind's devices array.
spec["datasheets"] / spec["footprints"] go through apply.py first (skipped with --devices-only, for a re-run
after they are in)."""
import json, os, subprocess, sys
P = 'schematical/schematical/Parts/'
spec = json.load(open(sys.argv[1]))
if (spec.get('datasheets') or spec.get('footprints')) and '--devices-only' not in sys.argv:
    tmp = sys.argv[1] + '.tmp'
    json.dump({k: spec[k] for k in ('datasheets', 'footprints') if k in spec}, open(tmp, 'w'))
    subprocess.run([sys.executable, '-I', os.path.join(os.path.dirname(os.path.abspath(__file__)), 'apply.py'), tmp], check=True)
    os.remove(tmp)
dsids = {x['id'] for x in json.load(open(P + 'datasheets.json'))['datasheets']}
fpids = {x['id'] for x in json.load(open(P + 'footprints.json'))['footprints']}
for kind, devs in spec.get('devices', {}).items():
    p = P + kind + '.json'
    s = open(p).read()
    d = json.loads(s)
    ids = {x['id'] for x in d['devices']}
    names = {q['name'] for x in d['devices'] for q in x['parts']}
    tail = [t for t in ('\n ]\n}\n', '\n ]\n}') if s.endswith(t)]
    assert tail, kind
    tail = tail[0]
    body = s[:-len(tail)]
    for dev in devs:
        assert dev['id'] not in ids, dev['id']
        for q in dev['parts']:
            assert q['name'] not in names and q['datasheet'] in dsids and q['footprint'] in fpids and q['pinout'] in dev['pinouts'], q
            names.add(q['name'])
        text = json.dumps(dev, indent=1, ensure_ascii=False)
        body += ',\n' + '\n'.join('  ' + l for l in text.split('\n'))
        ids.add(dev['id'])
    open(p, 'w').write(body + tail)
    json.load(open(p))
    print('%s +%d devices' % (kind, len(devs)))
