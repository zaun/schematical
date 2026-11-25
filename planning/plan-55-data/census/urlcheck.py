#!/usr/bin/env python3
"""plan-55 census: which maker URLs answer a PDF (plan-55-B Phase 2).

usage: python3 -I urlcheck.py urls-needed.tsv [known url-check.tsv] > url-check.tsv
Reads the chains of urls.py. Rank by rank: every chain's first URL is fetched (the User-Agent `Mozilla/5.0`; a 403
is tried again with a bare `curl -sL`, which onsemi's host answers), the first 5 bytes read; a chain whose URL did
not answer a PDF gets its next URL fetched in the next round. One request at a time per host, 1 s apart; hosts in
parallel. An earlier url-check.tsv given as `known` answers the URLs it has. Columns: url, host, http status,
ok (1 when the body starts with `%PDF-`). A URL is listed once.
"""
import collections
import concurrent.futures
import csv
import subprocess
import sys
import time
import urllib.error
import urllib.request
from urllib.parse import urlparse

UA = 'Mozilla/5.0'


def curl_bare(url):
    try:
        out = subprocess.run(['curl', '-sL', '--max-time', '45', '-r', '0-9', url], capture_output=True, timeout=60).stdout
        return 200, 1 if out[:5] == b'%PDF-' else 0
    except Exception:
        return 0, 0


def check(url):
    code = 0
    for attempt in range(2):
        try:
            r = urllib.request.urlopen(urllib.request.Request(url, headers={'User-Agent': UA, 'Accept': '*/*'}), timeout=45)
            return r.status, 1 if r.read(5) == b'%PDF-' else 0
        except urllib.error.HTTPError as e:
            code = e.code
            if e.code == 403:
                bare = curl_bare(url)
                return (bare[0] or code), bare[1]
            if e.code in (404, 410):
                return e.code, 0
            time.sleep(2)
        except Exception:
            time.sleep(2)
    return code, 0


def run_host(urls):
    out = []
    for u in urls:
        out.append((u, *check(u)))
        time.sleep(1)
    return out


def main():
    rows = list(csv.DictReader(open(sys.argv[1]), delimiter='\t'))
    chains = [r['chain'].split() for r in rows]
    known = {}
    if len(sys.argv) > 2:
        for k in csv.DictReader(open(sys.argv[2]), delimiter='\t'):
            known[k['url']] = k
    results = {}
    print('url\thost\thttp\tok')
    rank = 0
    pending = [c for c in chains]
    while pending:
        need = collections.OrderedDict()
        for c in pending:
            u = c[rank]
            if u in results:
                continue
            if u in known:
                results[u] = (int(known[u]['http'] or 0), int(known[u]['ok']))
                print(f"{u}\t{urlparse(u).netloc}\t{known[u]['http']}\t{known[u]['ok']}")
            else:
                need.setdefault(urlparse(u).netloc, []).append(u)
        if need:
            with concurrent.futures.ThreadPoolExecutor(len(need)) as pool:
                for res in pool.map(run_host, [sorted(set(v)) for v in need.values()]):
                    for url, http, ok in res:
                        results[url] = (http, ok)
                        print(f'{url}\t{urlparse(url).netloc}\t{http}\t{ok}', flush=True)
        pending = [c for c in pending if not results[c[rank]][1] and len(c) > rank + 1]
        rank += 1
        pending = [c for c in pending]
        # only chains still failing continue; a chain shorter than the rank is done


main()
