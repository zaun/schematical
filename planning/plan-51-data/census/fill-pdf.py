#!/usr/bin/env python3
"""plan-51-B phase 2: fills column 11 (pdf) of candidates.tsv and writes datasheets.tsv in each
datasheets/ic-socket/<maker>/ folder.  usage: python3 -I fill-pdf.py   (from anywhere)

Which PDF a row cites (all on the maker's own site):
  Mill-Max   every row: the catalog 088-145.pdf (the per-part pages are not PDFs; the DigiKey copy is not the
             maker's site)
  Preci-Dip  a DIP row: CATALOG-DILSOCKETS.pdf; a PLCC row (series 540): CATALOG-PGABGAPLCCSOCKETS.pdf. The
             rows whose DigiKey link is empty, a DigiKey copy or a product-page generator (AppHost, which answers
             an HTML page) cite the catalog that holds their series
  Assmann    the ASS_<n>_CO.pdf of the row's datasheet link; A 32-LC-TR (a DigiKey copy) is in ASS_6449_CO.pdf
Run it once over a candidates.tsv whose column 11 is empty; it refuses a filled one.
"""
import collections, os, re, sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..'))
TSV = os.path.join(ROOT, 'planning', 'plan-51-data', 'candidates.tsv')
SHEETS = os.path.join(ROOT, 'datasheets', 'ic-socket')

URLS = {
    ('mill-max', '088-145.pdf'): 'https://www.mill-max.com/sites/default/files/external/catalog/2018-12/088-145.pdf',
    ('preci-dip', 'CATALOG-DILSOCKETS.pdf'): 'https://precidip.com/wp-content/uploads/2024/10/CATALOG-DILSOCKETS.pdf',
    ('preci-dip', 'CATALOG-PGABGAPLCCSOCKETS.pdf'):
        'https://precidip.com/wp-content/uploads/2024/10/CATALOG-PGABGAPLCCSOCKETS.pdf',
}


def pdf_of(row):
    maker, mpn, family, link = row[0], row[1], row[3], row[9]
    if maker == 'Mill-Max':
        return ('mill-max', '088-145.pdf')
    if maker == 'Preci-Dip':
        return ('preci-dip', 'CATALOG-PGABGAPLCCSOCKETS.pdf' if family == 'PLCC' else 'CATALOG-DILSOCKETS.pdf')
    if maker == 'Assmann':
        m = re.search(r'(ASS_\d+_CO\.pdf)$', link)
        if m:
            return ('assmann', m.group(1))
        if mpn == 'A 32-LC-TR':
            return ('assmann', 'ASS_6449_CO.pdf')
    raise SystemExit(f'no PDF for {row[:3]} {link}')


lines = [l.rstrip('\n').split('\t') for l in open(TSV)]
header, rows = lines[0], lines[1:]
if any(r[10] for r in rows):
    raise SystemExit('column 11 is already filled')
cited = collections.Counter()
for r in rows:
    folder, name = pdf_of(r)
    path = os.path.join(SHEETS, folder, name)
    if not os.path.isfile(path):
        raise SystemExit(f'missing {path}')
    r[10] = f'datasheets/ic-socket/{folder}/{name}'
    cited[(folder, name)] += 1
with open(TSV, 'w') as f:
    f.write('\t'.join(header) + '\n')
    for r in rows:
        f.write('\t'.join(r) + '\n')
for folder in sorted({k[0] for k in cited}):
    with open(os.path.join(SHEETS, folder, 'datasheets.tsv'), 'w') as f:
        f.write('file\turl\tbytes\tparts\n')
        for (fo, name), n in sorted(cited.items()):
            if fo != folder:
                continue
            url = URLS.get((fo, name)) or f'https://www.assmann-wsw.com/uploads/datasheets/{name}'
            f.write(f'{name}\t{url}\t{os.path.getsize(os.path.join(SHEETS, fo, name))}\t{n}\n')
print(sum(cited.values()), 'rows,', len(cited), 'PDFs')
