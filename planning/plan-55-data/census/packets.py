#!/usr/bin/env python3
"""plan-55-C: split the unread spec rows into reader packets.

usage: python3 -I packets.py [<packet dir>] [<pdfs per packet>]     default /tmp/p55/packets, 8

Groups the spec rows of specs.tsv that are neither read nor skip rows and that a candidate row uses, by (company, family,
PDF), and deals whole PDFs into packets of at most <n> PDFs of one company and one family. Writes
<dir>/pNN/assign.tsv: one line per spec key with `pdf`, `text` (the pdftotext file, /tmp/p55/text/...), `package`,
`supplier_package`, `family`, `rows`, `parts` (`mpn:hit` of its candidate rows, most-stocked first) and `digikey` (the
DigiKey parameters of the most-stocked part: Switch Type, Number of Outputs, Voltage - Load, Current - Output (Max),
Rds On (Typ), Features, Fault Protection). The reader writes <dir>/pNN/out.tsv (the columns of specs.tsv).
"""
import collections
import csv
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

out = sys.argv[1] if len(sys.argv) > 1 else '/tmp/p55/packets'
per = int(sys.argv[2]) if len(sys.argv) > 2 else 8
raw = {}
import gzip, glob  # noqa: E402,E401
for f in sorted(glob.glob(os.path.join(common.DATA, 'raw', '*.jsonl*'))):
    for line in gzip.open(f, 'rt'):
        r = json.loads(line)
        raw.setdefault((r['family'], r['mpn']), r)
cand = collections.defaultdict(list)
for c in common.read_tsv(common.CANDIDATES):
    if common.eligible(c):
        cand[common.spec_key(c)].append(c)
keys = {}
for s in common.read_tsv(common.SPECS):
    k = common.spec_key(s)
    if s['skip'] == '' and not s['pinout'] and cand.get(k) and k not in keys:
        keys[k] = s
groups = collections.defaultdict(list)
for k in sorted(keys):
    fam = cand[k][0]['family']
    groups[(k[0], fam, k[2])].append(k)
packets = []
by_cf = collections.defaultdict(list)
for (company, fam, pdf), ks in sorted(groups.items()):
    by_cf[(company, fam)].append((pdf, ks))
for (company, fam), items in sorted(by_cf.items()):
    # balance by key count: fill packets of at most `per` PDFs and about 14 keys
    cur, n = [], 0
    for pdf, ks in items:
        if cur and (len(cur) >= per or n + len(ks) > 16):
            packets.append(cur)
            cur, n = [], 0
        cur.append((pdf, ks))
        n += len(ks)
    if cur:
        packets.append(cur)
os.makedirs(out, exist_ok=True)
for i, pk in enumerate(packets, 1):
    d = os.path.join(out, f'p{i:02d}')
    os.makedirs(d, exist_ok=True)
    with open(os.path.join(d, 'assign.tsv'), 'w') as fh:
        w = csv.writer(fh, delimiter='\t', lineterminator='\n')
        w.writerow(['company', 'family', 'pdf', 'text', 'package', 'supplier_package', 'rows', 'parts', 'digikey'])
        for pdf, ks in pk:
            for k in ks:
                cs = sorted(cand[k], key=lambda c: -int(c['stock'] or 0))
                r0 = raw[(cs[0]['family'], cs[0]['mpn'])]['params']
                dk = {x: r0.get(x, '') for x in ('Switch Type', 'Number of Outputs', 'Voltage - Load', 'Current - Output (Max)',
                                                  'Rds On (Typ)', 'Features', 'Fault Protection')}
                text = '/tmp/p55/text/' + pdf[len('datasheets/powerswitch/'):][:-4] + '.txt'
                w.writerow([k[0], cs[0]['family'], pdf, text, k[3], k[4], len(cs),
                            ' '.join(f"{c['mpn'].replace(' ', '_')}:{c['hit'] or 0}" for c in cs), json.dumps(dk)])
    print(f'p{i:02d}: {len(pk)} pdfs, {sum(len(ks) for _, ks in pk)} keys, {packets and sum(len(cand[k]) for _, ks in pk for k in ks)} parts '
          f'({pk[0][1][0][0]}, {len(common.read_tsv(os.path.join(d, "assign.tsv")))} lines)')
