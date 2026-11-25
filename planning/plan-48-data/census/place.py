#!/usr/bin/env python3
"""plan-48-A phase 3: copy the datasheet PDFs of the picked rows into the tree.
usage: python3 -I place.py <candidates.tsv> <pdf cache dir>   copy, write datasheets.tsv, set `pdf` to the path
       python3 -I place.py --verify <candidates.tsv>          every cited PDF is placed; no unused PDF

A PDF is needed when a row of state `candidate` (after phase 3, every one is
an `add` row) names its cache file in `pdf`. Each distinct file (by SHA-256,
per type and company) lands at datasheets/<timer|opamp>/<ti|adi|st|renesas>/<name>.pdf:
<name> is the last path segment of the row's `final` URL, lower case, `.pdf`
appended when missing (the Renesas document `icm7555-icm7556-datasheet`), the
characters outside a-z0-9._- replaced by `-`; a clash with another file of
the same name gets `-<first 6 hex of the md5 of its URL>`. Each folder's
datasheets.tsv has one line per URL: file, url, bytes, sha256. The copy
rewrites `pdf` in <candidates.tsv> from the cache file name to the path."""
import collections
import csv
import hashlib
import os
import re
import shutil
import sys
import urllib.parse

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
COLS = ['file', 'url', 'bytes', 'sha256']


def read(tsv):
    return list(csv.DictReader(open(tsv, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE))


def write(tsv, rows):
    cols = list(rows[0].keys())
    with open(tsv, 'w', encoding='utf-8', newline='') as f:
        f.write('\t'.join(cols) + '\n')
        for r in rows:
            f.write('\t'.join(str(r[c]) for c in cols) + '\n')


def sha256(path):
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for block in iter(lambda: f.read(1 << 20), b''):
            h.update(block)
    return h.hexdigest()


def name_of(url):
    seg = urllib.parse.urlparse(url).path.rstrip('/').split('/')[-1].lower()
    if not seg.endswith('.pdf'):
        seg += '.pdf'
    return re.sub(r'[^a-z0-9._-]', '-', seg)


def folder(r):
    return os.path.join('datasheets', r['type'], r['company'])


def place(tsv, cache):
    rows = read(tsv)
    groups = collections.defaultdict(list)  # (type, company, sha) -> rows
    for r in rows:
        if r['state'] == 'candidate' and r['pdf'] and not r['pdf'].startswith('datasheets/'):
            groups[(r['type'], r['company'], sha256(os.path.join(cache, r['pdf'])))].append(r)
    taken, lines, path_of = {}, collections.defaultdict(list), {}
    for key in sorted(groups, key=lambda k: sorted(x['final'] for x in groups[k])[0]):
        rs = groups[key]
        urls = sorted({x['final'] for x in rs})
        first = urls[0]
        rel = os.path.join(folder(rs[0]), name_of(first))
        if rel in taken and taken[rel] != key[2]:
            stem, ext = os.path.splitext(name_of(first))
            rel = os.path.join(folder(rs[0]), '%s-%s%s' % (stem, hashlib.md5(first.encode()).hexdigest()[:6], ext))
        taken[rel] = key[2]
        dst = os.path.join(ROOT, rel)
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        shutil.copyfile(os.path.join(cache, rs[0]['pdf']), dst)
        for x in rs:
            path_of[x['mpn']] = rel
        for url in urls:
            lines[os.path.dirname(rel)].append([os.path.basename(rel), url, str(os.path.getsize(dst)), key[2]])
    for d, ls in lines.items():
        with open(os.path.join(ROOT, d, 'datasheets.tsv'), 'w', encoding='utf-8', newline='') as f:
            f.write('\t'.join(COLS) + '\n')
            for line in sorted(ls):
                f.write('\t'.join(line) + '\n')
    for r in rows:
        if r['mpn'] in path_of:
            r['pdf'] = path_of[r['mpn']]
    write(tsv, rows)
    print('%d PDFs placed for %d rows' % (len(set(path_of.values())), len(path_of)))


def verify(tsv):
    rows = read(tsv)
    need = {r['pdf'] for r in rows if r['state'] == 'candidate' and r['pdf']}
    have, listed = set(), set()
    for t in ('timer', 'opamp'):
        base = os.path.join(ROOT, 'datasheets', t)
        if not os.path.isdir(base):
            continue
        for c in os.listdir(base):
            d = os.path.join(base, c)
            if not os.path.isdir(d):
                continue
            have |= {os.path.join('datasheets', t, c, f) for f in os.listdir(d) if f.endswith('.pdf')}
            ds = os.path.join(d, 'datasheets.tsv')
            if os.path.exists(ds):
                listed |= {os.path.join('datasheets', t, c, r['file']) for r in read(ds)}
    missing = sorted(p for p in need if p not in have or p not in listed)
    unused = sorted(have - need)
    for p in missing:
        print('missing', p)
    for p in unused:
        print('unused', p)
    print('%d missing, %d unused' % (len(missing), len(unused)))
    return 1 if missing or unused else 0


if __name__ == '__main__':
    if sys.argv[1] == '--verify':
        sys.exit(verify(sys.argv[2]))
    place(sys.argv[1], sys.argv[2])
