#!/usr/bin/env python3
"""plan-55-B § 3.4: copy the needed datasheet PDFs into the tree.

usage: python3 -I place.py <candidates.tsv> <pdf-dir>   copy, write datasheets.tsv, set `pdf` to the path
       python3 -I place.py --verify <candidates.tsv>    every needed PDF is placed; no unused PDF

A PDF is needed when a candidate row's `pdf` names it (textscan.py: the row's own URL; --sibling: a same-company
one). Each distinct file (by SHA-256: two URLs of one document share one file) lands at
datasheets/powerswitch/<company>/<name>.pdf: <name> is the last path segment of the file's first (sorted) URL,
query string dropped, `%20` -> `-`, `.pdf` appended when missing, characters outside A-Za-z0-9._- -> `-`; a clash
with another file of the same name gets `-<first 6 hex of the md5 of its URL>`.
datasheets.tsv (datasheets/powerswitch/): file, url (its first URL), bytes, sha256, one line per file. The copy rewrites `pdf` in
<candidates.tsv> from the URL to the path; --verify reads paths.
"""
import collections
import hashlib
import os
import re
import shutil
import sys
import urllib.parse

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

COLS = ['file', 'url', 'bytes', 'sha256']
PREFIX = os.path.join('datasheets', 'powerswitch')
COLS_CAND = ['family', 'company', 'mpn', 'series', 'stock', 'status', 'package', 'supplier_package', 'grade',
             'datasheet_url', 'maker_url', 'state', 'reason', 'pdf', 'hit', 'dead', 'stock_checked']


def sha256(path):
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for block in iter(lambda: f.read(1 << 20), b''):
            h.update(block)
    return h.hexdigest()


def name_of(url):
    u = urllib.parse.urlparse(url)
    segment = u.path.rstrip('/').split('/')[-1]
    segment = segment.replace('%20', '-')
    if not segment.lower().endswith('.pdf'):
        segment += '.pdf'
    return re.sub(r'[^A-Za-z0-9._-]', '-', segment)


def place(tsv, pdf_dir):
    rows = common.read_tsv(tsv)
    companies = {}
    for r in rows:
        if common.eligible(r) and r['pdf'] and not r['pdf'].startswith('datasheets/'):
            companies.setdefault(r['pdf'], r['company'])
    by_sha = collections.defaultdict(list)
    for url in sorted(companies):
        by_sha[(companies[url], sha256(common.pdf_path(pdf_dir, url)))].append(url)
    taken = {}
    lines = []
    path_of = {}
    for (company, digest), urls in sorted(by_sha.items(), key=lambda kv: kv[1][0]):
        first = urls[0]
        name = name_of(first)
        rel = os.path.join(PREFIX, company, name)
        if rel.lower() in taken and taken[rel.lower()] != digest:
            stem, ext = os.path.splitext(name)
            rel = os.path.join(PREFIX, company, f'{stem}-{common.md5(first)[:6]}{ext}')
        taken[rel.lower()] = digest
        dst = os.path.join(common.ROOT, rel)
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        shutil.copyfile(common.pdf_path(pdf_dir, first), dst)
        for url in urls:
            path_of[url] = rel
        lines.append([rel, first, str(os.path.getsize(dst)), digest])
    os.makedirs(common.TREE, exist_ok=True)
    with open(common.PLACED, 'w') as out:
        out.write('\t'.join(COLS) + '\n')
        for line in sorted(lines):
            out.write('\t'.join(line) + '\n')
    for r in rows:
        if r['pdf'] in path_of:
            r['pdf'] = path_of[r['pdf']]
    common.write_tsv(tsv, COLS_CAND, rows)
    print(len(set(path_of.values())), 'PDFs placed for', len(path_of), 'URLs')


def tree_files():
    out = set()
    for company in common.COMPANIES:
        d = os.path.join(common.TREE, company)
        if os.path.isdir(d):
            out |= {os.path.join(PREFIX, company, f) for f in os.listdir(d) if f.lower().endswith('.pdf')}
    return out


def verify(tsv):
    rows = common.read_tsv(tsv)
    need = {r['pdf'] for r in rows if common.eligible(r) and r['pdf']}
    have = tree_files()
    listed = {r['file'] for r in common.read_tsv(common.PLACED)}
    missing = sorted(p for p in need if p not in have or p not in listed)
    unused = sorted(have - need)
    for p in missing:
        print('missing', p)
    for p in unused:
        print('unused', p)
    print(f'{len(missing)} missing, {len(unused)} unused')
    return 1 if missing or unused else 0


if __name__ == '__main__':
    if sys.argv[1] == '--verify':
        sys.exit(verify(sys.argv[2]))
    place(sys.argv[1], sys.argv[2])
