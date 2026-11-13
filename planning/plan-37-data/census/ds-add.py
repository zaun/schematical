#!/usr/bin/env python3
"""plan-37-D/E/F: add the datasheets.json entries of the add rows' PDFs.
usage: python3 -I ds-add.py <kind> <device regex>   (from the repository root)
Each PDF named by an add row of that kind and device gets {"id": <file stem>,
"url": <its datasheets.tsv URL>}, in id order; an id already present with
the same URL is left alone, with another URL is an error."""
import csv, json, re, sys

kind, rex = sys.argv[1], re.compile(sys.argv[2])
path = 'schematical/schematical/Parts/datasheets.json'
urls = {l.split('\t')[0]: l.split('\t')[1] for l in open('planning/plan-37-data/datasheets.tsv').read().splitlines()[1:]}
rows = [r for r in csv.DictReader(open('planning/plan-37-data/candidates.tsv'), delimiter='\t')
        if r['verdict'] == 'add' and r['kind'] == kind and rex.search(r['device'])]
want = {}
for r in rows:
    want[r['pdf'].split('/')[1][:-4]] = urls[r['pdf']]
entries = json.load(open(path))['datasheets']
have = {e['id']: e['url'] for e in entries}
added = 0
for i, u in sorted(want.items()):
    if i in have:
        if have[i] != u:
            sys.exit('ds-add.py: %s is %s already, not %s' % (i, have[i], u))
        continue
    entries.append({'id': i, 'url': u})
    added += 1
entries.sort(key=lambda e: e['id'])
text = '{\n  "format": "schematical-datasheets",\n  "version": 1,\n  "datasheets": [\n'
text += ',\n'.join('    ' + json.dumps(e, ensure_ascii=False).replace('", "', '", "') for e in entries)
text += '\n  ]\n}\n'
open(path, 'w').write(text)
print('%d datasheets added (%d named)' % (added, len(want)))
