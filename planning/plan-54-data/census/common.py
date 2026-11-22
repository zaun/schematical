"""plan-54 census: shared code (reading the listing, scope, duplicates, datasheet source, variants)."""
import collections
import glob
import gzip
import json
import os
import re
from urllib.parse import urlparse

# DigiKey category -> family. 414 Memory Connectors - PC Card Sockets, 413 Inline Module Sockets,
# 303 Card Edge Connectors - Edgeboard Connectors.
FAMILY = {'414': 'card', '413': 'module', '303': 'edge'}
# DigiKey maker label -> the company (a company's labels count together).
MAKER = {
    'Same Sky (Formerly CUI Devices)': 'Same Sky', 'ATTEND Technology': 'ATTEND Technology',
    'Amphenol ICC (FCI)': 'Amphenol ICC', 'Amphenol ICC (Commercial Products)': 'Amphenol ICC',
    'TE Connectivity AMP Connectors': 'TE Connectivity', 'TE Connectivity': 'TE Connectivity',
    'Samtec Inc.': 'Samtec', 'Molex': 'Molex', 'GCT': 'GCT', '3M': '3M',
}
# The three makers of each family (plan-54-A § 2).
MAKERS = {'card': {'Same Sky', 'Amphenol ICC', 'GCT'}, 'module': {'TE Connectivity', 'Amphenol ICC', 'Molex'},
          'edge': {'Samtec', 'TE Connectivity', 'Amphenol ICC'}}
SAFARI = 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.0 Safari/605.1.15'


def norm_url(u):
    if not u:
        return ''
    return 'https:' + u if u.startswith('//') else u


def read_listing(raw):
    """Every listed part once per (category, mpn), the Parameters flattened into `params`."""
    rows = collections.OrderedDict()
    for f in sorted(glob.glob(os.path.join(raw, '*.jsonl*'))):
        op = gzip.open if f.endswith('.gz') else open
        with op(f, 'rt') as fh:
            for line in fh:
                r = json.loads(line)
                rows.setdefault((r['category'], r['mpn']), r)
    return list(rows.values())


def first_int(s):
    m = re.search(r'\d+', s or '')
    return int(m.group()) if m else 0


def pitch_mm(s):
    m = re.search(r'\(([\d.]+)mm\)', s or '')
    return m.group(1) if m else ''


def src_for(maker, url):
    """(maker URL to name in datasheets.json, URL to download from, reason when none)."""
    if not url:
        return '', '', 'no link'
    host = urlparse(url).netloc
    base = os.path.basename(urlparse(url).path)
    if maker == 'Same Sky' and host in ('www.cuidevices.com', 'www.sameskydevices.com'):
        return url, url, ''
    if maker == 'ATTEND Technology' and host == 'www.attend.com.tw':
        return url, url, ''
    if maker == 'Samtec':
        if host == 'suddendocs.samtec.com':
            u = url.split('?')[0]
            return u, u, ''
        if host == 'mm.digikey.com' and base.endswith('-mkt.pdf'):
            u = 'https://suddendocs.samtec.com/prints/' + base
            return u, u, ''
        return '', '', 'no maker url'
    if maker == 'Amphenol ICC':
        if host in ('cdn.amphenol-cs.com', 'cdn.amphenol-icc.com', 'www.amphenol-icc.com', 'www.amphenol-cs.com'):
            return url, 'https://s3.us-west-2.amazonaws.com/catsy.925/' + base, ''
        return '', '', 'no maker url'
    if maker == 'TE Connectivity':
        if host in ('www.te.com', 'www-qa.te.com'):
            return url, url, ''
        return '', '', 'no maker url'
    if maker == 'GCT' and host in ('gct.co', 'www.gct.co'):
        return url, url, ''
    if maker == '3M' and host == 'multimedia.3m.com':
        return url, url, ''
    if maker == 'Molex':
        if host in ('www.molex.com', 'www.literature.molex.com'):
            return url, url, ''
        return '', '', 'no maker url'
    return '', '', 'no maker url'
