#!/usr/bin/env python3
"""DigiKey facts of the parts of fit.tsv: one keyword search per part number,
the raw answer kept in raw/, the matching product written to facts.tsv.

usage: python3 -I census/lookup.py   (from planning/plan-43-data, DIGIKEY_* set)
A part whose raw file exists is not asked again."""
import csv, json, os, re, subprocess, sys
HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROOT = os.path.dirname(os.path.dirname(HERE))
def slug(s): return re.sub(r'[^A-Za-z0-9.+-]+', '_', s)
rows = list(csv.DictReader(open(os.path.join(HERE, 'census/fit.tsv')), delimiter='\t'))
out = []
for r in rows:
    raw = os.path.join(HERE, 'raw', slug(r['mpn']) + '.json')
    if not os.path.exists(raw):
        res = subprocess.run(['node', os.path.join(ROOT, 'scripts/digikey.js'), '--json', '--pages', '1',
                              '--limit', '10', r['mpn']], capture_output=True, text=True)
        sys.stderr.write(res.stderr)
        if not res.stdout.strip(): continue
        open(raw, 'w').write(res.stdout.splitlines()[0] + '\n')
    d = json.loads(open(raw).readline())
    hit = None
    for p in d.get('ExactMatches', []) + d.get('Products', []):
        if p['ManufacturerProductNumber'] == r['mpn'] and p['Manufacturer']['Name'] == r['maker']:
            hit = p; break
    if hit is None:
        out.append([r['kind'], r['mpn'], r['maker'], 'NOT FOUND'] + [''] * 7); continue
    par = {q['ParameterText']: q['ValueText'] for q in hit['Parameters']}
    stock = max([v.get('QuantityAvailableforPackageType') or 0 for v in hit['ProductVariations']] or [0])
    keep = '; '.join('%s=%s' % (k, v) for k, v in par.items()
                     if k not in ('Package / Case', 'Supplier Device Package', 'Mounting Type', 'Operating Temperature',
                                  'Grade', 'Qualification') and v != '-')
    out.append([r['kind'], r['mpn'], r['maker'], hit['ProductStatus']['Status'], str(stock),
                hit['Category']['Name'], par.get('Package / Case', ''), par.get('Supplier Device Package', ''),
                par.get('Mounting Type', ''), hit.get('DatasheetUrl') or '', keep])
w = csv.writer(open(os.path.join(HERE, 'facts.tsv'), 'w'), delimiter='\t', lineterminator='\n')
w.writerow(['kind', 'mpn', 'maker', 'status', 'stock', 'category', 'package_case', 'supplier_package', 'mounting',
            'datasheet_url', 'parameters'])
w.writerows(out)
print(len(out), 'rows;', sum(1 for o in out if o[3] == 'NOT FOUND'), 'not found;',
      sum(1 for o in out if o[3] != 'NOT FOUND' and int(o[4]) == 0), 'with no stock')
