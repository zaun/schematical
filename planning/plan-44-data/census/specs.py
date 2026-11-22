"""plan-44-B: what scheme-check.py, pinout-check.py and final.py share: reading
specs.tsv, a row's spec, the ordering-code scheme match, the fact checks, and
the pad count of a DigiKey Package / Case.

specs.tsv columns (one row per (maker, series, pdf, package) of the download set):

  maker series pdf package rows example     the key and the skeleton (plan-44-A)
  scheme      a Python regex that must `re.fullmatch` every part number the
              datasheet's ordering table allows; options are groups. A group
              named `hz` holds the frequency as number + unit
              (`32.768KHZ`, `8.000MHZ`, `3M`, `12345HZ`) and must equal the
              row's `hz`. A part number that spells its frequency as a code
              (`AWSCR-800CV`) names the group `hzc` and appends a map after
              `;`: `<regex>;800=8000000,455=455000` (the regex has no `;`).
              `-` = the PDF has no ordering table (only a verbatim `hit` counts).
  label       the part's "package" family: SMD, HC-49/US, HC-49/U, HC-46/X,
              SOJ, Cylinder (docs/parts-format.md forbids sizes and pin counts)
  pinout      the whole pin list in pad order, `<designator>=<name>` joined by
              commas (`1=X1,2=NC,3=X2,4=NC`); names X1, X2, NC, GND
  terminals   always `X1,X2`
  capacitors  1 when the datasheet says the part has built-in load capacitors
              (a three-pad ceramic resonator), else 0
  page        the datasheet page(s) of the ordering table / electrical table
  hz_range    the frequencies the datasheet gives for the series: `min-max`
              in Hz, comma-separated list of ranges, or `*` (not given)
  loads       load capacitances in pF: numbers, `a-b` ranges, `-` for series
              resonance, comma-separated; `*` = not given (not checked)
  tols        frequency tolerances in ppm (numbers / ranges), `*` = not given
  stabs       frequency stabilities in ppm (numbers / ranges), `*` = not given
  esr_max     the largest ESR in ohms (impedance for a resonator) the
              datasheet gives for the series, `*` = not given
"""
import csv
import os
import re

import common

SPECS = os.path.join(common.DATA, 'specs.tsv')
CANDIDATES = os.path.join(common.DATA, 'candidates.tsv')
COLS = ['maker', 'series', 'pdf', 'package', 'rows', 'example', 'scheme', 'label', 'pinout',
        'terminals', 'capacitors', 'page', 'hz_range', 'loads', 'tols', 'stabs', 'esr_max']
LABELS = ('SMD', 'HC-49/US', 'HC-49/U', 'HC-46/X', 'SOJ', 'Cylinder')
NAMES = ('X1', 'X2', 'NC', 'GND')
UNITS = {'': 1.0, 'HZ': 1.0, 'K': 1e3, 'KHZ': 1e3, 'M': 1e6, 'MHZ': 1e6}


def key(r):
    return (r['maker'], r['series'], r['pdf'], r['package'])


def load(path=SPECS):
    return {key(r): r for r in csv.DictReader(open(path), delimiter='\t')}


def parse_hz(text):
    """`32.768KHZ` -> 32768.0, `8M` -> 8e6, `12345HZ` -> 12345.0; None when not a frequency."""
    m = re.fullmatch(r'(\d+(?:\.\d+)?)(HZ|KHZ|MHZ|K|M)?', (text or '').upper())
    if not m:
        return None
    return float(m.group(1)) * UNITS[m.group(2) or '']


def split_scheme(s):
    """(regex, {code: Hz}) of a `scheme` cell."""
    if ';' not in s:
        return s, {}
    regex, _, table = s.partition(';')
    out = {}
    for item in table.split(','):
        if item.strip():
            c, _, hz = item.partition('=')
            out[c.strip()] = float(hz)
    return regex, out


def same_hz(a, b):
    return abs(a - b) <= max(1.0, abs(b) * 1e-6)


def matches(spec, row):
    """The spec's scheme full-matches the row's part number, and its frequency
    group (`hz`, or `hzc` through the map) equals the row's `hz`."""
    s = (spec or {}).get('scheme', '')
    if not s or s == '-':
        return False
    regex, table = split_scheme(s)
    m = re.fullmatch(regex, row['mpn'])
    if m is None:
        return False
    groups = m.groupdict()
    want = float(row['hz']) if row.get('hz') not in (None, '') else None
    if groups.get('hz') is not None:
        hz = parse_hz(groups['hz'])
        return hz is not None and (want is None or same_hz(hz, want))
    if groups.get('hzc') is not None:
        hz = table.get(groups['hzc'])
        return hz is not None and (want is None or same_hz(hz, want))
    return True


def in_table(row, spec):
    return row.get('hit') == '1' or matches(spec, row)


def _tokens(cell):
    return [t.strip() for t in cell.split(',') if t.strip()]


def _in_list(value, cell, series_ok=False):
    """value (float or None for 'series') in a list cell of numbers, a-b ranges and `-`."""
    if cell in ('', '*'):
        return True
    for t in _tokens(cell):
        if t == '-':
            if value is None:
                return True
            continue
        if value is None:
            continue
        m = re.fullmatch(r'([\d.]+)-([\d.]+)', t)
        if m:
            if float(m.group(1)) - 1e-9 <= value <= float(m.group(2)) + 1e-9:
                return True
        elif re.fullmatch(r'[\d.]+', t) and abs(float(t) - value) < 1e-6:
            return True
    return False


def fact_problems(row, spec):
    """Facts of the row (DigiKey's) the datasheet's tables do not give."""
    out = []
    if spec is None:
        return out
    if row['hz'] not in ('', None) and spec['hz_range'] not in ('', '*'):
        hz = float(row['hz'])
        ok = False
        for t in _tokens(spec['hz_range']):
            lo, _, hi = t.partition('-')
            if float(lo) * (1 - 1e-9) - 1 <= hz <= float(hi) * (1 + 1e-9) + 1:
                ok = True
        if not ok:
            out.append(f"hz {row['hz']} outside {spec['hz_range']}")
    load = float(row['load_pf']) if row['load_pf'] not in ('', None) else None
    if spec['loads'] not in ('', '*') and not _in_list(load, spec['loads']):
        out.append(f"load {row['load_pf'] or 'series'} not in {spec['loads']}")
    for col, spec_col in (('tol_ppm', 'tols'), ('stab_ppm', 'stabs')):
        if row[col] not in ('', None) and not _in_list(float(row[col]), spec[spec_col]):
            out.append(f"{col} {row[col]} not in {spec[spec_col]}")
    if row['esr_ohm'] not in ('', None) and spec['esr_max'] not in ('', '*'):
        if float(row['esr_ohm']) > float(spec['esr_max']) * (1 + 1e-9):
            out.append(f"esr {row['esr_ohm']} above {spec['esr_max']}")
    return out


def pad_counts(package):
    """The pad counts a DigiKey Package / Case names, as a set (empty: not named).
    `2-SMD, No Lead` 2; `4-SOJ` 4; `HC-49/US - 3 Lead` 3; `4-SMD, Flat Lead, 3 Leads`
    4 or 3; `HC-49/U`, `HC-49/US`, `HC-46/X`, `Cylindrical Can, Radial` 2 leads."""
    out = set()
    for part in [p.strip() for p in package.split(',')]:
        m = re.match(r'^(\d)-', part)
        if m:
            out.add(int(m.group(1)))
        m = re.search(r'(\d) Leads?\b', part)
        if m:
            out.add(int(m.group(1)))
    if not out and re.match(r'(HC-49|HC-46|Cylindrical)', package):
        out.add(2)
    return out


def pinout_items(pinout):
    """[(designator, name)] of a pinout string."""
    out = []
    for item in pinout.split(','):
        item = item.strip()
        if '=' not in item:
            raise ValueError(item)
        d, _, n = item.partition('=')
        out.append((d.strip(), n.strip()))
    return out
