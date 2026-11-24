"""plan-55 census: shared code (reading the listing, the company of a DigiKey maker label, the datasheet source).

Imported by the other scripts as
`sys.path.insert(0, os.path.dirname(os.path.abspath(__file__))); import common`
(`python3 -I` does not put the script's directory on the path).
"""
import csv
import glob
import gzip
import hashlib
import json
import os
import re
from urllib.parse import urlparse

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
TREE = os.path.join(ROOT, 'datasheets', 'powerswitch')
PLACED = os.path.join(TREE, 'datasheets.tsv')

# DigiKey maker id -> the company (a company's labels count together, plan-55-A § Decisions).
COMPANY = {
    '296': 'ti', '14': 'ti',
    '505': 'analog-devices', '175': 'analog-devices',
    '1589': 'monolithic-power-systems',
    '488': 'onsemi', '600039': 'onsemi',
    '150': 'microchip', '576': 'microchip',
    '31': 'diodes',
}
# The three makers of each family (plan-55-A § 2).
MAKERS = {'ideal': {'ti', 'analog-devices', 'monolithic-power-systems'},
          'load': {'ti', 'onsemi', 'microchip'},
          'limited': {'ti', 'microchip', 'diodes'}}
FAMILIES = ('ideal', 'load', 'limited')
COMPANIES = ('ti', 'analog-devices', 'monolithic-power-systems', 'onsemi', 'microchip', 'diodes')

# Hosts that are the maker's own site, per company.
MAKER_HOSTS = {
    'ti': {'www.ti.com', 'ti.com'},
    'analog-devices': {'www.analog.com', 'analog.com', 'www.maximintegrated.com', 'datasheets.maximintegrated.com',
                       'pdfserv.maximintegrated.com'},
    'monolithic-power-systems': {'www.monolithicpower.com', 'monolithicpower.com'},
    'onsemi': {'www.onsemi.com', 'onsemi.com', 'www.fairchildsemi.com', 'fairchildsemi.com'},
    'microchip': {'ww1.microchip.com', 'www.microchip.com', 'microchip.com'},
    'diodes': {'www.diodes.com', 'diodes.com'},
}


def md5(s):
    return hashlib.md5(s.encode()).hexdigest()


def norm_url(u):
    if not u:
        return ''
    return 'https:' + u if u.startswith('//') else u


def read_listing(raw):
    """Every listed record in file order (the piece files sorted by name); a record whose (family, mpn) is
    already listed (under another maker label) is marked `dup`."""
    rows = []
    seen = set()
    for f in sorted(glob.glob(os.path.join(raw, '*.jsonl*'))):
        op = gzip.open if f.endswith('.gz') else open
        with op(f, 'rt') as fh:
            for line in fh:
                r = json.loads(line)
                key = (r['family'], r['mpn'])
                r['dup'] = key in seen
                seen.add(key)
                rows.append(r)
    return rows


def base_name(mpn):
    """The part number up to the first '/' (TI writes `LM3880QDRVRQ1/NOPB`)."""
    return mpn.split('/')[0]


def ti_names(mpn, link):
    """The ti.com document names to try for a TI part (plan-55-B § 3.3 rule 1): the DigiKey link's `gotoUrl` name
    first, then the lower-cased base name, then its leading letters-and-digits run."""
    names = []
    m = re.search(r'gotoUrl=[^&]*lit%2Fgpn%2F([A-Za-z0-9_.-]+)', link or '')
    if m:
        names.append(m.group(1).lower())
    base = base_name(mpn).lower()
    names.append(base)
    run = re.match(r'[a-z]+[0-9]+[a-z]?', base)
    if run:
        names.append(run.group())
    out = []
    for n in names:
        if n not in out:
            out.append(n)
    return out


def alternatives(company, mpn, link):
    """The maker URLs to try for a part, best first (plan-55-B § 3.3): a link on the maker's own host as given
    (a TI `suppproductinfo` link by the document name in its `gotoUrl`); for TI a Rochester / DigiKey link or
    none gives the part's own names; a Microchip row with no link gives `DeviceDoc/<base>.pdf`. Anything else
    gives none (rule 4: a same-company PDF that prints the part number, else the row is out)."""
    host = urlparse(link).netloc if link else ''
    if company == 'ti':
        if host in MAKER_HOSTS['ti'] and 'lit/ds/symlink' in link:
            urls = [link.split('?')[0]]
        else:
            urls = []
        urls += [f'https://www.ti.com/lit/ds/symlink/{n}.pdf' for n in ti_names(mpn, link)]
        out = []
        for u in urls:
            if u not in out:
                out.append(u)
        return out
    if company == 'microchip' and not link:
        base = re.sub(r'[^A-Za-z0-9].*$', '', base_name(mpn))
        return [f'https://ww1.microchip.com/downloads/en/DeviceDoc/{base}.pdf'] if base else []
    if host in MAKER_HOSTS.get(company, ()):
        return [link]
    return []


def read_tsv(path):
    return list(csv.DictReader(open(path), delimiter='\t'))


def write_tsv(path, cols, rows):
    with open(path, 'w', newline='') as fh:
        w = csv.DictWriter(fh, cols, delimiter='\t', lineterminator='\n', extrasaction='ignore')
        w.writeheader()
        w.writerows(rows)


def eligible(r):
    return r['state'] == 'candidate'


def pdf_path(directory, url):
    return os.path.join(directory, md5(url) + '.pdf')


def is_pdf(path):
    try:
        with open(path, 'rb') as fh:
            return fh.read(5) == b'%PDF-'
    except OSError:
        return False
