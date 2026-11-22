#!/usr/bin/env python3
"""plan-54 census: which of the URLs of urls-needed.tsv download as a PDF.

usage: python3 -I urlcheck.py urls-needed.tsv [known.tsv] > url-check.tsv
Fetches each src (an earlier url-check.tsv given as `known` answers the sources it has) with the User-Agent `Mozilla/5.0`, one request at a time per host (1 s apart; hosts in
parallel), reads the first 5 bytes. Columns: src, host, http status, ok (1 when the body starts with %PDF-).
"""
import collections
import concurrent.futures
import csv
import sys
import time
import urllib.error
import urllib.request

UA = 'Mozilla/5.0'


def check(url):
    for attempt in range(2):
        try:
            r = urllib.request.urlopen(urllib.request.Request(url, headers={'User-Agent': UA, 'Accept': '*/*'}), timeout=45)
            return r.status, 1 if r.read(5) == b'%PDF-' else 0
        except urllib.error.HTTPError as e:
            return e.code, 0
        except Exception:
            time.sleep(2)
    return 0, 0


def run_host(rows):
    out = []
    for row in rows:
        out.append((row['src'], row['host'], *check(row['src'])))
        time.sleep(1)
    return out


rows = list(csv.DictReader(open(sys.argv[1]), delimiter='\t'))
known = {}
if len(sys.argv) > 2:
    for k in csv.DictReader(open(sys.argv[2]), delimiter='\t'):
        known[k['url']] = k
by = collections.defaultdict(list)
print('src\thost\thttp\tok')
for r in rows:
    if r['src'] in known:
        k = known[r['src']]
        print(f"{r['src']}\t{r['host']}\t{k['http']}\t{k['ok']}")
    else:
        by[r['host']].append(r)
with concurrent.futures.ThreadPoolExecutor(len(by)) as pool:
    for res in pool.map(run_host, by.values()):
        for url, host, http, ok in res:
            print(f'{url}\t{host}\t{http}\t{ok}')
