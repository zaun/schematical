"""plan-51 census: reading the listings, the scope rules, the tally.

Two listing formats are read:
  raw/<maker>.jsonl[.gz]            census/list.js: one flat JSON line per part
  raw/digikey-json/<maker>.jsonl.gz `node scripts/digikey.js --json`: one DigiKey
                                    response (Products[]) per line; planning-time only
"""
import gzip, json, os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
RAW = os.path.join(HERE, '..', 'raw')

MAKERS = {54: 'Mill-Max', 1212: 'Preci-Dip', 123: 'Assmann', 43: 'Aries', 19: '3M',
          17: 'TE AMP', 612: 'Samtec', 2057: 'Adam Tech', 609: 'Amphenol ICC'}

# The shapes of the chips in the library (plan-51-A § 2): (pins) per row spacing.
DIP_SHAPES = {'0.3': {8, 14, 16, 20, 24, 28}, '0.6': {24, 28, 32, 40, 48}}
PLCC_SHAPES = {28, 32, 44, 68}


def opener(path):
    return gzip.open(path, 'rt') if path.endswith('.gz') else open(path)


def rows(maker):
    """Every part of one maker as a flat dict."""
    for name in (f'{maker}.jsonl', f'{maker}.jsonl.gz'):
        path = os.path.join(RAW, name)
        if os.path.exists(path):
            with opener(path) as f:
                for line in f:
                    if line.strip():
                        yield json.loads(line)
            return
    path = os.path.join(RAW, 'digikey-json', f'{maker}.jsonl.gz')
    with opener(path) as f:
        for line in f:
            if not line.strip():
                continue
            for p in json.loads(line)['Products']:
                yield {'mpn': p['ManufacturerProductNumber'], 'maker': p['Manufacturer']['Name'],
                       'makerId': maker, 'series': (p.get('Series') or {}).get('Name', ''),
                       'stock': p['QuantityAvailable'], 'status': p['ProductStatus']['Status'],
                       'datasheet': p.get('DatasheetUrl') or '',
                       'description': (p.get('Description') or {}).get('ProductDescription', ''),
                       'params': {x['ParameterText']: x['ValueText'] for x in p['Parameters']}}


def scope(r):
    """('DIP'|'PLCC', pins, spacing, mounting) when the part is a candidate, else None."""
    p = r['params']
    t, mounting = p.get('Type', ''), p.get('Mounting Type', '')
    if r['status'] not in ('Active', 'Last Time Buy') or r['stock'] <= 0:
        return None
    if mounting not in ('Through Hole', 'Surface Mount'):
        return None
    pins = p.get('Number of Positions or Pins (Grid)', '')
    if t.startswith('DIP') and 'ZIF' not in t:
        m = re.fullmatch(r'(\d+) \(2 x (\d+)\)', pins)
        if not m or p.get('Pitch - Mating') != '0.100" (2.54mm)':
            return None
        n = int(m.group(1))
        for spacing, shapes in DIP_SHAPES.items():
            if f'{spacing}"' in t and n in shapes:
                return ('DIP', n, spacing, mounting)
    if t == 'PLCC':
        m = re.match(r'(\d+) ', pins)
        if m and int(m.group(1)) in PLCC_SHAPES:
            return ('PLCC', int(m.group(1)), '', mounting)
    return None
