#!/usr/bin/env python3
"""plan-54-A § 4: download every datasheet source the candidate rows cite.

usage: python3 -I fetch.py candidates.tsv <pdf dir> [failed.tsv]

Each distinct `src` of a `candidate` row is fetched into <pdf dir>/<md5 of src>.pdf. A file that already holds a
`%PDF-` body is kept (a rerun resumes). Rules: `User-Agent: Mozilla/5.0` for every host (www.te.com answers a PDF to
urllib and 403 to curl and Node's fetch); one file at a time per host, hosts in parallel, a host 1 s apart; a 429 or
5xx is waited out (60 s) and retried, up to 5 tries; any other failure gets one retry. The body must start with
`%PDF-` and `pdfinfo` must read it. A source that fails is written to failed.tsv (src, host, reason) and its rows are
`no datasheet` for B, C and D.
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
import urllib.request
from urllib.parse import urlparse

UA = 'Mozilla/5.0'


def md5(s):
    return hashlib.md5(s.encode()).hexdigest()


def is_pdf(path):
    try:
        with open(path, 'rb') as f:
            return f.read(5) == b'%PDF-'
    except OSError:
        return False


def pdfinfo_ok(path):
    r = subprocess.run(['pdfinfo', path], capture_output=True, timeout=120)
    return r.returncode == 0


def get(url, dst):
    """'' on success, else the reason."""
    reason = ''
    for attempt in range(1, 6):
        try:
            req = urllib.request.Request(url, headers={'User-Agent': UA, 'Accept': '*/*'})
            with urllib.request.urlopen(req, timeout=120) as r:
                body = r.read()
            if body[:5] != b'%PDF-':
                return 'not a PDF'
            tmp = dst + '.part'
            with open(tmp, 'wb') as f:
                f.write(body)
            if not pdfinfo_ok(tmp):
                os.remove(tmp)
                return 'pdfinfo failed'
            os.replace(tmp, dst)
            return ''
        except urllib.error.HTTPError as e:
            reason = f'HTTP {e.code}'
            if e.code == 429 or e.code >= 500:
                time.sleep(60)
                continue
        except Exception as e:  # noqa: BLE001
            reason = type(e).__name__
        if attempt >= 2:
            return reason
        time.sleep(3)
    return reason


def run_host(job):
    host, urls, directory = job
    out = []
    for url in urls:
        dst = os.path.join(directory, md5(url) + '.pdf')
        if is_pdf(dst):
            out.append((url, host, ''))
            continue
        out.append((url, host, get(url, dst)))
        time.sleep(1)
    return out


def main():
    cand, directory = sys.argv[1], sys.argv[2]
    failed_path = sys.argv[3] if len(sys.argv) > 3 else None
    os.makedirs(directory, exist_ok=True)
    srcs = sorted({r['src'] for r in csv.DictReader(open(cand), delimiter='\t') if r['state'] == 'candidate' and r['src']})
    by = collections.defaultdict(list)
    for s in srcs:
        by[urlparse(s).netloc].append(s)
    jobs = [(h, u, directory) for h, u in by.items()]
    print(f'fetch: {len(srcs)} sources on {len(by)} hosts', file=sys.stderr, flush=True)
    failed = []
    done = 0
    with concurrent.futures.ThreadPoolExecutor(len(jobs)) as pool:
        for res in pool.map(run_host, jobs):
            for url, host, reason in res:
                done += 1
                if reason:
                    failed.append((url, host, reason))
    print(f'fetch: {done} sources, {len(failed)} failed', file=sys.stderr)
    if failed_path:
        with open(failed_path, 'w') as f:
            f.write('src\thost\treason\n')
            for row in sorted(failed):
                f.write('\t'.join(row) + '\n')
    for row in sorted(failed):
        print('\t'.join(row))


main()
