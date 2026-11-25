#!/usr/bin/env python3
"""plan-51-F: adds column 15 `finish` to candidates.tsv: DigiKey's `Contact Finish - Mating` of the part, in lower case
(`gold`, `tin`, `tin-lead`: the spelling the `socket` kind documents), read from the raw listing of its maker
(raw/<maker id>.jsonl.gz). A part without the field gets `-`. Run it once: it does nothing when the column exists."""
import gzip, json, os

DATA = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
MAKER_ID = {'Mill-Max': 54, 'Preci-Dip': 1212, 'Assmann': 123}
TSV = os.path.join(DATA, 'candidates.tsv')
lines = [l.rstrip('\n').split('\t') for l in open(TSV)]
if 'finish' in lines[0]:
    raise SystemExit('candidates.tsv already has a finish column')
finish = {}
for maker, mid in MAKER_ID.items():
    for line in gzip.open(os.path.join(DATA, 'raw', f'{mid}.jsonl.gz'), 'rt'):
        d = json.loads(line)
        value = d['params'].get('Contact Finish - Mating')
        finish[(maker, d['mpn'])] = value.lower() if value else '-'
lines[0].append('finish')
missing = 0
for r in lines[1:]:
    r.append(finish.get((r[0], r[1]), '-'))
    missing += r[14] == '-'
with open(TSV, 'w') as f:
    f.write('\n'.join('\t'.join(r) for r in lines) + '\n')
print(len(lines) - 1, 'rows,', missing, 'without a finish')
