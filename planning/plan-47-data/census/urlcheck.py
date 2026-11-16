#!/usr/bin/env python3
"""plan-47 census: which URLs of urls-needed.tsv answer with a PDF.
usage: urlcheck.py <urls-needed.tsv> <cache dir> > url-check.tsv
Each URL is fetched once with Python's urllib and a Safari user agent
(the client every one of the thirteen maker sites answers; curl is refused
by we-online.com, littelfuse.com, te.com and eaton.com) and kept as
<cache dir>/<md5 of the URL>.pdf, which plan-47-A's fetch.py reuses.
Columns: company, url, ok (1 = the body starts with %PDF-), status, bytes,
final (the URL that answered: for a Panasonic URL that fails, the same file
under the other directories of common.PANASONIC_DIRS is tried).
8 fetches at a time, one retry, 60 s each."""
import concurrent.futures
import hashlib
import os
import sys
import urllib.parse
import urllib.request

PANASONIC_DIRS = ['signal', 'power', 'photomos', 'vehicle', 'interception']

UA = ('Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 '
      '(KHTML, like Gecko) Version/17.0 Safari/605.1.15')
HEAD = {'User-Agent': UA, 'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
        'Accept-Language': 'en-US,en;q=0.9'}


def fetch(url, cache):
    ok, status, size = fetch_one(url, cache)
    if ok or '//industry.panasonic.com/ac/cdn/e/control/relay/' not in url:
        return ok, status, size, url if ok else ''
    head, tail = url.split('/control/relay/')
    name = tail.split('/catalog/')[1]
    for d in PANASONIC_DIRS:
        alt = head + '/control/relay/' + d + '/catalog/' + name
        if alt == url:
            continue
        ok, st, size = fetch_one(alt, cache)
        if ok:
            return ok, st, size, alt
    return 0, status, 0, ''


def fetch_one(url, cache):
    path = os.path.join(cache, hashlib.md5(url.encode()).hexdigest() + '.pdf')
    if os.path.exists(path) and open(path, 'rb').read(5) == b'%PDF-':
        return 1, 'cached', os.path.getsize(path)
    last = ''
    for _ in range(2):
        try:
            req = urllib.request.Request(urllib.parse.quote(url, safe=':/?&=%~@+,;()'), headers=HEAD)
            with urllib.request.urlopen(req, timeout=60) as r:
                body = r.read()
                if body[:5] == b'%PDF-':
                    with open(path, 'wb') as f:
                        f.write(body)
                    return 1, str(r.status), len(body)
                last = '%s %s' % (r.status, (r.headers.get('content-type') or '').split(';')[0])
        except Exception as e:  # noqa: BLE001 - every failure is a "no"
            last = type(e).__name__ + ' ' + str(getattr(e, 'code', ''))
    return 0, last.strip(), 0


def main():
    src, cache = sys.argv[1], sys.argv[2]
    os.makedirs(cache, exist_ok=True)
    rows = [l.rstrip('\n').split('\t') for l in open(src, encoding='utf-8')][1:]
    with concurrent.futures.ThreadPoolExecutor(8) as pool:
        res = list(pool.map(lambda r: fetch(r[1], cache), rows))
    print('company\turl\tok\tstatus\tbytes\tfinal')
    for r, (ok, status, size, final) in zip(rows, res):
        print(r[0], r[1], ok, status, size, final, sep='\t')


if __name__ == '__main__':
    main()
