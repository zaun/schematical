"""plan-47-B: what scheme-check.py and pinout-check.py share.

specs.tsv columns (plan-47-A skeleton, then B's): type, maker, series, pdf,
package, rows, example, scheme, label, page, pinout, terminals, pads, then the
fact columns of plan-47-B section 3.4:

  values      the series' value list as `a|b|c` or a range `lo..hi` (inductors in
              microhenries, fuses in amperes, PTCs: the hold current in amperes);
              `-` = not checked
  tols        inductor tolerances, `|` separated, as DigiKey prints them (`±20%`)
  amps_max, isat_max   inductor currents, amperes: the row's value may not exceed them
  dcr_max     inductor DC resistance, ohms
  shield      inductor shielding: shielded|semi-shielded|unshielded, `|` separated
  speed       fuse response: fast|medium|slow, `|` separated
  vac, vdc    fuse voltage ratings the series gives, volts (the row's may not exceed)
  break_max   fuse breaking capacity, amperes
  vmax, imax  PTC maximum voltage (volts) and maximum current (amperes)

`scheme` is a regex over the part number written from the datasheet's ordering
table (full match, upper case as printed); `verbatim` (a one-part PDF: the row
must be printed in it) or `-` (no ordering table: only a verbatim hit counts).
Named groups carry the value: `uh` or `nh` (an inductor's three-character code),
`amps` (a fuse's current, as printed), `hold` (a PTC's hold in hundredths of an
ampere). A scheme `<regex>;<code>=<value>,<code>=<value>` is a value map: group
`code` is looked up (inductors in microhenries, fuses and PTCs in amperes).
"""
import csv
import glob
import gzip
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
sys.path.append(HERE)
import common  # noqa: E402

CANDIDATES = os.path.join(DATA, 'candidates.tsv')
SPECS = os.path.join(DATA, 'specs.tsv')
TYPES = ['inductor', 'fuse', 'ptc']
FACT_COLS = ['pads', 'values', 'tols', 'amps_max', 'isat_max', 'dcr_max', 'shield', 'speed', 'vac', 'vdc',
             'break_max', 'vmax', 'imax']
LABELS = {'SMD', 'Radial', 'Axial', 'Cartridge'}


def read(path):
    return list(csv.DictReader(open(path, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE))


def write(path, rows, cols):
    with open(path, 'w', encoding='utf-8') as out:
        out.write('\t'.join(cols) + '\n')
        for r in rows:
            out.write('\t'.join(str(r.get(c, '')) for c in cols) + '\n')


def in_set(row):
    return row['state'] in ('candidate', 'reserve')


def key(row):
    """The specs row of a candidates row: (type, maker, series, pdf, package)."""
    return (row['type'], row['company'], row['series'], row['pdf'], row['package'])


def load_specs(path=SPECS):
    return {(s['type'], s['maker'], s['series'], s['pdf'], s['package']): s for s in read(path)}


def params_by_mpn(raw=os.path.join(DATA, 'raw')):
    out = {}
    for p in sorted(glob.glob(os.path.join(raw, '*.jsonl.gz')) + glob.glob(os.path.join(raw, '*.jsonl'))):
        opener = gzip.open if p.endswith('.gz') else open
        with opener(p, 'rt', encoding='utf-8') as f:
            for line in f:
                d = json.loads(line)
                out.setdefault(d['mpn'], d['params'])
    return out


# ---- numbers ------------------------------------------------------------------

UNITS = {
    'a': 1.0, 'ma': 1e-3, 'ua': 1e-6, 'µa': 1e-6, 'v': 1.0, 'mv': 1e-3, 'kv': 1e3,
    'h': 1e6, 'mh': 1e3, 'uh': 1.0, 'µh': 1.0, 'nh': 1e-3, 'ph': 1e-6,          # henries: to microhenries
    'ohm': 1.0, 'ohms': 1.0, 'mohm': 1e-3, 'mohms': 1e-3, 'kohm': 1e3, 'kohms': 1e3,
}
NUMBER = re.compile(r'([0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?)\s*([A-Za-zµΩ]*)')


def parse(text):
    """A DigiKey figure ("2.2 µH", "125 mA", "72mOhm Max", "24V") as a number in
    amperes, volts, microhenries or ohms; None when there is none."""
    if text is None:
        return None
    m = NUMBER.search(text.replace('Ω', 'Ohm'))
    if not m:
        return None
    unit = m.group(2).lower()
    if unit not in UNITS:
        return float(m.group(1)) if unit == '' else None
    return float(m.group(1)) * UNITS[unit]


def close(a, b):
    return abs(a - b) <= 1e-6 * max(abs(a), abs(b), 1e-12)


def in_values(spec_values, v):
    """v in a `a|b|c` list or a `lo..hi` range; True when the column is `-` or empty."""
    if spec_values in ('', '-'):
        return True
    if '..' in spec_values:
        lo, hi = spec_values.split('..')
        return float(lo) * (1 - 1e-6) <= v <= float(hi) * (1 + 1e-6)
    return any(close(float(x), v) for x in spec_values.split('|') if x)


def inductor_code(code):
    """A three-character value code (100, 4R7, R47, 10N, 1N5) as microhenries."""
    c = code.upper()
    if 'N' in c:
        return float(c.replace('N', '.')) / 1000.0
    if 'R' in c:
        return float(c.replace('R', '.'))
    if c.isdigit() and len(c) >= 3:
        return int(c[:-1]) * (10 ** int(c[-1]))
    raise ValueError(code)


def scheme_value(scheme, mpn):
    """(matched, value or None): the full match of the scheme regex and the value its groups
    (or its value map) give. matched is False when the part number does not match."""
    valuemap = None
    if ';' in scheme:
        scheme, rest = scheme.split(';', 1)
        valuemap = dict(p.split('=', 1) for p in rest.split(',') if '=' in p)
    m = re.fullmatch(scheme, mpn)
    if not m:
        return False, None
    g = m.groupdict()
    try:
        if valuemap is not None:
            return True, float(valuemap[g['code']]) if g.get('code') in valuemap else None
        if g.get('uh'):
            return True, inductor_code(g['uh'])
        if g.get('nh'):
            return True, inductor_code(g['nh'])
        if g.get('amps'):
            return True, float(g['amps'])
        if g.get('hold'):
            return True, int(g['hold']) / 100.0
    except (ValueError, KeyError):
        return True, None
    return True, None


def row_value(r):
    """The row's DigiKey value in the scheme's unit (µH, A)."""
    return parse(r['value'])
