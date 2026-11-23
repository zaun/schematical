#!/usr/bin/env python3
"""plan-47-A Phase 3: download every datasheet URL the download set cites.

usage: python3 -I fetch.py <candidates.tsv> <clips.jsonl> <pdf dir> [fetch.tsv]

The URLs are the distinct `url` of every `candidate` and `reserve` row (select.py
already resolved them to the URL that answered, section 4) and the datasheets of
the six fuse clips of section 6. Each is fetched with urlcheck.py's fetch_one
(Python's urllib, the Safari headers, 8 at a time, one retry); a file already in
<pdf dir> as <md5 of the URL>.pdf is reused. A body is accepted when it starts
with `%PDF-` and `pdfinfo` reads it. Prints one line per URL to <fetch.tsv>
(url, ok, status, bytes, host); the counts go to stderr. A URL that fails is
recorded in plan-47-A's Corrections with its host and its rows become `out`
(reason `no datasheet`).
"""
import concurrent.futures
import csv
import hashlib
import importlib.util
import json
import os
import subprocess
import sys
import urllib.parse

HERE = os.path.dirname(os.path.abspath(__file__))
_spec = importlib.util.spec_from_file_location('urlcheck', os.path.join(HERE, 'urlcheck.py'))
urlcheck = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(urlcheck)

CLIPS = ['01110501Z', '01020071Z', 'BK/1A5018-10-R', 'BK/1A1907-06-R', '0751.0052', '0751.0056']


def urls_needed(cand, clips):
    urls = {}
    for r in csv.DictReader(open(cand, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE):
        if r['state'] in ('candidate', 'reserve') and r['url']:
            urls.setdefault(r['url'], r['company'])
    for line in open(clips, encoding='utf-8'):
        d = json.loads(line)
        if d['mpn'] in CLIPS and d.get('datasheet'):
            urls.setdefault(d['datasheet'], 'clip')
    return urls


def check(url, cache):
    ok, status, size = urlcheck.fetch_one(url, cache)
    path = os.path.join(cache, hashlib.md5(url.encode()).hexdigest() + '.pdf')
    if ok:
        info = subprocess.run(['pdfinfo', path], capture_output=True, timeout=120)
        if info.returncode != 0:
            ok, status = 0, 'pdfinfo failed'
            os.remove(path)
    return ok, status, size


def main():
    cand, clips, cache = sys.argv[1:4]
    out = sys.argv[4] if len(sys.argv) > 4 else None
    os.makedirs(cache, exist_ok=True)
    urls = sorted(urls_needed(cand, clips))
    with concurrent.futures.ThreadPoolExecutor(8) as pool:
        res = list(pool.map(lambda u: check(u, cache), urls))
    lines = ['url\tok\tstatus\tbytes\thost']
    for u, (ok, status, size) in zip(urls, res):
        lines.append('\t'.join([u, str(ok), str(status), str(size), urllib.parse.urlparse(u).netloc]))
    if out:
        open(out, 'w', encoding='utf-8').write('\n'.join(lines) + '\n')
    good = sum(1 for ok, _, _ in res if ok)
    print(f'fetch: {len(urls)} URLs, {good} PDFs, {len(urls) - good} failed', file=sys.stderr)
    for u, (ok, status, _) in zip(urls, res):
        if not ok:
            print(f'failed\t{status}\t{u}', file=sys.stderr)


if __name__ == '__main__':
    main()
