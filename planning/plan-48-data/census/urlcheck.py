#!/usr/bin/env python3
"""plan-48 census: the distinct maker-site datasheet URLs of the in-scope
rows, and which of them answer with a PDF.
usage: urlcheck.py <raw dir> <cache dir> > url-check.tsv
Each URL is fetched once with Python's urllib and a Safari user agent and
kept as <cache dir>/<md5 of the URL>.pdf (plan-48-A's fetch reuses it).
A guessed ti.com/lit/gpn/<base> that fails is tried again as <base>-n (the
former National Semiconductor parts).
Columns: company, via, url, ok (1 = the body starts with %PDF-), status,
bytes, final (the URL that answered, without its query), rows.
8 fetches at a time, one retry, 60 s each."""
import collections
import concurrent.futures
import hashlib
import os
import sys
import urllib.parse
import urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

UA = ('Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 '
      '(KHTML, like Gecko) Version/17.0 Safari/605.1.15')
HEAD = {'User-Agent': UA, 'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
        'Accept-Language': 'en-US,en;q=0.9'}


def fetch_one(url, cache, key):
    path = os.path.join(cache, hashlib.md5(key.encode()).hexdigest() + '.pdf')
    last = ''
    for _ in range(2):
        try:
            req = urllib.request.Request(urllib.parse.quote(url, safe=':/?&=%~@+,;()'), headers=HEAD)
            with urllib.request.urlopen(req, timeout=60) as r:
                body = r.read()
                if body[:5] == b'%PDF-':
                    with open(path, 'wb') as f:
                        f.write(body)
                    return 1, str(r.status), len(body), r.geturl().split('?')[0]
                last = '%s %s' % (r.status, (r.headers.get('content-type') or '').split(';')[0])
        except Exception as e:  # noqa: BLE001 - every failure is a "no"
            last = type(e).__name__ + ' ' + str(getattr(e, 'code', ''))
    return 0, last.strip(), 0, ''


def fetch(job, cache):
    (company, via, url), rows = job
    res = fetch_one(url, cache, url)
    if not res[0] and via == 'guess' and '/lit/gpn/' in url:
        res = fetch_one(url + '-n', cache, url)
    return company, via, url, res, rows


def main():
    raw, cache = sys.argv[1], sys.argv[2]
    os.makedirs(cache, exist_ok=True)
    jobs = collections.Counter()
    for r in common.read_raw(raw):
        if common.classify(r):
            continue
        via, url = common.resolve(r)
        if url:
            jobs[(r['company'], via, url)] += 1
    print('company\tvia\turl\tok\tstatus\tbytes\tfinal\trows')
    with concurrent.futures.ThreadPoolExecutor(8) as ex:
        for company, via, url, (ok, status, size, final), rows in sorted(ex.map(lambda j: fetch(j, cache), jobs.items())):
            print('\t'.join(map(str, (company, via, url, ok, status, size, final, rows))))


if __name__ == '__main__':
    main()
