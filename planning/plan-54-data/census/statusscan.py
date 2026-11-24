#!/usr/bin/env python3
"""plan-54-B/C/D Phase 2: scan every placed PDF whole for status words, with context.

usage: python3 -I statusscan.py [--family F]... [--maker M]... [--ocr] [candidates.tsv]

For each distinct `pdf` of the selected rows (state not `out`), runs `pdftotext -layout` and prints every line holding one of
OBSOLETE, DISCONTINU, NOT RECOMMENDED FOR NEW, NRND, LAST TIME BUY, END OF LIFE, EOL, SUPERSEDED, WITHDRAWN, with the
PDF's path, the page and the line (trimmed). A PDF with none prints nothing; the last line counts the PDFs scanned and
those that hit. A PDF whose text is empty (a scan) is named `no text`; with --ocr its first 4 pages are rendered
(`pdftoppm -r 150`) and read with `tesseract`, and the words found in that text are printed as `ocr pN:`.
"""
import os
import re
import subprocess
import sys
import tempfile

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs  # noqa: E402

WORDS = re.compile(r'OBSOLETE|DISCONTINUED|DISCONTINUATION|NOT RECOMMENDED FOR NEW|NRND|LAST TIME BUY|END OF LIFE|(?<![A-Z0-9])EOL(?![A-Z0-9])|SUPERSEDED|WITHDRAWN', re.I)


def main(argv):
    fams, makers, rest = [], [], []
    ocr = False
    i = 0
    while i < len(argv):
        if argv[i] == '--family':
            fams.append(argv[i + 1]); i += 2; continue
        if argv[i] == '--maker':
            makers.append(argv[i + 1]); i += 2; continue
        if argv[i] == '--ocr':
            ocr = True; i += 1; continue
        rest.append(argv[i]); i += 1
    rows = specs.candidates(rest[0] if rest else specs.CANDIDATES)
    pdfs = sorted({r['pdf'] for r in rows if r['pdf'] and r['state'] != 'out' and (not fams or r['family'] in fams)
                   and (not makers or r['maker'] in makers)})
    hits = 0
    for pdf in pdfs:
        path = os.path.join(specs.ROOT, pdf)
        if not os.path.exists(path):
            print(pdf, 'MISSING')
            continue
        text = subprocess.run(['pdftotext', '-layout', path, '-'], capture_output=True, timeout=300).stdout.decode('utf8', 'replace')
        if len(text.strip()) < 40:
            if not ocr:
                print(pdf, 'no text')
                continue
            found = False
            with tempfile.TemporaryDirectory() as tmp:
                subprocess.run(['pdftoppm', '-r', '150', '-l', '4', '-png', path, os.path.join(tmp, 'p')], timeout=600)
                for png in sorted(os.listdir(tmp)):
                    out = subprocess.run(['tesseract', os.path.join(tmp, png), '-', '--psm', '11'], capture_output=True,
                                         timeout=300).stdout.decode('utf8', 'replace')
                    for line in out.splitlines():
                        if WORDS.search(line):
                            found = True
                            print(f'{pdf} ocr {png}: {line.strip()[:160]}')
            hits += 1 if found else 0
            continue
        found = False
        for page, body in enumerate(text.split('\f'), 1):
            for line in body.splitlines():
                if WORDS.search(line):
                    found = True
                    print(f'{pdf} p{page}: {re.sub(r"[ ]{2,}", "  ", line.strip())[:160]}')
        hits += 1 if found else 0
    print(f'{len(pdfs)} PDFs scanned, {hits} with a status word')


main(sys.argv[1:])
