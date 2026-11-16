"""usage: lineadd.py <spec.json>   (from the repository root)
For the part files kept one part per line (buffer, oscillator, switch, connector, rtc, converter).
spec: {"datasheets": [...], "footprints": [...]   (handed to apply.py's code paths),
       "parts": {"<kind>": {"<device id>": [part, ...]}},
       "devices": {"<kind>": [{"after": "<device id>" | null, "device": {...}}]}}
A part is inserted as one compact line, by name when the device's parts are sorted.  A device is written in the
files' layout: one key per line, pinouts one pin per line, parts one per line; `after` null appends it."""
import json, os, subprocess, sys
P = 'schematical/schematical/Parts/'
spec = json.load(open(sys.argv[1]))
C = dict(separators=(',', ':'), ensure_ascii=False)
# values other than pins and parts: `["a", "b"]`, or `["a","b"]` when the spec says "compact": true (switch.json)
CV = C if spec.get('compact') else dict(ensure_ascii=False)

if spec.get('datasheets') or spec.get('footprints'):
    tmp = sys.argv[1] + '.tmp'
    json.dump({k: spec[k] for k in ('datasheets', 'footprints') if k in spec}, open(tmp, 'w'))
    subprocess.run([sys.executable, '-I', os.path.join(os.path.dirname(os.path.abspath(__file__)), 'apply.py'), tmp], check=True)
    os.remove(tmp)
dsids = {x['id'] for x in json.load(open(P + 'datasheets.json'))['datasheets']}
fpids = {x['id'] for x in json.load(open(P + 'footprints.json'))['footprints']}

def check(q, names):
    assert q['name'] not in names, q['name']
    assert q['datasheet'] in dsids, q['datasheet']
    assert q['footprint'] in fpids, q['footprint']
    names.add(q['name'])

def fmt_device(dev):
    out = ['    {']
    keys = list(dev)
    for n, k in enumerate(keys):
        v = dev[k]
        last = '' if n == len(keys) - 1 else ','
        if k == 'pinouts':
            out.append('      "pinouts": {')
            pk = list(v)
            for m, name in enumerate(pk):
                out.append('        %s: [' % json.dumps(name))
                out += ['          ' + json.dumps(p, **C) + (',' if i < len(v[name]) - 1 else '') for i, p in enumerate(v[name])]
                out.append('        ]' + (',' if m < len(pk) - 1 else ''))
            out.append('      }' + last)
        elif k == 'parts':
            out.append('      "parts": [')
            out += ['        ' + json.dumps(p, **C) + (',' if i < len(v) - 1 else '') for i, p in enumerate(v)]
            out.append('      ]' + last)
        elif k in spec.get('multiline', []):
            out.append('      %s: [' % json.dumps(k))
            out += ['        ' + json.dumps(p, **CV) + (',' if i < len(v) - 1 else '') for i, p in enumerate(v)]
            out.append('      ]' + last)
        else:
            out.append('      %s: %s%s' % (json.dumps(k), json.dumps(v, **CV), last))
    out.append('    }')
    return out

for kind in sorted(set(spec.get('parts', {})) | set(spec.get('devices', {}))):
    p = P + kind + '.json'
    L = open(p).read().split('\n')
    d = json.loads('\n'.join(L))
    names = {q['name'] for dev in d['devices'] for q in dev['parts']}
    n = 0
    for did, parts in spec.get('parts', {}).get(kind, {}).items():
        start = [i for i, l in enumerate(L) if l.startswith('      "id": ' + json.dumps(did) + ',')]
        assert len(start) == 1, did
        i = next(k for k in range(start[0], len(L)) if L[k].startswith('      "parts": ['))
        j = next(k for k in range(i, len(L)) if L[k].startswith('      ]'))
        rows = [l.rstrip(',') for l in L[i + 1:j]]
        assert all(r.startswith('        {"name":') for r in rows), did
        was = [json.loads(r)['name'] for r in rows]
        dev = [x for x in d['devices'] if x['id'] == did][0]
        for q in parts:
            check(q, names); n += 1
            if isinstance(dev.get('pinouts'), dict): assert q['pinout'] in dev['pinouts'], q
            rows.append('        ' + json.dumps(q, **C))
        if was == sorted(was): rows.sort(key=lambda r: json.loads(r)['name'])
        L[i + 1:j] = [r + (',' if k < len(rows) - 1 else '') for k, r in enumerate(rows)]
    for item in spec.get('devices', {}).get(kind, []):
        dev = item['device']
        assert dev['id'] not in {x['id'] for x in json.loads('\n'.join(L))['devices']}, dev['id']
        for q in dev['parts']:
            check(q, names); n += 1
            assert q['pinout'] in dev['pinouts'], q
        block = fmt_device(dev)
        if item.get('after'):
            s = [i for i, l in enumerate(L) if l.startswith('      "id": ' + json.dumps(item['after']) + ',')]
            assert len(s) == 1, item['after']
            e = next(k for k in range(s[0], len(L)) if L[k] in ('    },', '    }'))
        else:
            e = max(k for k, l in enumerate(L) if l in ('    },', '    }'))
        tail_comma = L[e].endswith(',')
        L[e] = '    },'
        block[-1] = '    }' + (',' if tail_comma else '')
        L[e + 1:e + 1] = block
    open(p, 'w').write('\n'.join(L))
    json.load(open(p))
    print('%s +%d parts' % (kind, n))
