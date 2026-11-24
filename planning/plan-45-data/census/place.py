#!/usr/bin/env python3
"""plan-45-A § 4: place the downloaded datasheets in the tree and write the datasheets.tsv files.

usage: python3 -I place.py <candidates.tsv> <pdf-dir>   copy, write the four datasheets.tsv, set `pdf`
       python3 -I place.py --verify                      every `candidate` / `add` row has its file, every file a row
       python3 -I place.py --prune                       (plan-45-B) delete the files no `add` row uses

<pdf-dir> is where fetch.py put <md5 of the URL>.pdf. Each distinct file (by SHA-256 within one maker
folder: two URLs of one document share one file) lands at

    datasheets/connector-terminal/phoenix|weidmuller|wurth/<name>.pdf     Phoenix Contact, Weidmuller, Wurth
    datasheets/connector-jst/<name>.pdf                                   JST

<name> is the part number for the per-item makers (the Phoenix item of `/products/<item>/pdf`, the
Weidmuller item of `/pdf/en/<item>/scope/2/`, the Wurth file's own name) and the URL path's last segment
for JST (query string dropped, `.pdf` appended when missing); characters outside A-Za-z0-9._- become `-`;
a name taken by another file gets `-<first 6 hex of the md5 of the URL>`. Every folder gets a
datasheets.tsv: path, url, bytes, sha256, source (`maker`, or `digikey-copy` when the bytes came from
DigiKey's copy). A row whose file is not in <pdf-dir> becomes `out`, reason `no datasheet`.
The copy rewrites `pdf` in candidates.tsv to the placed path.
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
DATA = os.path.join(ROOT, 'planning', 'plan-45-data')
CAND = os.path.join(DATA, 'candidates.tsv')
FOLDER = {'Phoenix Contact': 'datasheets/connector-terminal/phoenix',
          'Weidmuller': 'datasheets/connector-terminal/weidmuller',
          'Wurth Elektronik': 'datasheets/connector-terminal/wurth',
          'JST': 'datasheets/connector-jst'}
COLS = ['path', 'url', 'bytes', 'sha256', 'source']


def md5(text):
    return hashlib.md5(text.encode()).hexdigest()


def sha256(path):
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for block in iter(lambda: f.read(1 << 20), b''):
            h.update(block)
    return h.hexdigest()


def clean(name):
    return re.sub(r'[^A-Za-z0-9._-]', '-', name)


def name_of(maker, url):
    u = urllib.parse.urlparse(url)
    parts = [p for p in u.path.split('/') if p]
    if maker == 'Phoenix Contact':            # /us/products/<item>/pdf
        stem = parts[parts.index('products') + 1]
    elif maker == 'Weidmuller':               # /pdf/en/<item>/scope/2/
        stem = parts[parts.index('en') + 1]
    else:                                     # Wurth: <item>.pdf; JST: eXH.pdf
        stem = parts[-1]
    stem = urllib.parse.unquote(stem)
    if not stem.lower().endswith('.pdf'):
        stem += '.pdf'
    return clean(stem)


def read(tsv):
    return list(csv.DictReader(open(tsv), delimiter='\t'))


def write(tsv, rows):
    cols = list(rows[0].keys())
    with open(tsv, 'w') as out:
        out.write('\t'.join(cols) + '\n')
        for r in rows:
            out.write('\t'.join(str(r[c]) for c in cols) + '\n')


def placed_tsv(folder):
    return os.path.join(ROOT, folder, 'datasheets.tsv')


def place(cand, pdf_dir):
    rows = read(cand)
    by_url = {}
    for r in rows:
        if r['state'] == 'candidate':
            by_url.setdefault(r['url'], r)
    missing = set()
    groups = collections.defaultdict(list)            # (maker, sha) -> [url]
    for url, r in sorted(by_url.items()):
        f = os.path.join(pdf_dir, md5(url) + '.pdf')
        if not os.path.exists(f):
            missing.add(url)
            continue
        groups[(r['maker'], sha256(f))].append(url)
    taken = {}
    lines = collections.defaultdict(list)
    path_of = {}
    for (maker, digest), urls in sorted(groups.items(), key=lambda kv: kv[1][0]):
        first = urls[0]
        rel = FOLDER[maker] + '/' + name_of(maker, first)
        if rel.lower() in taken and taken[rel.lower()] != digest:
            stem, ext = os.path.splitext(name_of(maker, first))
            rel = f'{FOLDER[maker]}/{stem}-{md5(first)[:6]}{ext}'
        taken[rel.lower()] = digest
        dst = os.path.join(ROOT, rel)
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        shutil.copyfile(os.path.join(pdf_dir, md5(first) + '.pdf'), dst)
        for url in urls:
            src = by_url[url]['src']
            lines[FOLDER[maker]].append([rel, url, str(os.path.getsize(dst)), digest,
                                         'maker' if src == url else 'digikey-copy'])
            path_of[url] = rel
    for folder, ls in lines.items():
        with open(placed_tsv(folder), 'w') as out:
            out.write('\t'.join(COLS) + '\n')
            for line in sorted(ls):
                out.write('\t'.join(line) + '\n')
    gone = 0
    for r in rows:
        if r['state'] != 'candidate':
            continue
        if r['url'] in path_of:
            r['pdf'] = path_of[r['url']]
        else:
            r['state'], r['reason'], r['pdf'] = 'out', 'no datasheet', ''
            gone += 1
    write(cand, rows)
    for url in sorted(missing):
        print('no file for', url)
    print(f'{len(set(path_of.values()))} files placed for {len(path_of)} URLs; {gone} rows out (no datasheet)')


def tree_files():
    out = set()
    for folder in FOLDER.values():
        d = os.path.join(ROOT, folder)
        if os.path.isdir(d):
            out |= {f'{folder}/{f}' for f in os.listdir(d) if f.lower().endswith('.pdf')}
    return out


def listed_files():
    out = {}
    for folder in FOLDER.values():
        t = placed_tsv(folder)
        if os.path.exists(t):
            for r in csv.DictReader(open(t), delimiter='\t'):
                out[r['path']] = r
    return out


def verify(keep):
    rows = [r for r in read(CAND) if keep(r)]
    have, listed = tree_files(), listed_files()
    need = {r['pdf'] for r in rows if r['pdf']}
    nofile = [r['mpn'] for r in rows if not r['pdf'] or r['pdf'] not in have or r['pdf'] not in listed]
    unused = sorted(have - need)
    bad = []
    for p in sorted(need & have & set(listed)):
        r = listed[p]
        if os.path.getsize(os.path.join(ROOT, p)) != int(r['bytes']) or sha256(os.path.join(ROOT, p)) != r['sha256']:
            bad.append(p)
    for m in nofile[:20]:
        print('no file for row', m)
    for p in unused:
        print('unused', p)
    for p in bad:
        print('bytes or sha256 differ from datasheets.tsv:', p)
    ok = not nofile and not unused and not bad
    print(('ok: ' if ok else 'FAILED: ') + f'{len(need & have)} files, {len(nofile)} rows without a file, '
          f'{len(unused)} unused, {len(bad)} changed')
    return 0 if ok else 1


def prune():
    rows = read(CAND)
    need = {r['pdf'] for r in rows if r['state'] == 'add' and r['pdf']}
    for p in sorted(tree_files() - need):
        os.remove(os.path.join(ROOT, p))
        print('deleted', p)
    for folder in FOLDER.values():
        t = placed_tsv(folder)
        if os.path.exists(t):
            kept = [ln for ln in open(t) if ln.startswith('path\t') or ln.split('\t')[0] in need]
            with open(t, 'w') as out:
                out.writelines(kept)


def main():
    if len(sys.argv) > 1 and sys.argv[1] == '--verify':
        # once B has set `add` rows the pick is what the tree must hold
        picked = any(r['state'] == 'add' for r in read(CAND))
        sys.exit(verify((lambda r: r['state'] == 'add') if picked else (lambda r: r['state'] == 'candidate')))
    if len(sys.argv) > 1 and sys.argv[1] == '--prune':
        prune()
        return
    place(sys.argv[1], sys.argv[2])


main()
