#!/usr/bin/env python3
"""plan-55-B Phase 3: each candidate row's own datasheet, and whether it prints the row's part number.

usage: python3 -I textscan.py <candidates.tsv> <pdf-dir> <out.tsv>
       python3 -I textscan.py --sibling <candidates.tsv> <pdf-dir> <out.tsv>

First form: for each candidate row whose `maker_url` downloaded as a PDF (<pdf-dir>/<md5 of the URL>.pdf):
`pdf` = that URL (place.py turns it into the file's path in the tree), `hit` = 1 when the part number (or the
ordering-table form of its base: see `keys`) stands between token boundaries in the `pdftotext -layout` text
(cached as <md5>.txt in <pdf-dir>), else 0, `dead` = 0. A row with no PDF keeps `pdf`, `hit` and `dead` empty.
--sibling (plan-55-B § 3.3 rule 4): a candidate row still without a PDF gets the first (by URL) downloaded PDF
of the same company that prints its part number; `pdf` and `hit` 1 are set from it. A row still without one
is `out`, reason `no datasheet`, `dead` 1 (C's skip reason 1: the maker has no datasheet we can fetch).
"""
import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

COLS = ['family', 'company', 'mpn', 'series', 'stock', 'status', 'package', 'supplier_package', 'grade',
        'datasheet_url', 'maker_url', 'state', 'reason', 'pdf', 'hit', 'dead', 'stock_checked']


def norm(text):
    text = text.upper().replace('‐', '-').replace('‑', '-').replace('‒', '-').replace('–', '-') \
        .replace('−', '-').replace('­', '')
    return re.sub(r'\s+', ' ', text)


def keys(mpn):
    """The strings a datasheet may print for a part: the part number, its base (TI writes `LM3880QDRVRQ1/NOPB`),
    and the ordering-table forms of the base: the packing suffix cut (`-TR`, ` TR`, `RG4`, a last `R` or `T`
    after a package code, `-7` / `-13` tape reels of Diodes)."""
    full = norm(mpn)
    base = norm(common.base_name(mpn)).strip()
    out = [full, base]
    b = base
    for pat in (r'[- ]?TR$', r'RG4$', r'[- ]?T&R$', r'-?(7|13)$', r'(?<=[A-Z])[RT]$', r'T1G$'):
        m = re.sub(pat, '', b)
        if m != b and len(m) >= 5:
            out.append(m)
            b = m
    seen = []
    for k in out:
        if k and k not in seen:
            seen.append(k)
    return seen


def text_of(directory, url):
    pdf = common.pdf_path(directory, url)
    if not common.is_pdf(pdf):
        return None
    txt = pdf[:-4] + '.txt'
    if not os.path.exists(txt):
        subprocess.run(['pdftotext', '-q', '-layout', pdf, txt], check=False, timeout=240)
    if not os.path.exists(txt):
        return None
    return open(txt, errors='replace').read()


def found(body, mpn):
    for k in keys(mpn):
        if re.search(r'(?<![A-Z0-9])' + re.escape(k) + r'(?![A-Z0-9])', body):
            return True
    return False


def main():
    args = [a for a in sys.argv[1:] if a != '--sibling']
    sibling = '--sibling' in sys.argv
    tsv, directory, out = args
    rows = common.read_tsv(tsv)
    texts = {}

    def text(url):
        if url not in texts:
            t = text_of(directory, url)
            texts[url] = None if t is None else norm(t)
        return texts[url]

    if not sibling:
        for r in rows:
            r['pdf'] = r['hit'] = r['dead'] = ''
            if not common.eligible(r) or not r['maker_url']:
                continue
            body = text(r['maker_url'])
            if body is not None:
                r['pdf'] = r['maker_url']
                r['hit'] = '1' if found(body, r['mpn']) else '0'
                r['dead'] = '0'
    else:
        by_company = {}
        for r in rows:
            if r['pdf'] and not r['pdf'].startswith('datasheets/'):
                by_company.setdefault(r['company'], set()).add(r['pdf'])
        need = hit = 0
        for r in rows:
            if not common.eligible(r) or r['pdf']:
                continue
            need += 1
            for url in sorted(by_company.get(r['company'], ())):
                body = text(url)
                if body is not None and found(body, r['mpn']):
                    r['pdf'], r['hit'], r['dead'] = url, '1', '0'
                    hit += 1
                    break
            else:
                r['state'], r['reason'], r['dead'] = 'out', 'no datasheet', '1'
        print(need, 'candidate rows without their own PDF;', hit, 'printed in a same-company PDF;', need - hit, 'out')
    common.write_tsv(out, COLS, rows)
    c = [r for r in rows if common.eligible(r)]
    print(sum(1 for r in c if r['pdf']), 'candidate rows with a PDF;', sum(1 for r in c if r['hit'] == '1'),
          'print the part number;', sum(1 for r in c if r['hit'] == '0'), 'do not')


main()
