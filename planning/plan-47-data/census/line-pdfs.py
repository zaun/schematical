#!/usr/bin/env python3
"""plan-47-D: for each footprint line, the datasheets and parts behind it.

usage: python3 -I line-pdfs.py <lines.txt> [<candidates.tsv>]

<lines.txt> is footprints-needed.txt or a slice of it. For every line prints its key, then
one row per datasheet (the placed PDF path) with the number of parts and up to three example
part numbers (the picked rows of candidates-provisional.tsv, else of candidates.tsv:
state `add`, else `candidate` and `reserve`), plus, for a relay, the specs.tsv pinout and
coils / contacts of the series, and for every type the `package` label. Read the drawing in
the PDF (`pdftotext -layout`, `pdftoppm -png` and look at the image) and draw the footprint.
"""
import collections
import csv
import os
import sys

DATA = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LETTER = {'inductor': 'D', 'fuse': 'E', 'ptc': 'F', 'relay': 'G', 'buzzer': 'H'}


def main(argv):
    lines = argv[0]
    cand = argv[1] if len(argv) > 1 else os.path.join(DATA, 'candidates-provisional.tsv')
    if not os.path.exists(cand):
        cand = os.path.join(DATA, 'candidates.tsv')
    rows = list(csv.DictReader(open(cand, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE))
    states = ('add',) if any(r['state'] == 'add' for r in rows) else ('candidate', 'reserve')
    by_line = collections.defaultdict(list)
    for r in rows:
        if r['state'] not in states:
            continue
        relay = r['type'] == 'relay'
        det = (r['form'] + '; ' + r['detail']) if relay else (r['detail'] if r['type'] in ('ptc', 'buzzer') else '')
        k = (LETTER[r['type']], r['type'], r['company'], r['series'] if relay else '', r['mounting'], r['package'],
             r['supplier_package'], r['size'], det)
        by_line[k].append(r)
    specs = {}
    sp_path = os.path.join(DATA, 'specs.tsv')
    for s in csv.DictReader(open(sp_path, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE):
        specs[(s['type'], s['maker'], s['series'], s['pdf'])] = s
    for line in open(lines, encoding='utf-8'):
        if line.startswith('#') or not line.strip():
            continue
        f = line.rstrip('\n').split('\t')
        k = tuple(f[:9])
        print('LINE ' + ' | '.join(k))
        by_pdf = collections.defaultdict(list)
        for r in by_line.get(k, []):
            by_pdf[r['pdf']].append(r)
        for pdf, rs in sorted(by_pdf.items()):
            ex = ', '.join(r['mpn'] for r in sorted(rs, key=lambda r: -int(r['stock']))[:3])
            print(f"  {pdf}  parts={len(rs)}  e.g. {ex}")
            if k[1] in ('relay', 'buzzer'):
                for s in specs.values():
                    if s['pdf'] == pdf and s['type'] == k[1] and s['maker'] == k[2] and (k[1] == 'buzzer' or s['series'] == k[3]):
                        print(f"      specs: label={s['label']} pads={s['pads']} pinout={s['pinout']} coils={s['coils']} "
                              f"contacts={s['contacts']}")
                        break
            else:
                for s in specs.values():
                    if s['pdf'] == pdf and s['type'] == k[1] and s['maker'] == k[2]:
                        print(f"      specs: label={s['label']} pads={s['pads']} pinout={s['pinout']}")
                        break


if __name__ == '__main__':
    main(sys.argv[1:])
