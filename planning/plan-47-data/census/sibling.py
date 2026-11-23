#!/usr/bin/env python3
"""plan-47-A Phase 3 (section 4 rule 3): a same-maker PDF for download-set rows with none.

usage: python3 -I sibling.py <candidates.tsv> <pdf-dir> <out.tsv>

A download-set row whose own URL gave no PDF (textscan.py left `pdf` empty)
gets the first (by URL) downloaded PDF of the same maker and family that prints
its part number verbatim; `pdf`, `hit` (1) and `dead` are set from that PDF.
The rest stay without a PDF (B and C skip them: "no datasheet on the maker's
site").
"""
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import scancommon as sc  # noqa: E402


def main(tsv, directory, out_path):
    rows = sc.read(tsv)
    texts = {}
    for r in rows:
        if r['pdf']:
            key = (sc.FAMILY[r['type']], r['company'])
            if r['pdf'] not in texts.setdefault(key, {}):
                texts[key][r['pdf']] = sc.norm(sc.text_of(directory, r['pdf']))
    need = found = 0
    for r in rows:
        if not sc.in_set(r) or r['pdf']:
            continue
        need += 1
        for url in sorted(texts.get((sc.FAMILY[r['type']], r['company']), {})):
            hit, dead = sc.scan(texts[(sc.FAMILY[r['type']], r['company'])][url], r['mpn'])
            if hit == '1':
                r['pdf'], r['hit'], r['dead'] = url, hit, dead
                found += 1
                break
    sc.write(out_path, rows)
    print(need, 'download-set rows without their own PDF;', found, 'printed in a same-maker PDF')


if __name__ == '__main__':
    main(*sys.argv[1:4])
