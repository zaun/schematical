"""plan-44 census: shared code (reading the listing, the scope rules, the datasheet URL rules).

Imported by select.py, urls.py and footprint-lines.py as
`sys.path.insert(0, os.path.dirname(os.path.abspath(__file__))); import common`
(`python3 -I` does not put the script's directory on the path).
"""
import collections
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
TREE = os.path.join(ROOT, 'datasheets', 'crystal')
PLACED = os.path.join(TREE, 'datasheets.tsv')
SLUG = {'Abracon': 'abracon', 'ECS': 'ecs', 'Suntsu': 'suntsu'}
MAKERS = {'535': 'Abracon', '50': 'ECS', '2388': 'Suntsu'}
MAX_HZ = 100_000_000
# Hosts that are the maker's own site.
MAKER_HOSTS = {'Abracon': {'abracon.com', 'www.abracon.com'},
               'ECS': {'ecsxtal.com', 'www.ecsxtal.com'},
               'Suntsu': {'suntsu.com', 'www.suntsu.com', 'suntsucompany.wpengine.com'}}
# When a row's link is a distributor copy (Waldom, DigiKey) or missing: the
# URL of another row of the same (maker, series) on the maker's site, else this.
SERIES_URL = {'Abracon': 'https://abracon.com/Resonators/{series}.pdf',
              'ECS': 'https://ecsxtal.com/store/pdf/{series}.pdf'}


def num(text):
    m = re.search(r'([\d.]+)', text or '')
    return float(m.group(1)) if m else None


def ppm(text):
    """`±20ppm` -> 20.0, `±0.5%` -> 5000.0 (a ceramic resonator's tolerance is a percentage);
    `±75kHz` and `-` -> None (an absolute frequency has no ppm without the frequency)."""
    text = text or ''
    if re.search(r'kHz', text, re.I):
        return None
    v = num(text)
    if v is not None and '%' in text:
        v = round(v * 10000, 6)
    return v


def ohms(text):
    """`70 kOhms` -> 70000.0, `60 Ohms` -> 60.0 (DigiKey spells a kHz crystal's ESR in kOhms)."""
    v = num(text)
    if v is not None and re.search(r'\bk\s*Ohm', text or '', re.I):
        v *= 1000
    return v


def hz_of(text):
    m = re.match(r'\s*([\d.]+)\s*(k|M|G)?Hz', text or '')
    if not m:
        return None
    return int(round(float(m.group(1)) * {'': 1, 'k': 1e3, 'M': 1e6, 'G': 1e9}[m.group(2) or '']))


def read_raw(raw):
    cut = {}
    for err in glob.glob(os.path.join(raw, '*.err')):
        cat, maker = os.path.basename(err)[:-4].split('-')
        for line in open(err):
            m = re.match(r'series (.*): (\d+) of (\d+)( \(cut at 300)?', line)
            if m:
                cut[(cat, maker, m.group(1))] = bool(m.group(4))
    rows = []
    for path in sorted(glob.glob(os.path.join(raw, '*.jsonl*'))):
        for line in (gzip.open(path, 'rt') if path.endswith('.gz') else open(path)):
            d = json.loads(line)
            p = d['params']
            cat = d['category']
            if cat == '174':
                cls = 'resonator'
            elif 'kHz' in p.get('Type', ''):
                cls = 'khz'
            else:
                cls = 'mhz'
            rows.append({
                'mpn': d['mpn'], 'maker': MAKERS[d['makerId']], 'class': cls, 'series': d['series'],
                'stock': d['stock'], 'status': d['status'], 'hz': hz_of(p.get('Frequency')),
                'load_pf': num(p.get('Load Capacitance', '')) if 'pF' in p.get('Load Capacitance', '') else '',
                'tol_ppm': ppm(p.get('Frequency Tolerance', '')),
                'stab_ppm': ppm(p.get('Frequency Stability', '')),
                'esr_ohm': ohms(p.get('ESR (Equivalent Series Resistance)', p.get('Impedance', ''))),
                'temp': p.get('Operating Temperature', ''), 'mounting': p.get('Mounting Type', ''),
                'package': p.get('Package / Case', ''),
                'supplier_package': p.get('Supplier Device Package', ''),
                'size': p.get('Size / Dimension', ''), 'height': p.get('Height - Seated (Max)', p.get('Height', '')),
                'rating': p.get('Ratings', '-'), 'features': p.get('Features', ''),
                'datasheet': d['datasheet'], 'host': urlparse(d['datasheet']).netloc,
                'cut': int(cut.get((cat, d['makerId'], d['series']), False)),
                '_type': p.get('Type', ''), '_mode': p.get('Operating Mode', ''),
            })
    return rows


def scope(r):
    """The scope reason a row is out for, or ''."""
    if r['status'] not in ('Active', 'Last Time Buy'):
        return 'status'
    if not r['stock'] or r['stock'] <= 0:
        return 'no-stock'
    if 'Overtone' in r['_mode']:
        return 'overtone'
    if 'Thermistor' in r['_type']:
        return 'thermistor'
    if 'SAW' in r['_type']:
        return 'saw'
    if r['class'] != 'resonator' and r['_mode'] != 'Fundamental':
        return 'mode'
    if not r['hz'] or r['hz'] > MAX_HZ:
        return 'frequency'
    return ''


def dedupe(rows):
    """Marks `duplicate` on every in-scope row that is not the most-stocked of its key."""
    seen = set()
    for r in sorted(rows, key=lambda r: (-r['stock'], r['mpn'])):
        if r['reason']:
            continue
        key = tuple(r[k] for k in ('maker', 'class', 'series', 'hz', 'load_pf', 'tol_ppm', 'stab_ppm',
                                   'esr_ohm', 'temp', 'package', 'supplier_package', 'rating', 'features'))
        if key in seen:
            r['reason'] = 'duplicate'
        else:
            seen.add(key)


def resolve_urls(rows):
    """Sets r['url'] (the maker-site URL, '' when none) and r['via'] ('direct' or 'archive')
    on every row. Rule order: the row's own link on the maker's site (a Suntsu link on
    suntsucompany.wpengine.com moves to suntsu.com); else the commonest maker-site link of
    another row of the same (maker, series); else SERIES_URL; else ''. Suntsu answers 403 to
    every automated fetch, so its URLs are fetched from the Internet Archive's copy."""
    sibling = collections.defaultdict(collections.Counter)
    for r in rows:
        if r['host'] in MAKER_HOSTS[r['maker']]:
            sibling[(r['maker'], r['series'])][fix(r['datasheet'])] += 1
    for r in rows:
        if r['host'] in MAKER_HOSTS[r['maker']]:
            url = fix(r['datasheet'])
        elif sibling[(r['maker'], r['series'])]:
            url = sibling[(r['maker'], r['series'])].most_common(1)[0][0]
        elif r['maker'] in SERIES_URL:
            url = SERIES_URL[r['maker']].format(series=r['series'])
        else:
            url = ''
        r['url'] = url
        r['via'] = 'archive' if 'suntsu.com' in url else 'direct'


def fix(url):
    url = url.replace('suntsucompany.wpengine.com', 'suntsu.com')
    return re.sub(r'^https?://www\.(abracon|suntsu)\.com', r'https://\1.com', url).replace('http://', 'https://')


# ---- plan-44-A Phase 3: the downloaded PDFs and the download set ----

def eligible(row):
    """A download-set row of candidates.tsv: candidate or reserve (`state` is not `out`)."""
    return row['state'] != 'out'


def md5(url):
    return hashlib.md5(url.encode()).hexdigest()


def pdf_path(directory, url):
    return os.path.join(directory, md5(url) + '.pdf')


def is_pdf(path):
    if not os.path.exists(path):
        return False
    with open(path, 'rb') as f:
        return f.read(5) == b'%PDF-'


def norm(s):
    """Every run of non-alphanumerics becomes one space (textscan's token rule)."""
    return ' ' + re.sub(r'[^0-9A-Z]+', ' ', (s or '').upper()).strip() + ' '


def source(row):
    """The URL a row's own datasheet is downloaded from (candidates.tsv `url`)."""
    return row['url']
