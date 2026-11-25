#!/usr/bin/env python3
"""plan-47-A section 4: copy the needed datasheet PDFs into the tree.

usage: python3 -I place.py <candidates.tsv> <clips.jsonl> <pdf-dir>   copy, write the datasheets.tsv files, set `pdf` to the path
       python3 -I place.py --verify <candidates.tsv>                  every needed PDF is placed; no unused PDF
       python3 -I place.py --prune <candidates.tsv>                   (plan-47-C) drop the PDFs no `add` row uses

A PDF is needed when a download-set row's `pdf` names it (textscan.py: the
row's own URL; sibling.py: a same-maker one) or when it is one of the six fuse
clips' datasheets. Each distinct file (by SHA-256 within one family and maker:
two URLs of one document share one file) lands at
datasheets/<family>/<maker-slug>/<name>.pdf: <family> inductor, fuse (fuses,
PTCs and clips), relay or buzzer; <maker-slug> the `company` column; <name> the
last path segment of the file's first (sorted) URL, query string dropped,
`%20` -> `-`, `.pdf` appended when missing, characters outside A-Za-z0-9._- ->
`-`; for a TE document URL its `DocNm`; a clash with another file of the same
name gets `-<first 6 hex of the md5 of its URL>`.
datasheets/<family>/datasheets.tsv: path, url, bytes, sha256, one line per URL.
The copy rewrites `pdf` in <candidates.tsv> from the URL to the path; --verify
and --prune read paths. `decision` (plan-47-C) is read by --prune when present,
else `state`.
"""
import collections
import hashlib
import os
import re
import shutil
import sys
import urllib.parse

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import scancommon as sc  # noqa: E402

COLS = ['path', 'url', 'bytes', 'sha256']


def sha256(path):
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for block in iter(lambda: f.read(1 << 20), b''):
            h.update(block)
    return h.hexdigest()


def name_of(url):
    u = urllib.parse.urlparse(url)
    q = urllib.parse.parse_qs(u.query)
    if 'DocNm' in q:
        segment = q['DocNm'][0]
    else:
        segment = u.path.rstrip('/').split('/')[-1]
    segment = segment.replace('%20', '-')
    if not segment.lower().endswith('.pdf'):
        segment += '.pdf'
    return re.sub(r'[^A-Za-z0-9._-]', '-', segment)


def dir_of(family, slug):
    return os.path.join('datasheets', family, slug)


def needed(rows, clip_list):
    """{(family, slug): set of URLs}: the download set's PDFs and the clips'."""
    out = collections.defaultdict(set)
    for r in rows:
        if sc.in_set(r) and r['pdf'] and not r['pdf'].startswith('datasheets/'):
            out[(sc.FAMILY[r['type']], r['company'])].add(r['pdf'])
    for slug, url, _ in clip_list:
        out[('fuse', slug)].add(url)
    return out


def place(tsv, clips_path, pdf_dir):
    rows = sc.read(tsv)
    cols = list(rows[0].keys())
    clip_list = sc.clips(clips_path)
    path_of = {}
    lines = collections.defaultdict(list)
    taken = {}
    for (family, slug), urls in sorted(needed(rows, clip_list).items()):
        by_sha = collections.defaultdict(list)
        for url in sorted(urls):
            by_sha[sha256(sc.pdf_path(pdf_dir, url))].append(url)
        for digest, group in sorted(by_sha.items(), key=lambda kv: kv[1][0]):
            first = group[0]
            name = name_of(first)
            rel = os.path.join(dir_of(family, slug), name)
            if rel.lower() in taken and taken[rel.lower()] != digest:
                stem, ext = os.path.splitext(name)
                rel = os.path.join(dir_of(family, slug), f'{stem}-{sc.md5(first)[:6]}{ext}')
            taken[rel.lower()] = digest
            dst = os.path.join(sc.ROOT, rel)
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            shutil.copyfile(sc.pdf_path(pdf_dir, first), dst)
            for url in group:
                path_of[(family, slug, url)] = rel
                lines[family].append([rel, url, str(os.path.getsize(dst)), digest])
    for family in sc.FAMILIES:
        if not lines[family]:
            continue
        old = os.path.join(sc.ROOT, 'datasheets', family, 'datasheets.tsv')
        if os.path.exists(old):
            have = {line[0] for line in lines[family]}
            lines[family] += [x for x in (r for r in
                              (line.rstrip('\n').split('\t') for line in open(old, encoding='utf-8')
                               if not line.startswith('path\t'))) if x[0] not in have and len(x) == len(COLS)]
        with open(os.path.join(sc.ROOT, 'datasheets', family, 'datasheets.tsv'), 'w', encoding='utf-8') as out:
            out.write('\t'.join(COLS) + '\n')
            for line in sorted(lines[family]):
                out.write('\t'.join(line) + '\n')
    for r in rows:
        key = (sc.FAMILY[r['type']], r['company'], r['pdf'])
        if key in path_of:
            r['pdf'] = path_of[key]
    sc.write(tsv, rows, cols)
    print(len(set(path_of.values())), 'PDFs placed for', len(path_of), 'URLs')


def tree_files():
    out = set()
    for family in sc.FAMILIES:
        base = os.path.join(sc.ROOT, 'datasheets', family)
        for dp, _, files in os.walk(base):
            for f in files:
                if f.lower().endswith('.pdf'):
                    out.add(os.path.relpath(os.path.join(dp, f), sc.ROOT))
    return out


def listed():
    out = set()
    for family in sc.FAMILIES:
        p = os.path.join(sc.ROOT, 'datasheets', family, 'datasheets.tsv')
        if os.path.exists(p):
            out |= {r['path'] for r in sc.read(p)}
    return out


def verify(tsv, keep):
    need = {r['pdf'] for r in sc.read(tsv) if keep(r) and r['pdf']}
    have = tree_files()
    lst = listed()
    missing = sorted(p for p in need if p not in have or p not in lst)
    unused = sorted(have - need)
    for p in missing:
        print('missing', p)
    for p in unused:
        print('unused', p)
    print(f'{len(missing)} missing, {len(unused)} unused')
    return 1 if missing or unused else 0


def decided(r):
    return (r.get('decision') or r['state']) == 'add'


def prune(tsv, keep_extra=()):
    need = {r['pdf'] for r in sc.read(tsv) if decided(r) and r['pdf']} | set(keep_extra) | clip_pdf_paths()
    for p in sorted(tree_files() - need):
        os.remove(os.path.join(sc.ROOT, p))
        print('deleted', p)
    for family in sc.FAMILIES:
        p = os.path.join(sc.ROOT, 'datasheets', family, 'datasheets.tsv')
        if os.path.exists(p):
            kept = [line for line in open(p, encoding='utf-8') if line.startswith('path\t') or line.split('\t')[0] in need]
            open(p, 'w', encoding='utf-8').writelines(kept)


FINAL = False       # main() sets it: the file carries the final pick (`add` rows)


def in_download_set(r):
    return r['state'] == 'add' if FINAL else sc.in_set(r)


def clip_pdf_paths():
    """The fuse clips' PDFs: in datasheets.tsv, not in any row's `pdf`."""
    urls = {u for _, u, _ in sc.clips(os.path.join(sc.DATA, 'clips', 'clips.jsonl'))}
    p = os.path.join(sc.ROOT, 'datasheets', 'fuse', 'datasheets.tsv')
    return {r['path'] for r in sc.read(p) if r['url'] in urls} if os.path.exists(p) else set()


def main():
    global FINAL
    if sys.argv[1] in ('--verify', '--prune'):
        FINAL = any(r['state'] == 'add' for r in sc.read(sys.argv[2]))
    if sys.argv[1] == '--verify':
        # the clips' PDFs are needed too: they are in datasheets.tsv, not in a row's `pdf`
        clip_paths = {r['path'] for r in sc.read(os.path.join(sc.ROOT, 'datasheets', 'fuse', 'datasheets.tsv'))
                      if r['url'] in {u for _, u, _ in sc.clips(os.path.join(sc.DATA, 'clips', 'clips.jsonl'))}}
        rows = sc.read(sys.argv[2])
        for p in sorted(clip_paths):
            rows.append({'state': 'add' if FINAL else 'candidate', 'pdf': p})
        tmp = os.path.join(os.path.dirname(os.path.abspath(sys.argv[2])), '.verify.tmp')
        sc.write(tmp, [{'state': r['state'], 'pdf': r['pdf']} for r in rows])
        try:
            sys.exit(verify(tmp, in_download_set))
        finally:
            os.remove(tmp)
    if sys.argv[1] == '--prune':
        prune(sys.argv[2], sys.argv[3:])
        return
    place(sys.argv[1], sys.argv[2], sys.argv[3])


if __name__ == '__main__':
    main()
