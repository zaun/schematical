"""plan-45 census: shared code (reading the listing, the scope rules, the datasheet URL rules).

Imported by select.py, urls.py and footprint-lines.py as
`sys.path.insert(0, os.path.dirname(os.path.abspath(__file__))); import common`
(`python3 -I` does not put the script's directory on the path).
"""
import collections
import glob
import gzip
import json
import os
import re
from urllib.parse import urlparse

MAKERS = {'277': 'Phoenix Contact', '281': 'Weidmuller', '732': 'Wurth Elektronik', '455': 'JST'}
SHORT = {'Phoenix Contact': 'PHX', 'Weidmuller': 'WDM', 'Wurth Elektronik': 'WE', 'JST': 'JST'}
PITCHES = {'3.50': 3.5, '3.81': 3.81, '5.00': 5.0, '5.08': 5.08, '7.50': 7.5, '7.62': 7.62}
MAX_POS = 12
# The hosts that are the maker's own site (a DigiKey link on another host is a distributor copy).
MAKER_HOSTS = {'Phoenix Contact': {'www.phoenixcontact.com', 'phoenixcontact.com', 'product-download.phoenixcontact.com'},
               'Weidmuller': {'datasheet.weidmueller.com'},
               'Wurth Elektronik': {'www.we-online.com', 'we-online.com', 'katalog.we-online.de'},
               'JST': {'www.jst-mfg.com', 'jst-mfg.com', 'www.jst.com', 'jst.com'}}


def num(text):
    m = re.search(r'([\d.]+)', text or '')
    return float(m.group(1)) if m else None


def pitch_of(text):
    m = re.search(r'\(([\d.]+)mm\)', text or '')
    return float(m.group(1)) if m else None


def wire_class(p):
    """screw / spring / push-in / pluggable (a 370 board-side header) / '' (not a wire clamp we take)."""
    if 'Header Orientation' in p:
        return 'pluggable'
    t = p.get('Wire Termination', '')
    if t.startswith('Screw -') or t == 'Screw' or t.startswith('Screw with'):
        return 'screw'
    if 'Push-In' in t:
        return 'push-in'
    if 'Spring Cage' in t or 'Push Button' in t or 'Tension' in t or t == 'Screwless':
        return 'spring'
    return ''


def orientation(p, mounting):
    """vertical / horizontal / angled (a terminal block's wire entry versus the board)."""
    o = p.get('Mating Orientation') or p.get('Header Orientation') or ''
    if 'Right Angle' in mounting:
        return 'horizontal'
    if o.startswith('Vertical') or o == 'Vertical':
        return 'vertical'
    if o.startswith('Horizontal') or o.startswith('90°'):
        return 'horizontal'
    if '°' in o:
        return 'angled'
    return 'vertical' if 'Surface' in mounting and not o else (o or '')


def read_raw(raw):
    rows = []
    for path in sorted(glob.glob(os.path.join(raw, '*.jsonl*'))):
        for line in (gzip.open(path, 'rt') if path.endswith('.gz') else open(path)):
            d = json.loads(line)
            p = d['params']
            maker = MAKERS[d['makerId']]
            jst = maker == 'JST'
            mounting = p.get('Mounting Type', '')
            rows.append({
                'mpn': d['mpn'], 'maker': maker, 'group': 'jst' if jst else 'terminal', 'category': d['category'],
                'series': d['series'], 'stock': d.get('stock') or 0, 'status': d.get('status') or '',
                'pitch': pitch_of(p.get('Pitch - Mating' if jst else 'Pitch')),
                'positions': int(num(p.get('Number of Positions' if jst else 'Positions Per Level')) or 0),
                'levels': p.get('Number of Levels', '1'),
                'class': 'jst' if jst else wire_class(p),
                'orientation': orientation(p, mounting) if not jst else ('horizontal' if 'Right Angle' in mounting else 'vertical'),
                'mounting': 'smd' if 'Surface' in mounting else ('tht' if mounting.startswith('Through Hole') else mounting),
                'mounting_raw': mounting,
                'amps': num(p.get('Current') or p.get('Current - IEC') or p.get('Current Rating (Amps)')),
                'volts': num(p.get('Voltage') or p.get('Voltage - IEC') or p.get('Voltage Rating')),
                'type': p.get('Type', ''), 'shroud': p.get('Shrouding', ''),
                'datasheet': d['datasheet'], 'host': urlparse(d['datasheet']).netloc,
                'description': d['description'],
            })
    return rows


def scope(r):
    """The scope reason a row is out for, or ''."""
    if r['status'] not in ('Active', 'Last Time Buy'):
        return 'status'
    if not r['stock'] or r['stock'] <= 0:
        return 'no-stock'
    if not 2 <= r['positions'] <= MAX_POS:
        return 'positions'
    if r['group'] == 'jst':
        return ''
    if r['levels'] != '1':
        return 'levels'
    if r['pitch'] is None or round(r['pitch'], 2) not in PITCHES.values():
        return 'pitch'
    if 'SKEDD' in r['mounting_raw'] or 'Press-Fit' in r['mounting_raw']:
        return 'mounting'
    if r['mounting'] not in ('tht', 'smd'):
        return 'mounting'
    if not r['class']:
        return 'termination'
    return ''


def dedupe(rows):
    """Marks `duplicate` on every in-scope row that is not the most-stocked of its key (colour and
    packing variants share everything the library keeps)."""
    seen = set()
    for r in sorted(rows, key=lambda r: (-r['stock'], r['mpn'])):
        if r['reason']:
            continue
        key = (r['maker'], r['series'], r['pitch'], r['positions'], r['orientation'], r['mounting'],
               r['class'], r['amps'], r['volts'], r['type'], r['shroud'])
        if key in seen:
            r['reason'] = 'duplicate'
        else:
            seen.add(key)


def variant(r):
    """The footprint family a row belongs to: one drawing per position count."""
    return (r['maker'], r['group'], r['series'], r['pitch'], r['orientation'], r['mounting'],
            r['class'] if r['group'] == 'terminal' else '', r['type'] if r['category'] == '370' else '',
            r['shroud'] if r['category'] == '370' else '')


def resolve_urls(rows):
    """Sets r['url'] (the maker-site URL, '' when none) and r['src'] (where the file is downloaded from).
    Phoenix Contact answers 403 to every automated fetch (curl, Python, with a browser User-Agent), so
    its file is DigiKey's copy (`mm.digikey.com`, the maker's own per-item PDF) and its URL the maker's
    per-item link `https://www.phoenixcontact.com/us/products/<item>/pdf`; a row with no DigiKey copy
    has no source. Weidmuller: the row's own link on datasheet.weidmueller.com. Wurth: the row's own
    link on we-online.com (downloaded with the User-Agent `Mozilla/5.0`). JST: the row's own link
    (jst-mfg.com / jst.com)."""
    for r in rows:
        link = r['datasheet']
        if link.startswith('//'):
            link = 'https:' + link
        link = link.replace('http://', 'https://')
        r['url'] = r['src'] = ''
        if r['maker'] == 'Phoenix Contact':
            if r['host'] == 'mm.digikey.com' and link.endswith('/' + r['mpn'] + '.pdf'):
                r['url'] = f"https://www.phoenixcontact.com/us/products/{r['mpn']}/pdf"
                r['src'] = link
        elif r['host'] in MAKER_HOSTS[r['maker']]:
            r['url'] = r['src'] = link
