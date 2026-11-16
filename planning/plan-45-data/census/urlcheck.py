#!/usr/bin/env python3
"""plan-45 census: which of the URLs of urls-needed.tsv download as a PDF.

usage: python3 -I urlcheck.py urls-needed.tsv > url-check.tsv
Fetches each `src` with the User-Agent `Mozilla/5.0` (Wurth, Weidmuller and DigiKey's copy of
Phoenix Contact files answer it; we-online.com refuses curl's and Safari's agents) or, for
jst-mfg.com / jst.com, a Safari User-Agent (they refuse `Mozilla/5.0`),
4 at a time, and reads the first 5 bytes. Columns: url, src, http status, ok (1 when the body
starts with %PDF-), size of the first read.
"""
import concurrent.futures
import csv
import sys
import urllib.request

UA = 'Mozilla/5.0'
SAFARI = 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.0 Safari/605.1.15'


def check(row):
    src = row['src']
    if not src:
        return row['url'], src, 0, 0
    for attempt in range(3):
        try:
            r = urllib.request.urlopen(urllib.request.Request(src, headers={'User-Agent': SAFARI if 'jst' in src else UA, 'Accept': '*/*'}), timeout=60)
            head = r.read(5)
            return row['url'], src, r.status, 1 if head == b'%PDF-' else 0
        except urllib.error.HTTPError as e:
            return row['url'], src, e.code, 0
        except Exception:
            continue
    return row['url'], src, 0, 0


rows = list(csv.DictReader(open(sys.argv[1]), delimiter='\t'))
print('url\tsrc\thttp\tok')
with concurrent.futures.ThreadPoolExecutor(4) as pool:
    for url, src, http, ok in pool.map(check, rows):
        print(f'{url}\t{src}\t{http}\t{ok}')
