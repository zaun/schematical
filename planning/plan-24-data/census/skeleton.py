#!/usr/bin/env python3
"""Write one work skeleton per device for the spec writers, and the batch lists.

usage: skeleton.py <GROUP> rows-<G>.tsv devices-<G>.tsv <outdir> <devices per batch>
<outdir>/<DEVICE>.json: {"device": {id, function, drawer}, "packages": {code: {pinout: "?",
footprint, package}}, "pdf", "pdfs", "parts": [[name, code, pins, poa status]]}
<outdir>/batches.tsv: batch  cluster  device
"""
import collections, csv, json, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
IDS = os.path.join(os.path.dirname(HERE), 'footprint-ids.tsv')
FAMILY = {'PDIP': 'DIP', 'CDIP': 'DIP', 'CDIP SB': 'DIP', 'MSOP': 'VSSOP', 'SC70': 'SC-70',
          'SOT-23-THIN': 'SOT-23-THIN', 'SOT-5X3': 'SOT-5X3'}

G, rows_f, dev_f, out, per = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], int(sys.argv[5])
os.makedirs(out, exist_ok=True)
fid = {}
for l in open(IDS):
    if l.startswith('#') or not l.strip():
        continue
    p = l.rstrip('\n').split('\t')
    fid[p[0]] = p[1]
rows = {r['name']: r for r in csv.DictReader(open(rows_f), delimiter='\t')}
fn = {'ADC': 'ADC', 'DAC': 'DAC', 'RTC': 'RTC'}[G]
clusters = collections.OrderedDict()
for l in open(dev_f):
    cl, dev, pdfs, codes, parts = l.rstrip('\n').split('\t')
    pk = {}
    plist = []
    for n in parts.split():
        r = rows[n]
        code = r['code'] + '-' + r['pins']
        pk[r['code']] = {'pinout': '?', 'footprint': fid.get(code, '?'),
                         'package': FAMILY.get(r['pkgtype'], r['pkgtype']), 'pins': int(r['pins'])}
        plist.append([n, r['code'], int(r['pins']), r['poa_status']])
    sk = {'device': {'id': dev, 'function': fn, 'drawer': ['Interface']},
          'packages': pk, 'pdf': sorted(pdfs.split())[0], 'pdfs': sorted(pdfs.split()),
          'parts': plist}
    json.dump(sk, open(os.path.join(out, dev + '.json'), 'w'), indent=1)
    clusters.setdefault(cl, []).append(dev)
# batches: whole clusters, about `per` devices each
b, n, lines = 1, 0, []
for cl, devs in clusters.items():
    if n and n + len(devs) > per:
        b += 1; n = 0
    for d in devs:
        lines.append(f'{b}\t{cl}\t{d}')
    n += len(devs)
open(os.path.join(out, 'batches.tsv'), 'w').write('\n'.join(lines) + '\n')
print(len(clusters), 'clusters,', sum(len(v) for v in clusters.values()), 'devices,', b, 'batches')
