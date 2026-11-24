#!/usr/bin/env python3
"""plan-54-A § 4: copy the needed datasheet PDFs into the tree.

usage: python3 -I place.py candidates.tsv <pdf dir>      copy, write each folder's datasheets.tsv, set `pdf` to the path
       python3 -I place.py --verify candidates.tsv       every needed PDF is placed; no unused PDF
       python3 -I place.py --prune candidates.tsv        (plan-54-B, C, D) drop the PDFs no `add` row uses

A PDF is needed when a `candidate` (later `add`) row's `pdf` names it. Each distinct file (by SHA-256 per family
and maker: two URLs of one document share one file) lands at datasheets/<folder>/<maker-slug>/<name>.pdf, <folder> by
the row's family: card -> connector-card-socket, module -> connector-memory-module, edge -> connector-card-edge.
<name>: for a te.com source the `DocNm` value of the query (`2309407`, `108-60121-1`) and, when the source has none,
the source's last path segment (e.g. `product-1-2309407-1.datasheet.pdf`); every other host the source's last path
segment, query string dropped, `.pdf` appended when missing, characters outside A-Za-z0-9._- replaced by `-`; a clash
with another file of the same name gets `-<first 6 hex of the md5 of the source>`.
datasheets.tsv (in each folder): path (relative to the folder), url (the maker URL of candidates.tsv), bytes, sha256,
source (the host the file was downloaded from). One line per (file, url).
--verify: with no `add` row yet the `candidate` rows' PDFs are what the tree must hold; once B, C and D have set
`state` = add, the `add` rows' are.
"""
import collections
import csv
import hashlib
import os
import re
import shutil
import sys
import urllib.parse

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..'))
FOLDER = {'card': 'connector-card-socket', 'module': 'connector-memory-module', 'edge': 'connector-card-edge'}
SLUG = {'Same Sky': 'same-sky', 'Amphenol ICC': 'amphenol', 'GCT': 'gct', 'TE Connectivity': 'te', 'Molex': 'molex',
        'Samtec': 'samtec'}
COLS = ['path', 'url', 'bytes', 'sha256', 'source']


def md5(s):
    return hashlib.md5(s.encode()).hexdigest()


def sha256(path):
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for block in iter(lambda: f.read(1 << 20), b''):
            h.update(block)
    return h.hexdigest()


def clean(name):
    if not name.lower().endswith('.pdf'):
        name += '.pdf'
    return re.sub(r'[^A-Za-z0-9._-]', '-', name)


def name_of(src):
    u = urllib.parse.urlparse(src)
    if u.netloc.endswith('te.com'):
        doc = urllib.parse.parse_qs(u.query).get('DocNm')
        if doc:
            return clean(doc[0])
    return clean(urllib.parse.unquote(u.path.rstrip('/').split('/')[-1]))


def read(tsv):
    return list(csv.DictReader(open(tsv), delimiter='\t'))


def write(tsv, rows):
    cols = list(rows[0].keys())
    with open(tsv, 'w', newline='') as out:
        w = csv.DictWriter(out, cols, delimiter='\t', lineterminator='\n')
        w.writeheader()
        w.writerows(rows)


def dest_dir(family, maker):
    return os.path.join('datasheets', FOLDER[family], SLUG[maker])


def place(tsv, pdf_dir):
    rows = read(tsv)
    need = {}   # src -> (family, maker, url)
    for r in rows:
        if r['state'] == 'candidate' and r['pdf'] and not r['pdf'].startswith('datasheets/'):
            need.setdefault((r['family'], r['maker'], r['src']), set()).add(r['url'])
    by_sha = collections.defaultdict(list)
    for (family, maker, src), urls in sorted(need.items()):
        by_sha[(family, maker, sha256(os.path.join(pdf_dir, md5(src) + '.pdf')))].append((src, urls))
    taken = {}
    path_of = {}
    lines = collections.defaultdict(list)
    for (family, maker, digest), group in sorted(by_sha.items(), key=lambda kv: kv[1][0][0]):
        first = group[0][0]
        name = name_of(first)
        folder = dest_dir(family, maker)
        rel = os.path.join(folder, name)
        if rel.lower() in taken and taken[rel.lower()] != digest:
            stem, ext = os.path.splitext(name)
            rel = os.path.join(folder, f'{stem}-{md5(first)[:6]}{ext}')
        taken[rel.lower()] = digest
        dst = os.path.join(ROOT, rel)
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        shutil.copyfile(os.path.join(pdf_dir, md5(first) + '.pdf'), dst)
        size = str(os.path.getsize(dst))
        for src, urls in group:
            path_of[(family, maker, src)] = rel
            for url in sorted(urls):
                lines[os.path.join('datasheets', FOLDER[family])].append(
                    [os.path.relpath(rel, os.path.join('datasheets', FOLDER[family])), url, size, digest,
                     urllib.parse.urlparse(src).netloc])
    for folder, ls in lines.items():
        with open(os.path.join(ROOT, folder, 'datasheets.tsv'), 'w') as out:
            out.write('\t'.join(COLS) + '\n')
            for line in sorted(ls):
                out.write('\t'.join(line) + '\n')
    for r in rows:
        k = (r['family'], r['maker'], r['pdf'])
        if r['state'] == 'candidate' and k in path_of:
            r['pdf'] = path_of[k]
    write(tsv, rows)
    print(len(set(path_of.values())), 'PDFs placed for', len(path_of), 'sources')


def tree_files():
    out = set()
    for folder in FOLDER.values():
        base = os.path.join(ROOT, 'datasheets', folder)
        if os.path.isdir(base):
            for slug in os.listdir(base):
                d = os.path.join(base, slug)
                if os.path.isdir(d):
                    out |= {os.path.join('datasheets', folder, slug, f) for f in os.listdir(d) if f.lower().endswith('.pdf')}
    return out


def listed():
    out = set()
    for folder in FOLDER.values():
        tsv = os.path.join(ROOT, 'datasheets', folder, 'datasheets.tsv')
        if os.path.exists(tsv):
            out |= {os.path.join('datasheets', folder, r['path']) for r in read(tsv)}
    return out


def needed(rows):
    picked = any(r['state'] == 'add' for r in rows)
    keep = 'add' if picked else 'candidate'
    return {r['pdf'] for r in rows if r['state'] == keep and r['pdf']}


def verify(tsv):
    need = needed(read(tsv))
    have = tree_files()
    lst = listed()
    missing = sorted(p for p in need if p not in have or p not in lst)
    unused = sorted(have - need)
    unlisted = sorted(have - lst)
    for p in missing:
        print('missing', p)
    for p in unused:
        print('unused', p)
    for p in unlisted:
        print('not in datasheets.tsv', p)
    print(f'{len(missing)} missing, {len(unused)} unused, {len(unlisted)} unlisted')
    return 1 if missing or unused or unlisted else 0


def prune(tsv):
    rows = read(tsv)
    need = {r['pdf'] for r in rows if r['state'] == 'add' and r['pdf']}
    for p in sorted(tree_files() - need):
        os.remove(os.path.join(ROOT, p))
        print('deleted', p)
    for folder in FOLDER.values():
        tsv_path = os.path.join(ROOT, 'datasheets', folder, 'datasheets.tsv')
        if not os.path.exists(tsv_path):
            continue
        keep = [line for n, line in enumerate(open(tsv_path))
                if n == 0 or os.path.join('datasheets', folder, line.split('\t')[0]) in need]
        with open(tsv_path, 'w') as out:
            out.writelines(keep)


def main():
    if sys.argv[1] == '--verify':
        sys.exit(verify(sys.argv[2]))
    if sys.argv[1] == '--prune':
        prune(sys.argv[2])
        return
    place(sys.argv[1], sys.argv[2])


main()
