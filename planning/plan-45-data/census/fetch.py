#!/usr/bin/env python3
"""plan-45-A § 4: download the datasheet of every candidate row into <pdf-dir>/<md5 of the URL>.pdf.

usage: python3 -I fetch.py <candidates.tsv> <pdf-dir> [--skip <host>]...
       (--skip mm.digikey.com leaves DigiKey's copies for a run when Safari is free)

The set is the distinct (`url`, `src`) of the rows whose `state` is `candidate`: `url` names the file
(the maker's URL, what datasheets.tsv records), `src` is where the bytes come from (the same URL, or
DigiKey's copy of a Phoenix Contact per-item file). Rules:

* `User-Agent` `Mozilla/5.0` for every host except jst-mfg.com and jst.com (the Safari string of
  urlcheck.py; www.jst.com answers 403 to Node and curl, 200 to urllib with it).
* Hosts run in parallel with each other, one file at a time within a host: datasheet.weidmueller.com
  and jst-mfg.com / jst.com 1 s apart, www.we-online.com 5 s apart (it throttles), and a 403 from it is
  waited out (120 s) and retried, up to 5 tries; any other host gets one retry.
* mm.digikey.com (DigiKey's copies of the Phoenix files) answers scripts with a Cloudflare challenge
  since 2026-11-23: those go through scripts/safari-digikey-pdf.sh (a tab in Safari's DigiKey window),
  one at a time, one retry.
* A body that does not start with `%PDF-` is not kept, and `pdfinfo` must read it. A cached file that
  is a PDF is kept, so a run can be repeated.

Writes <pdf-dir>/index.tsv: md5, the attempt that worked or the last HTTP status, bytes, `%PDF` when
it is one, url, src. Exit 1 when a file is missing at the end (it lists them: their rows are `out`,
`no datasheet`). The files are only read afterwards, by pdftotext and pdfinfo; nothing runs them.
"""
import collections
import concurrent.futures
import csv
import hashlib
import os
import subprocess
import sys
import time
import urllib.error
import urllib.parse
import urllib.request

UA = 'Mozilla/5.0'
SAFARI = ('Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) '
          'Version/17.0 Safari/605.1.15')
ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..'))
SAFARI_PDF = os.path.join(ROOT, 'scripts', 'safari-digikey-pdf.sh')
GAP = {'www.we-online.com': 5, 'we-online.com': 5}      # seconds between two files of a host; default 1
WAIT_403 = 120
TRIES_403 = 5


def md5(text):
    return hashlib.md5(text.encode()).hexdigest()


def is_pdf(path):
    try:
        with open(path, 'rb') as f:
            return f.read(5) == b'%PDF-'
    except OSError:
        return False


def pdfinfo_ok(path):
    try:
        return subprocess.run(['pdfinfo', path], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                              timeout=60).returncode == 0
    except subprocess.TimeoutExpired:
        return False


def get_http(src, dest):
    """One urllib request; returns the HTTP status (0 on a transport error). Keeps only a PDF."""
    host = urllib.parse.urlparse(src).netloc
    ua = SAFARI if ('jst-mfg.com' in host or 'jst.com' in host) else UA
    try:
        r = urllib.request.urlopen(urllib.request.Request(src, headers={'User-Agent': ua, 'Accept': '*/*'}), timeout=120)
        body = r.read()
        status = r.status
    except urllib.error.HTTPError as e:
        return e.code
    except Exception:
        return 0
    if not body.startswith(b'%PDF-'):
        return status * 1000          # 200000: a page that is not a PDF
    with open(dest + '.part', 'wb') as f:
        f.write(body)
    os.replace(dest + '.part', dest)
    return status


def get_safari(src, dest):
    """DigiKey's own PDF host, through Safari (scripts/safari-digikey-pdf.sh)."""
    part = dest + '.part'
    try:
        subprocess.run([SAFARI_PDF, src, part], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=180)
    except subprocess.TimeoutExpired:
        pass
    if is_pdf(part):
        os.replace(part, dest)
        return 200
    if os.path.exists(part):
        os.remove(part)
    return -1


def fetch_one(url, src, dest):
    """Returns (attempt-or-status, ok)."""
    host = urllib.parse.urlparse(src).netloc
    if is_pdf(dest) and pdfinfo_ok(dest):
        return 'cached', True
    if host == 'mm.digikey.com':
        for attempt in (1, 2):
            status = get_safari(src, dest)
            if status == 200 and pdfinfo_ok(dest):
                return f'safari-{attempt}', True
        return 'safari-failed', False
    status = 0
    tries = TRIES_403 if host in GAP else 2
    for attempt in range(1, tries + 1):
        status = get_http(src, dest)
        if status == 200 and is_pdf(dest) and pdfinfo_ok(dest):
            return f'try-{attempt}', True
        if status == 403 and host in GAP and attempt < tries:
            print(f'fetch.py: {host} 403 for {src}; waiting {WAIT_403} s', file=sys.stderr, flush=True)
            time.sleep(WAIT_403)
        elif attempt < tries:
            time.sleep(2)
    if os.path.exists(dest):
        os.remove(dest)
    return str(status), False


def run_host(host, items, pdf_dir):
    out = []
    gap = GAP.get(host, 1)
    for i, (url, src) in enumerate(items):
        dest = os.path.join(pdf_dir, md5(url) + '.pdf')
        was_cached = is_pdf(dest)
        how, ok = fetch_one(url, src, dest)
        size = os.path.getsize(dest) if ok else 0
        out.append((md5(url), how, size, '%PDF' if ok else '', url, src))
        print(f'fetch.py: {host} {i + 1}/{len(items)} {"ok" if ok else "FAILED"} {how} {url}', file=sys.stderr, flush=True)
        if not was_cached and host != 'mm.digikey.com':
            time.sleep(gap)
    return out


def main():
    args = sys.argv[1:]
    skip = set()
    while '--skip' in args:                    # --skip <host>: leave that host for a later run
        i = args.index('--skip')
        skip.add(args[i + 1])
        del args[i:i + 2]
    cand, pdf_dir = args[0], args[1]
    os.makedirs(pdf_dir, exist_ok=True)
    seen = {}
    for r in csv.DictReader(open(cand), delimiter='\t'):
        if r['state'] == 'candidate' and r['url'] and r['src']:
            seen.setdefault(r['url'], r['src'])
    by_host = collections.defaultdict(list)
    for url, src in sorted(seen.items()):
        host = urllib.parse.urlparse(src).netloc
        if host not in skip:
            by_host[host].append((url, src))
    print(f'fetch.py: {len(seen)} URLs on {len(by_host)} hosts: ' +
          ', '.join(f'{h} {len(v)}' for h, v in sorted(by_host.items())), file=sys.stderr, flush=True)
    results = []
    with concurrent.futures.ThreadPoolExecutor(len(by_host)) as pool:
        for part in pool.map(lambda kv: run_host(kv[0], kv[1], pdf_dir), sorted(by_host.items())):
            results += part
    index_path = os.path.join(pdf_dir, 'index.tsv')
    merged = {}
    if os.path.exists(index_path):                 # keep what an earlier run (another host) wrote
        for line in open(index_path).read().splitlines()[1:]:
            f = line.split('\t')
            merged[f[4]] = tuple(f)
    for row in results:
        merged[row[4]] = tuple(str(c) for c in row)
    with open(index_path, 'w') as f:
        f.write('md5\thow\tbytes\tpdf\turl\tsrc\n')
        for url in sorted(merged):
            f.write('\t'.join(merged[url]) + '\n')
    failed = [r for r in results if not r[3]]
    for r in failed:
        print(f'fetch.py: missing {r[4]} (via {r[5]}): {r[1]}', file=sys.stderr)
    print(f'fetch.py: {len(results) - len(failed)} of {len(results)} files', file=sys.stderr)
    sys.exit(1 if failed else 0)


main()
