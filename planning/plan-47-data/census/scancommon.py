"""plan-47-A Phase 3: what textscan.py, sibling.py, place.py and specs-skeleton.py share.

The download set is every `candidate` and `reserve` row of candidates.tsv
(column `state`). A PDF is cached as <pdf dir>/<md5 of its URL>.pdf (fetch.py)
and its text as <md5>.txt next to it.
"""
import csv
import hashlib
import json
import os
import re
import subprocess

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
FAMILY = {'inductor': 'inductor', 'fuse': 'fuse', 'ptc': 'fuse', 'relay': 'relay', 'buzzer': 'buzzer'}
FAMILIES = ['inductor', 'fuse', 'relay', 'buzzer']
DEAD = re.compile('OBSOLETE|DISCONTINU|NOT RECOMMENDED FOR NEW|END OF LIFE|NRND')
CLIP_MAKERS = {'Littelfuse Inc.': 'littelfuse', 'Eaton - Electronics Division': 'eaton', 'SCHURTER Inc.': 'schurter'}
CLIPS = ['01110501Z', '01020071Z', 'BK/1A5018-10-R', 'BK/1A1907-06-R', '0751.0052', '0751.0056']


def read(tsv):
    return list(csv.DictReader(open(tsv, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE))


def write(tsv, rows, cols=None):
    cols = cols or list(rows[0].keys())
    with open(tsv, 'w', encoding='utf-8') as out:
        out.write('\t'.join(cols) + '\n')
        for r in rows:
            out.write('\t'.join(str(r[c]) for c in cols) + '\n')


def in_set(row):
    return row['state'] in ('candidate', 'reserve')


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
    """Every run of non-alphanumerics becomes one space (the token rule)."""
    return ' ' + re.sub(r'[^0-9A-Z]+', ' ', (s or '').upper()).strip() + ' '


def text_of(directory, url):
    pdf = pdf_path(directory, url)
    if not is_pdf(pdf):
        return None
    txt = pdf[:-4] + '.txt'
    if not os.path.exists(txt):
        subprocess.run(['pdftotext', '-q', '-layout', pdf, txt], check=False, timeout=300)
    if not os.path.exists(txt):
        return None
    return open(txt, errors='replace').read()


def scan(body, mpn):
    """(hit, dead) of a part number in a normalized text."""
    key = norm(mpn)
    at = body.find(key) if key.strip() else -1
    if at < 0:
        return '0', '0'
    return '1', '1' if DEAD.search(body[max(0, at - 300):at + 300]) else '0'


# plan-47-A section 6: (maker slug, the clip's datasheet on the maker's site)
_EATON = ('https://www.eaton.com/content/dam/eaton/products/electronic-components/resources/data-sheet/'
          'eaton-1axxxx-pcb-fuse-clips-atc-atm-blade-data-sheet.pdf')
CLIP_URLS = {
    '01110501Z': ('littelfuse', 'https://www.littelfuse.com/assetdocs/littelfuse-fuse-clip-111-datasheet?assetguid=B1CA8750-5406-48D2-B921-B86728E90ADF'),
    '01020071Z': ('littelfuse', 'https://www.littelfuse.com/assetdocs/102-bt-series-datasheet?assetguid=896A24D8-F3D9-44D3-91AA-0A751A3C9FC6'),
    'BK/1A5018-10-R': ('eaton', _EATON),
    'BK/1A1907-06-R': ('eaton', _EATON),
    '0751.0052': ('schurter', 'https://www.schurter.com/en/datasheet/typ_OG__Clip__5x20.pdf'),
    '0751.0056': ('schurter', 'https://www.schurter.com/en/datasheet/typ_OG__Clip__5x20___6.3x32.pdf'),
}


def clips(path=None):
    """[(slug, url, mpn)] of the six fuse clips of plan-47-A section 6 (the
    maker-site URLs of the table, not DigiKey's link in clips.jsonl)."""
    return [(slug, url, mpn) for mpn, (slug, url) in CLIP_URLS.items()]
