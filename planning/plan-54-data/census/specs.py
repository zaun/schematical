"""plan-54-B/C/D: the specs table and the checks that use it (shared by scheme-check.py, pinout-check.py, final.py,
spec-rows.py, assign.py, footprint-lines.py).

specs.tsv has one row per (family, maker, series, pdf) of the candidate rows, and a reader may repeat a key on
several lines when one PDF holds more than one scheme (a part number then belongs to the line whose `scheme`
matches it; two lines of one key may not both match a part number). Columns (13, tab separated, no quotes):

  family maker series pdf rows example      the key and the skeleton (A's); never edited by a reader
  scheme     a Python regex that `re.fullmatch`es every part number the datasheet's ordering key or part-number
             table allows for this line (upper case, case sensitive); `-` when the PDF has no ordering table at
             all (then only the parts printed verbatim in the PDF, `hit` 1, count)
  label      the device's short name: `<series> <card>` (`SIM8055 Nano SIM`, `PCIe x4`)
  card       the text the sidebar shows (plan-54-B § 3.4): card sockets the standard (`SD`, `microSD`, `SIM`,
             `Micro SIM`, `Nano SIM`, `Mini SIM`, `Smart card`, `SIM / SAM`, `microSD / UFS`); module sockets
             `<generation> <style>` (`DDR4 SODIMM`); card-edge sockets `PCI Express`, `M.2`, `MicroTCA`,
             `MXM 3.0`, `Dual edge`, `Single edge`
  pinout     `seq` (pins 1..N named 1..N), `AB` (A1..A<N/2> then B1..B<N/2>, N even) or an explicit list
             `1=NAME,2=NAME,...` (pins in the datasheet's order, `SHIELD` last; the pad labels are the pin numbers
             1..N in list order (assign.py --merge renumbers them); names unique except NC, GND, VSS, SHIELD)
  positions  the position counts (DigiKey's Number of Positions) the datasheet lists for this line: `a,b,c-d,e-f/s`
             (`c-d` every integer in the range, `e-f/s` every s-th); `*` when the datasheet gives no list
  pitch      the contact pitch in mm of the drawing, `a,b` for several; `-` for a card socket (none kept)
  status     `;`-separated words: `active`; `obsolete` (the datasheet marks the part obsolete, NRND, last time buy,
             end of life: its rows are skip); `edge-mount` (a straddle-mount edge connector: H to K draw it with
             `opposite` pads and `edgeMount`); `footprint-skip:<feature>` (the land pattern needs a feature the
             footprint format lacks: the rows are skip); `pins n, digikey m` (a pin count the datasheet confirms
             against DigiKey's, § Corrections); free words after `note:` are ignored
"""
import csv
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.abspath(os.path.join(DATA, '..', '..'))
CANDIDATES = os.path.join(DATA, 'candidates.tsv')
SPECS = os.path.join(DATA, 'specs.tsv')
COLS = ['family', 'maker', 'series', 'pdf', 'rows', 'example', 'scheme', 'label', 'card', 'pinout', 'positions',
        'pitch', 'status']
FILL = COLS[6:]
KEYCOLS = COLS[:4]
FAMILIES = ('card', 'module', 'edge')
SHARED = ('NC', 'GND', 'VSS', 'SHIELD')

NOT_TABLE = "not in the datasheet's ordering table"
FACT = 'fact not in datasheet'
OBSOLETE = 'obsolete by the datasheet'


def key(r):
    return tuple(r[c] for c in KEYCOLS)


def read_rows(path):
    with open(path) as f:
        return list(csv.DictReader(f, delimiter='\t'))


def load(path=SPECS):
    """key -> [spec rows in file order]"""
    out = {}
    for r in read_rows(path):
        out.setdefault(key(r), []).append(r)
    return out


def candidates(path=CANDIDATES):
    return read_rows(path)


def write_rows(path, rows, cols):
    with open(path, 'w', newline='') as f:
        w = csv.writer(f, delimiter='\t', lineterminator='\n')
        w.writerow(cols)
        for r in rows:
            w.writerow([r.get(c, '') for c in cols])


def status_words(spec):
    return [w.strip() for w in spec['status'].split(';') if w.strip() and not w.strip().startswith('note:')]


def parse_set(text):
    """positions text -> set of ints, or None for `*`."""
    text = text.strip()
    if text == '*':
        return None
    out = set()
    for part in text.split(','):
        part = part.strip()
        m = re.fullmatch(r'(\d+)-(\d+)(?:/(\d+))?', part)
        if m:
            lo, hi, step = int(m.group(1)), int(m.group(2)), int(m.group(3) or 1)
            out.update(range(lo, hi + 1, step))
        elif part.isdigit():
            out.add(int(part))
        else:
            raise ValueError(f'bad positions item {part!r}')
    return out


def parse_pitches(text):
    text = text.strip()
    if text in ('-', '*', ''):
        return None
    return [float(x) for x in text.split(',')]


def scheme_matches(spec, mpn):
    s = spec['scheme']
    if s in ('', '-'):
        return False
    return re.fullmatch(s, mpn) is not None


def explicit_pins(pinout):
    """explicit `1=NAME,2=NAME` list -> [(pad, name)], else None."""
    if '=' not in pinout:
        return None
    out = []
    for item in pinout.split(','):
        pad, _, name = item.partition('=')
        out.append((pad.strip(), name.strip()))
    return out


def pin_names(pinout, n):
    """The pin names of a pinout rule for a part with `n` pins (n counts every pin); None when the rule cannot give n."""
    if pinout == 'seq':
        return [str(i) for i in range(1, n + 1)]
    if pinout == 'AB':
        if n % 2:
            return None
        h = n // 2
        return [f'A{i}' for i in range(1, h + 1)] + [f'B{i}' for i in range(1, h + 1)]
    e = explicit_pins(pinout)
    if e is None or len(e) != n:
        return None
    return [name for _, name in e]


def confirmed_difference(spec):
    """(n, m) of a `pins n, digikey m` status word (the datasheet confirms n pins where DigiKey says m), or None."""
    for w in status_words(spec):
        m = re.fullmatch(r'pins (\d+), digikey (\d+)', w)
        if m:
            return int(m.group(1)), int(m.group(2))
    return None


def pin_count(row, spec):
    """The pin count the pinout must have for this candidate row: DigiKey's positions, plus one SHIELD pin when
    the pinout names one and DigiKey's number (contacts + switch contacts) leaves it out; the `n` of a status word
    `pins n, digikey m` when DigiKey's number is that m (the datasheet is the authority)."""
    n = int(row['positions'] or 0)
    diff = confirmed_difference(spec)
    if diff is not None and n == diff[1]:
        return diff[0]
    e = explicit_pins(spec['pinout'])
    if e is not None and any(name == 'SHIELD' for _, name in e) and len(e) == n + 1:
        return n + 1
    return n


def filled(spec):
    return all(spec.get(c, '').strip() for c in FILL)


def verdict(row, specs_of_key):
    """(verdict, reason, detail, spec). verdict: unchecked | skip | ok."""
    if not specs_of_key:
        return 'unchecked', 'no spec row', '', None
    for s in specs_of_key:
        if not filled(s):
            return 'unchecked', 'spec row has an empty column', s['pdf'], s
    matched = [s for s in specs_of_key if scheme_matches(s, row['mpn'])]
    if len(matched) > 1:
        return 'unchecked', 'two schemes match', row['mpn'], matched[0]
    if not matched:
        if row['hit'] != '1':
            return 'skip', NOT_TABLE, specs_of_key[0]['scheme'], specs_of_key[0]
        matched = [specs_of_key[0]]
    spec = matched[0]
    words = status_words(spec)
    if 'obsolete' in words:
        return 'skip', OBSOLETE, spec['status'], spec
    for w in words:
        if w.startswith('footprint-skip:'):
            return 'skip', 'footprint not expressible: ' + w.split(':', 1)[1].strip(), '', spec
    problems = []
    pos = parse_set(spec['positions'])
    diff = confirmed_difference(spec)
    if pos is not None and diff is not None and int(row['positions'] or 0) == diff[1]:
        pos = None   # the datasheet confirms its own count for DigiKey's m (status `pins n, digikey m`)
    if pos is not None and int(row['positions'] or 0) not in pos:
        problems.append(f"positions {row['positions']} not in {spec['positions']}")
    pitches = parse_pitches(spec['pitch'])
    if pitches is not None and row['pitch_mm']:
        if not any(abs(float(row['pitch_mm']) - p) <= 0.006 for p in pitches):
            problems.append(f"pitch {row['pitch_mm']} not in {spec['pitch']}")
    if problems:
        return 'skip', FACT, '; '.join(problems), spec
    return 'ok', '', '', spec
