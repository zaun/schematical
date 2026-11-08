"""Append DigiKey rows missing from candidates.tsv (found by the 2026-11-07 re-check) and
refresh status from the in-stock Active/LTB listings."""
import csv, json, glob
W = '/Users/justinzaun/Development/logic/.claude/worktrees/P-24'
F = W + '/planning/plan-24-data/candidates.tsv'
hdr = open(F).readline().rstrip('\n').split('\t')
rows = list(csv.DictReader(open(F), delimiter='\t'))
names = {r['name'] for r in rows}
seen = {}
for f in glob.glob('/tmp/p24/dk/list-*.tsv') + glob.glob('/tmp/p24/dk/kw-700-*.tsv'):
    for l in open(f):
        p = l.rstrip('\n').split('\t')
        seen[p[0]] = p[1]

def iface(t):
    t0 = t
    if not t or t == '-':
        return 'other:' + ('-' if t == '-' else '')
    if 'JESD' in t or 'LVDS' in t:
        return 'highspeed'
    if t == 'UART':
        return 'other:UART'
    i2c = 'I2C' in t
    spi = any(k in t for k in ('SPI', 'Serial', 'DSP', 'Microwire', 'MICROWIRE'))
    par = 'Parallel' in t
    if par and spi: return 'parallel+spi'
    if par: return 'parallel'
    if i2c and (spi or '2-Wire' in t): return 'i2c+spi'
    if i2c: return 'i2c'
    if spi: return 'spi'
    return 'other:' + t0

added = []
for f in sorted(glob.glob('/tmp/p24/dk/kwj-700-*.jsonl')):
    for line in open(f):
        d = json.loads(line)
        for p in d['Products']:
            n = p['ManufacturerProductNumber']
            if n in names or n not in seen:
                continue
            P = {x['ParameterText']: x['ValueText'] for x in p['Parameters']}
            r = {k: '' for k in hdr}
            r.update(group='ADC', interface=iface(P.get('Data Interface', '')), name=n,
                     base=(p.get('BaseProductNumber') or {}).get('Name', ''),
                     status=p['ProductStatus']['Status'], qty=str(p['QuantityAvailable']),
                     supplier_pkg=P.get('Supplier Device Package', ''),
                     digikey_interface=P.get('Data Interface', ''),
                     bits=P.get('Number of Bits', ''), datasheet_url=p.get('DatasheetUrl', ''),
                     description=p['Description']['ProductDescription'])
            rows.append(r); names.add(n); added.append(n)
changed = []
for r in rows:
    if r['name'] in seen and seen[r['name']] != r['status']:
        changed.append((r['name'], r['status'], seen[r['name']]))
        r['status'] = seen[r['name']]
with open(F, 'w') as o:
    o.write('\t'.join(hdr) + '\n')
    for r in rows:
        o.write('\t'.join(r[k] for k in hdr) + '\n')
print('added', len(added)); print('changed', changed)
