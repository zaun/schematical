#!/usr/bin/env python3
"""Compare the part library with KiCad's stock symbol and footprint libraries.

usage: python3 scripts/kicad-compare.py [--kicad DIR] [--out FILE]

Reads every kind file in schematical/schematical/Parts/ (and footprints.json),
parses KiCad's symbols/*.kicad_sym and footprints/*.pretty, and writes a
Markdown report (default planning/kicad-compare.md): for each part that has a
KiCad symbol of the same part number, whether its pins and its footprint
match KiCad's. The report explains the rules; they are summarised here:

  * Symbol match: the KiCad symbol's name or Value is the front of the part
    number once the maker's prefix is dropped, no digit following (SN74HC00N
    -> 74HC00); the longest wins; 54xx military parts use the 74xx symbol.
  * Pins: compared by pin number (name tokens, direction, power/ground).
  * Footprint: every KiCad footprint with the same pad numbers, pad centres
    compared after centring (4 rotations, mirror); Yes <= 0.10 mm, Close <=
    0.50 mm.

--kicad is KiCad's SharedSupport directory (the one holding symbols/ and
footprints/). Default: ~/Applications/KiCad/KiCad.app, then
/Applications/KiCad/KiCad.app. Takes about a minute; needs only Python 3.

No file in the repo other than --out is written.
"""

import argparse
import collections
import fnmatch
import glob
import json
import math
import os
import plistlib
import re
import sys

REPO = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
PARTS_DIR = os.path.join(REPO, 'schematical', 'schematical', 'Parts')

YES, CLOSE = 0.10, 0.50   # pad-centre offset limits, mm


# ---------------------------------------------------------------- KiCad ----

TOKEN = re.compile(r'\(|\)|"(?:[^"\\]|\\.)*"|[^\s()"]+')


def sexp(text):
    """Parse KiCad's S-expressions into nested lists of strings."""
    stack = [[]]
    for m in TOKEN.finditer(text):
        t = m.group()
        if t == '(':
            stack.append([])
        elif t == ')':
            done = stack.pop()
            stack[-1].append(done)
        elif t[0] == '"':
            stack[-1].append(t[1:-1].replace('\\"', '"').replace('\\\\', '\\'))
        else:
            stack[-1].append(t)
    return stack[0]


def symbol_info(s):
    """(name, {ext, props, pins}) of one (symbol ...) form."""
    props, pins, ext = {}, [], None

    def walk(node):
        nonlocal ext
        for c in node:
            if not (isinstance(c, list) and c):
                continue
            if c[0] == 'extends':
                ext = c[1]
            elif c[0] == 'property':
                props[c[1]] = c[2]
            elif c[0] == 'pin':
                num = name = None
                for d in c[2:]:
                    if isinstance(d, list) and d:
                        if d[0] == 'name':
                            name = d[1]
                        elif d[0] == 'number':
                            num = d[1]
                pins.append([num, name, c[1]])
            elif c[0] == 'symbol':
                walk(c)
    walk(s[2:])
    keep = ('Value', 'Footprint', 'Datasheet', 'Description', 'ki_keywords', 'ki_fp_filters')
    return s[1], {'ext': ext, 'props': {k: props[k] for k in keep if k in props}, 'pins': pins}


def load_symbols(root):
    """{'Lib:Name': resolved {props, pins{number: (name, type)}}}; derived
    symbols (extends) inherit the pins and any empty property of their parent."""
    raw = {}
    for f in sorted(glob.glob(os.path.join(root, 'symbols', '*.kicad_sym'))):
        lib = os.path.basename(f)[:-len('.kicad_sym')]
        with open(f, encoding='utf-8') as fh:
            tree = sexp(fh.read())
        for s in tree[0][1:]:
            if isinstance(s, list) and s and s[0] == 'symbol':
                name, info = symbol_info(s)
                raw[lib + ':' + name] = info

    def resolve(key):
        s = raw[key]
        props, pins = dict(s['props']), s['pins']
        if s['ext'] and key.split(':')[0] + ':' + s['ext'] in raw:
            pp, ppins = resolve(key.split(':')[0] + ':' + s['ext'])
            for k, v in pp.items():
                if props.get(k, '') == '':
                    props[k] = v
            if not pins:
                pins = ppins
        return props, pins

    out = {}
    for key in raw:
        props, pins = resolve(key)
        seen = {}
        for num, name, typ in pins:     # multi-unit symbols repeat pins
            if num not in seen or (not seen[num][0] and name):
                seen[num] = (name, typ)
        out[key] = {'props': props, 'pins': seen}
    return out


PAD = re.compile(r'\(pad\s+("(?:[^"\\]|\\.)*"|\S+)\s+(\S+)\s+(\S+)\s*\(at\s+(-?[\d.]+)\s+(-?[\d.]+)'
                 r'(?:\s+(-?[\d.]+))?\)\s*(?:\(unlocked[^)]*\)\s*)?(?:\(locked[^)]*\)\s*)?'
                 r'\(size\s+(-?[\d.]+)\s+(-?[\d.]+)\)')
NUMBERED = re.compile(r'^\d+$')
BALL = re.compile(r'^[A-HJ-NPR-UVWY]{1,2}\d{1,2}$')


def load_footprints(root):
    """{'Lib:Name': {designator: [(x, y, w, h)]}} for every .kicad_mod."""
    out = {}
    for d in sorted(glob.glob(os.path.join(root, 'footprints', '*.pretty'))):
        lib = os.path.basename(d)[:-len('.pretty')]
        for f in glob.glob(os.path.join(d, '*.kicad_mod')):
            with open(f, encoding='utf-8') as fh:
                text = fh.read()
            pads = {}
            for m in PAD.finditer(text):
                pads.setdefault(m.group(1).strip('"'), []).append(
                    (float(m.group(4)), float(m.group(5)), float(m.group(7)), float(m.group(8))))
            out[lib + ':' + os.path.basename(f)[:-len('.kicad_mod')]] = pads
    return out


# ---------------------------------------------------------------- ours -----

ROWS = list('ABCDEFGHJKLMNPRTUVWY')


def expand(fp):
    """A footprints.json entry as {designator: [(x, y, w, h)]} (docs/parts-format.md § Footprints)."""
    st, n = fp['style'], fp['pins']
    pads = {}

    def add(d, x, y, w, h):
        pads.setdefault(str(d), []).append((x, y, w, h))
    if st in ('dual', 'quad'):
        slots = fp.get('slots') or list(range(1, n + 1))
        pw, ph = fp['pad']['width'], fp['pad']['height']
        if st == 'dual':
            per, pitch, span = fp.get('perSide', n // 2), fp['pitch'], fp['span']
            pos = {}
            for s in range(1, 2 * per + 1):
                if s <= per:
                    pos[s] = (-span / 2, (s - 1 - (per - 1) / 2) * pitch, pw, ph)
                else:
                    pos[s] = (span / 2, ((2 * per - s) - (per - 1) / 2) * pitch, pw, ph)
        else:
            left, bottom, right, top = fp['sides']
            pitch, sx, sy = fp['pitch'], fp['spanX'], fp['spanY']
            pos, s = {}, 1
            for k in range(left):
                pos[s] = (-sx / 2, (k - (left - 1) / 2) * pitch, pw, ph); s += 1
            for k in range(bottom):
                pos[s] = ((k - (bottom - 1) / 2) * pitch, sy / 2, ph, pw); s += 1
            for k in range(right):
                pos[s] = (sx / 2, ((right - 1) / 2 - k) * pitch, pw, ph); s += 1
            for k in range(top):
                pos[s] = (((top - 1) / 2 - k) * pitch, -sy / 2, ph, pw); s += 1
        for i, sl in enumerate(slots):
            if sl:
                add(i + 1, *pos[sl])
        if 'centre' in fp:
            c = fp['centre']
            add((slots.index(0) + 1) if 0 in slots else 'NULL', 0, 0, c['width'], c['height'])
    elif st == 'grid':
        rows, cols, pitch = fp['rows'], fp['columns'], fp['pitch']
        for b in fp['balls']:
            r, c = ROWS.index(b[0]), int(b[1:])
            add(b, (c - 1 - (cols - 1) / 2) * pitch, (r - (rows - 1) / 2) * pitch,
                fp['pad']['width'], fp['pad']['height'])
    elif st == 'header':
        rows = fp.get('rows', 1)
        per = n // rows
        for p in range(1, n + 1):
            c, k = (p - 1) // rows, (p - 1) % rows
            add(p, (c - (per - 1) / 2) * fp['pitch'], fp['firstRowY'] + k * fp.get('rowPitch', 0),
                fp['pad']['width'], fp['pad']['height'])
    elif st == 'explicit':
        for p in fp['pads']:
            d = p.get('ball', p.get('pin'))
            add('NULL' if d == 0 else d, p['x'], p['y'], p['width'], p['height'])
    return pads


def load_ours():
    footprints = {f['id']: f for f in json.load(open(os.path.join(PARTS_DIR, 'footprints.json')))['footprints']}
    parts = []
    for f in sorted(glob.glob(os.path.join(PARTS_DIR, '*.json'))):
        kind = os.path.basename(f)[:-len('.json')]
        if kind in ('footprints', 'datasheets', 'generic'):
            continue
        for dev in json.load(open(f))['devices']:
            for p in dev.get('parts', []):
                parts.append(dict(kind=kind, name=p['name'], fp=p.get('footprint'),
                                  pins=dev['pinouts'][p['pinout']], flags=p.get('flags', [])))
    return footprints, parts


# ------------------------------------------------------ symbol matching ----

def norm(s):
    return re.sub(r'[^A-Z0-9]', '', s.upper())


def build_symbol_keys(symbols):
    keys = collections.defaultdict(list)    # normalised name -> symbol keys
    for k, s in symbols.items():
        for cand in {k.split(':', 1)[1], s['props'].get('Value', '')}:
            c = norm(cand)
            if (len(c) >= 4 and not c.isdigit()) or (len(c) == 4 and c.isdigit()):
                keys[c].append(k)
    return keys


def find_symbols(keys, name, military=False):
    """(symbol keys, used the 74 symbol for a 54 part) for a part number."""
    N0 = norm(name)
    cands = [N0]
    if military:
        m = re.sub(r'^(SNJ|SNV|SN|CD|M|MC|HD|DM|MM|SNM)?54', '74', N0, count=1)
        if m != N0:
            cands.append(m)
        m2 = N0.replace('54', '74', 1)
        if m2 not in cands:
            cands.append(m2)
    best = None
    for N in cands:
        for L in range(min(len(N), 30), 3, -1):
            if best and L < best[0]:
                break
            for i in range(0, min(5, len(N) - L + 1)):
                c = N[i:i + L]
                if c in keys:
                    rest = N[i + L:]
                    if rest[:1].isdigit():
                        continue
                    if i > 0 and not N[:i].isalpha() and not re.fullmatch(r'[A-Z]+5?4?', N[:i]):
                        continue
                    best = (L, c, N != N0)
                    break
            if best and best[0] == L:
                break
    if not best:
        return [], False
    return keys[best[1]], best[2]


# ----------------------------------------------------------------- pins ----

KT = {'input': 'input', 'output': 'output', 'tri_state': 'output', 'open_collector': 'output',
      'open_emitter': 'output', 'bidirectional': 'inout', 'passive': 'inout', 'power_in': 'power',
      'power_out': 'power', 'no_connect': 'nc', 'unconnected': 'nc', 'free': 'inout', 'unspecified': 'inout'}


def canon(p):
    if p in ('VDD', 'VCC', 'V+'):
        return 'VCC'
    m = re.match(r'^(VDD|VCC)(\d*)([A-Z]?)$', p)
    if m:
        return 'VCC' + m.group(2) + m.group(3)
    m = re.match(r'^(VSS|GND|AGND|DGND|PGND|GROUND|0V|V-|VEE)(\d*)([A-Z]?)$', p)
    if m and p != 'VEE':
        return 'GND' + m.group(2)
    if p in ('NC', 'DNC', 'N', 'NOCONNECT', 'NOCONN') or re.match(r'^NC\d*$', p):
        return 'NC'
    return p


def toks(name):
    """The comparable spellings of a pin name (None: unnamed)."""
    if name in (None, '', '~'):
        return None
    t = re.sub(r'[~{}\\_]', '', name).upper()
    out = set()
    for p in re.split(r'[^A-Z0-9+\-]+', t):
        p = p.strip('-')
        if not p:
            continue
        nobar = re.sub(r'BAR$', '', p)
        for x in {p, nobar, re.sub(r'^\d+(?=[A-Z])', '', p), re.sub(r'^\d+(?=[A-Z])', '', nobar)}:
            if x:
                out.add(canon(x))
    out.add(canon(re.sub(r'[^A-Z0-9+]', '', t)))
    return out or None


def pclass(t):
    if not t:
        return None
    if any(x.startswith('GND') for x in t):
        return 'gnd'
    if any(x.startswith('VCC') for x in t):
        return 'pwr'
    return None


def role_ok(orole, ktype):
    k = KT.get(ktype, 'inout')
    if k == 'inout':
        return True
    if k == 'nc':
        return orole in ('nc', 'absent')
    if k == 'power':
        return orole in ('power', 'ground', 'output')
    if orole in ('power', 'ground', 'nc', 'absent'):
        return False
    if k == 'input':
        return orole in ('input', 'inout')
    return orole in ('output', 'inout')


def pin_compare(opins, kpins):
    """Compare our pinout with a KiCad symbol's {number: (name, type)}.

    exact / conv: pins whose name agrees / whose name differs only by
    convention; bad: [(number, ours, kicad, why)]; extra: KiCad pins ours
    lacks; ep: a lone exposed-pad pin KiCad adds."""
    exact = conv = 0
    bad, convl, idx = [], [], {}
    if any('ball' in p for p in opins) and not any(re.match(r'^[A-Z]+\d+$', k) for k in kpins):
        ok = tot = 0
        miss = []
        kt = [toks(v[0]) for v in kpins.values()]
        unnamed = sum(1 for t in kt if t is None)
        for p in opins:
            if p['role'] in ('nc', 'absent'):
                continue
            tot += 1
            a = toks(p['name'])
            if any(t and a and (t & a) for t in kt):
                ok += 1
            elif unnamed > 0:
                unnamed -= 1
                ok += 1
            else:
                miss.append((p.get('ball'), p['name']))
        return dict(ball=True, exact=ok, total=tot, miss=miss, conv=0, convl=[], bad=[], extra=[], ep=[])
    onum = {str(p.get('number', p.get('ball'))): p for p in opins}
    for num, p in onum.items():
        a = toks(p['name'])
        orole = p['role']
        if orole in ('nc', 'absent'):
            if num in kpins:
                kn, kt = kpins[num]
                b = toks(kn)
                if not (kt in ('no_connect', 'unconnected', 'passive') or b is None or 'NC' in b or (a and 'NC' in a)):
                    bad.append((num, p['name'], kn, 'ours is NC'))
            continue
        if num not in kpins:
            bad.append((num, p['name'], None, 'no such pin in KiCad'))
            continue
        kn, kt = kpins[num]
        b = toks(kn)
        oc = pclass(a) or ('gnd' if orole == 'ground' else 'pwr' if orole == 'power' else None)
        kc = pclass(b)
        if b is not None and a is not None and (a & b):
            exact += 1
            continue
        if oc and kc and oc != kc:
            bad.append((num, p['name'], kn, 'power/ground differ'))
            continue
        if not role_ok(orole, kt):
            bad.append((num, p['name'], kn or '~', KT.get(kt, kt) + ' vs ' + orole))
            continue
        if b is None:           # KiCad leaves the pin unnamed: only the direction is compared
            exact += 1
            continue
        # The names differ and the directions fit. Was our name used by another KiCad pin?
        swapped = None
        if a:
            for n2, (kn2, _) in kpins.items():
                if n2 != num and toks(kn2) and (toks(kn2) & a):
                    swapped = n2
                    break
        if swapped:
            ma = re.match(r'^(\d*[A-Z]+?)(\d+)$', re.sub(r'[^A-Z0-9]', '', (p['name'] or '').upper()))
            mb = re.match(r'^(\d*[A-Z]+?)(\d+)$', re.sub(r'[^A-Z0-9]', '', (kn or '').upper()))
            if ma and mb and ma.group(1) == mb.group(1):    # A1 vs A0: a bus numbered differently
                idx.setdefault(ma.group(1), []).append(
                    (num, p['name'], kn, int(ma.group(2)) - int(mb.group(2)), swapped))
                continue
            bad.append((num, p['name'], kn, 'name found on pin ' + swapped))
            continue
        conv += 1
        convl.append((num, p['name'], kn))
    for g in idx.values():
        # a bus is only "numbered differently" when every pin is off by the same amount,
        # or the pins just trade places among themselves
        if (len(g) >= 2 and len({x[3] for x in g}) == 1) or (len(g) >= 3 and {x[0] for x in g} == {x[4] for x in g}):
            conv += len(g)
            convl += [(x[0], x[1], x[2]) for x in g]
        else:
            bad += [(x[0], x[1], x[2], 'name found on pin ' + x[4]) for x in g]
    extra = [n for n in kpins if n not in onum and kpins[n][1] not in ('no_connect', 'unconnected')
             and kpins[n][0] not in ('NC', '')]
    ep = []
    if len(extra) == 1:
        t = toks(kpins[extra[0]][0]) or set()
        if any(x.startswith('GND') or x in ('EP', 'PAD', 'THERMAL') for x in t):
            ep = extra
    return dict(exact=exact, conv=conv, convl=convl, bad=bad, extra=[n for n in extra if n not in ep],
                ep=[(n, kpins[n][0]) for n in ep])


def pin_status(r):
    pr = r['pr']
    if pr.get('ball'):
        return 'n/a'
    nb = len(pr['bad']) + len(pr['extra'])
    tot = pr['exact'] + pr['conv'] + nb
    if nb == 0:
        return 'Yes' if pr['conv'] == 0 else 'Names differ'
    return 'Partial' if nb <= 0.25 * max(tot, 1) else 'No'


# ----------------------------------------------------------- footprints ----

def centre(pads):
    return (sum(p[0] for p in pads) / len(pads), sum(p[1] for p in pads) / len(pads))


def deviation(ours, kicad, ids):
    """Largest distance between same-numbered pads once both are centred,
    over the 4 rotations and a mirror, whichever fits best."""
    o = {d: centre(ours[d]) for d in ids}
    k = {d: centre(kicad[d]) for d in ids}
    oc = (sum(v[0] for v in o.values()) / len(o), sum(v[1] for v in o.values()) / len(o))
    kc = (sum(v[0] for v in k.values()) / len(k), sum(v[1] for v in k.values()) / len(k))
    best = None
    for r in range(4):
        for mir in (1, -1):
            m = 0
            for d in ids:
                ax, ay = o[d][0] - oc[0], o[d][1] - oc[1]
                x, y = k[d][0] - kc[0], k[d][1] - kc[1]
                bx, by = [(x, y), (-y, x), (-x, -y), (y, -x)][r]
                m = max(m, math.hypot(ax - bx * mir, ay - by))
                if best is not None and m >= best:
                    break
            if best is None or m < best:
                best = m
    return best


# Libraries that fit each kind of part. Chips take Package_*; the others only their own.
PREFERRED = {'diode': ('Diode_',), 'resistor': ('Resistor_',), 'capacitor': ('Capacitor_',),
             'crystal': ('Crystal',), 'oscillator': ('Oscillator',), 'led': ('LED_',),
             'switch': ('Button_Switch',), 'header': ('Connector_Pin',), 'connector': ('Connector_',),
             'charlcd': ('Display',), '7seg': ('Display',), 'nixie': ('Display',),
             'dekatron': ('Display',), 'supply': ('Resistor_',)}
SPECIAL = {'resistor', 'capacitor', 'crystal', 'oscillator', 'led', 'switch', 'header', 'connector',
           'charlcd', '7seg', 'nixie', 'dekatron', 'supply'}


def tier(d):
    return 0 if d <= YES else 1 if d <= CLOSE else 2


class Footprints:
    """Finds KiCad's closest footprint for one of ours."""

    def __init__(self, ours, parts, kicad, symbols):
        self.ours, self.kicad, self.symbols = ours, kicad, symbols
        self.kinds = collections.defaultdict(set)       # our footprint -> kinds using it
        for p in parts:
            if p['fp']:
                self.kinds[p['fp']].add(p['kind'])
        self.index = collections.defaultdict(list)      # numbered pad set -> KiCad footprints
        for k, pads in kicad.items():
            ids = frozenset(n for n in pads if NUMBERED.match(n)
                            or (BALL.match(n) and not n.startswith(('MP', 'SH'))))
            if ids:
                self.index[ids].append(k)
        self.names = [(k, k.split(':', 1)[1].lower()) for k in kicad]
        self.cands, self.pools = {}, {}

    def candidates(self, fpid):
        """[(offset, KiCad footprint)] with our pad numbers (and at most one extra thermal pad)."""
        if fpid not in self.cands:
            ours = expand(self.ours[fpid])
            ids = frozenset(d for d in ours if d != 'NULL')
            found = list(self.index.get(ids, []))
            if all(NUMBERED.match(d) for d in ids):
                found += self.index.get(ids | {str(len(ids) + 1)}, [])
            self.cands[fpid] = sorted((round(deviation(ours, self.kicad[k], ids), 3), k) for k in found)
        return self.cands[fpid]

    def pool(self, symkey):
        """The footprints the symbol itself suggests (ki_fp_filters, default Footprint)."""
        pr = self.symbols[symkey]['props']
        key = (pr.get('ki_fp_filters', ''), pr.get('Footprint', ''))
        if key not in self.pools:
            pats = [p.lower() for p in key[0].split()]
            s = {key[1]} if key[1] in self.kicad else set()
            s |= {k for k, n in self.names if any(fnmatch.fnmatchcase(n, p) for p in pats)}
            self.pools[key] = s
        return self.pools[key]

    def allowed(self, k, fpid):
        lib = k.split(':')[0]
        if lib.startswith('Package_') and not self.kinds[fpid] <= SPECIAL:
            return True
        return any(lib.startswith(PREFERRED.get(kind, ())) for kind in self.kinds[fpid])

    def result(self, fpid, symkey):
        """(Yes|Close|No|None, KiCad footprint, offset mm, in the symbol's pool)."""
        pool = self.pool(symkey) if symkey else set()
        elig = [(d, k) for d, k in self.candidates(fpid) if k in pool or self.allowed(k, fpid)]
        if not elig:
            return ('None', None, None, False)
        sizes = re.findall(r'(?<!\d)\d{4}(?!\d)', fpid)     # RES-0603-X prefers an R_0603 footprint
        if sizes and self.kinds[fpid] <= SPECIAL:
            near = [(d, k) for d, k in elig if tier(d) <= 1 and any(t in k.split(':', 1)[1] for t in sizes)]
            elig = near or elig
        d, k = min(elig, key=lambda t: (tier(t[0]), 0 if t[1] in pool else 1,
                                        0 if t[1].startswith('Package_') else 1, t[0], len(t[1]), t[1]))
        return (['Yes', 'Close', 'No'][tier(d)], k, d, k in pool)


# ---------------------------------------------------------------- report ----

def esc(s):
    return str(s).replace('|', '\\|')


def q(s):
    return '`' + esc(s) + '`'


def short(n, m=18):
    n = '-' if n is None else str(n)
    return n if len(n) <= m else n[:m - 1] + '…'


def pin_notes(r):
    pr, out = r['pr'], []
    if pr.get('ball'):
        s = "ours names pins by ball (A1…), KiCad's symbol numbers them; compared by name: %d of %d match" % (pr['exact'], pr['total'])
        if pr['miss']:
            s += ' (no KiCad name for ' + ', '.join('%s %s' % (b, short(n)) for b, n in pr['miss'][:3]) + ')'
        return [s]
    if pr['bad']:
        ex = []
        for num, on, kn, why in pr['bad'][:3]:
            if kn is None:
                ex.append('pin %s %s: not in KiCad symbol' % (num, q(short(on))))
            else:
                ex.append('pin %s: ours %s, KiCad %s (%s)' % (num, q(short(on)), q(short(kn)), why))
        more = len(pr['bad']) - 3
        out.append('%d pin%s differ: ' % (len(pr['bad']), '' if len(pr['bad']) == 1 else 's')
                   + '; '.join(ex) + (' …+%d' % more if more > 0 else ''))
    if pr['extra']:
        out.append('KiCad has pin%s %s that ours lacks' % ('s' if len(pr['extra']) > 1 else '',
                   ', '.join(pr['extra'][:4]) + ('…' if len(pr['extra']) > 4 else '')))
    if pr.get('ep'):
        out.append('KiCad adds exposed-pad pin %s (%s)' % (pr['ep'][0][0], q(pr['ep'][0][1])))
    if pr['conv']:
        ex = ', '.join('%s %s/%s' % (n, q(short(a, 10)), q(short(k, 10))) for n, a, k in pr['convl'][:2])
        out.append('%d pin names differ in convention (%s%s)' % (pr['conv'], ex, '…' if pr['conv'] > 2 else ''))
    if r['nk'] and r['nk'] != r['np'] and not pr.get('ep'):
        out.append('KiCad symbol has %d pins, ours %d' % (r['nk'], r['np']))
    return out


def fp_notes(r):
    st, k, d, inpool = r['fr']
    if st == 'None' or k is None:
        return ['no KiCad footprint with ours %s pad set' % q(r['fp'])]
    if st == 'No':
        return ['ours %s: nothing in KiCad within 0.5 mm; nearest is %s, %.1f mm off' % (q(r['fp']), q(k), d)]
    s = 'ours %s %s KiCad %s, max pad offset %.2f mm' % (q(r['fp']), '=' if st == 'Yes' else '≈', q(k), d)
    return [s + ("" if inpool else " (not in the symbol's footprint filter)")]


def table_row(r):
    note = 'Symbol ' + q(r['sym']) + (' (54 series → 74 symbol)' if r['mil'] else '')
    if r['nsyms'] > 1:
        note += ' (best of %d)' % r['nsyms']
    notes = [note] + pin_notes(r) + fp_notes(r)
    return '| %s | %s | %s | %s |' % (esc(r['name']), pin_status(r), r['fr'][0], '. '.join(notes))


KIND_TITLES = {'7seg': '7-segment displays', 'arith': 'Arithmetic', 'buffer': 'Buffers', 'capacitor': 'Capacitors',
               'charlcd': 'Character LCDs', 'connector': 'Connectors', 'converter': 'Converters (ADC/DAC)',
               'counter': 'Counters', 'cpu': 'Processors', 'crystal': 'Crystals', 'decoder': 'Decoders',
               'dekatron': 'Dekatrons', 'dff': 'D flip-flops', 'diode': 'Diodes', 'eeprom': 'Parallel EEPROM',
               'gate': 'Gates', 'header': 'Headers', 'jkff': 'JK flip-flops', 'latch': 'Latches', 'led': 'LEDs',
               'mcu': 'Microcontrollers', 'mux': 'Multiplexers', 'nixie': 'Nixie tubes', 'oscillator': 'Oscillators',
               'pio': 'Parallel I/O', 'register': 'Registers', 'regulator': 'Regulators', 'resistor': 'Resistors',
               'rtc': 'Real-time clocks', 'serial': 'Serial controllers', 'serialmem': 'Serial memory',
               'shift': 'Shift registers', 'sram': 'SRAM', 'supply': 'Supplies', 'switch': 'Switches',
               'transistor': 'Transistors'}

WHY_NO_SYMBOL = {
    'capacitor': 'KiCad has one generic `C`/`C_Polarized` symbol, not one per part number',
    'resistor': 'KiCad has one generic `R` symbol',
    'switch': 'KiCad has generic switch symbols, not these part numbers',
    'connector': 'KiCad connectors are generic (`Conn_01x04`, `USB_B_Micro`); no part numbers',
    'crystal': 'KiCad has a generic `Crystal` symbol',
    'oscillator': 'only a handful of oscillators have a symbol',
    'header': 'generic `Conn_01xNN` symbols',
    'supply': 'KiCad power symbols (`VCC`, `GND`) are labels, not parts',
    'led': 'KiCad has a generic `LED` symbol',
    'cpu': 'KiCad has no 6502/65816/Z80 symbols with these part numbers',
    'pio': 'no 6522/6521 symbols with these part numbers',
    'nixie': 'no tube symbols', 'dekatron': 'no tube symbols',
    '7seg': 'only two displays have a symbol', 'charlcd': 'only a few modules have a symbol',
    'serialmem': 'KiCad names its EEPROM symbols by family (`24LC256`, `AT24CS01-…`), not by these part numbers',
    'sram': 'KiCad names SRAM symbols by family (`62256`), not by these part numbers',
    'mcu': 'KiCad has no symbol for these exact devices',
    'converter': 'KiCad has symbols for only some converters',
    'regulator': 'KiCad has symbols for only some regulators',
    'diode': 'KiCad has generic diode symbols plus some named ones',
    'transistor': 'KiCad has generic and some named transistor symbols'}

PIN_VALUES = ['Yes', 'Names differ', 'Partial', 'No', 'n/a']
FP_VALUES = ['Yes', 'Close', 'No']

HOW_TO_READ = """A part is matched to a KiCad symbol by part number: the KiCad symbol's name or Value must be the front of our part number once the maker's prefix is dropped (`SN74HC00N` → `74HC00`, `ATTINY412-SSN` → `ATtiny412-SS`) and no digit may follow it (so `74HC13` never matches `74HC139`). The longest match wins. Military `54` parts (`SNJ54LS00FK`, `CD54HC00F`) are matched to the `74` symbol and say so in Notes. When several KiCad symbols fit, the one whose pins agree best is used (the plain library before the `_IEEE` one on a tie); Notes names the symbol.

**Pins Match** compares pin by pin, by pin number, our package pinout against the symbol's pins:

| Value | Meaning |
|---|---|
| Yes | Every pin has the same number, a name KiCad also uses (`VCC`/`VDD`, `GND`/`VSS`, `CLK`, `~{OE}`/`OE#` are equal) and a compatible direction. A KiCad pin with no name (all of the 74xx gates) counts as a match when the direction agrees. |
| Names differ | Same numbers and compatible directions, but KiCad names some pins differently: `1A1` vs `A0` (KiCad counts buses from 0), `1G` vs `Ea2`, `CLR` vs `~{MR}`. The function is the same; only the label is not. Notes gives two examples. |
| Partial | Up to a quarter of the pins disagree: a different function at a pin number, a name that sits on another pin, power vs ground swapped, a direction clash, or a pin only one side has. Notes lists the first three. |
| No | More than a quarter of the pins disagree. Typically KiCad's symbol numbers a different package (a DIP pinout for our 20-pad LCCC, SOT-23-5 vs a 6-pad DSF), or a KiCad symbol with sloppy pin types. |
| n/a | Our pinout names pins by ball (`A1`, `B2`; WLCSP/DSBGA) and KiCad's symbol numbers them `1…N`, so pin numbers cannot be compared. Notes says how many names agree. |

**Footprint Match** compares our footprint's pads with KiCad's footprints. It does not use KiCad's own pairing of symbol and footprint alone: it takes every KiCad footprint that has the same pad numbers as ours (plus at most one extra thermal pad), centres both pad sets on their middle, tries the four rotations and a mirror, and takes the largest distance between a pad of ours and the same-numbered KiCad pad. The footprints the symbol itself suggests (its `ki_fp_filters` and default Footprint) are preferred; when the match is found outside them, Notes says "not in the symbol's footprint filter". Only footprints from a library that fits the part are considered (`Package_*` for chips; `Diode_*`, `Resistor_*`, `Capacitor_*`, `Crystal`, `Oscillator`, `Button_Switch*`, … for those parts).

| Value | Meaning |
|---|---|
| Yes | Largest pad-centre offset 0.10 mm or less: the same land pattern. |
| Close | 0.10 to 0.50 mm: the same package, but our pad rows are narrower or wider than KiCad's (SOIC 0.225 mm, TQFP 0.188 mm). Usable, not identical. |
| No | Nothing within 0.50 mm. Notes names the nearest footprint KiCad has and how far off it is (ceramic flat packs, CPGA, bridge rectifiers and power MOSFETs have no KiCad equivalent). |

Pad **sizes**, silkscreen, courtyards and 3D models are not compared, only where the pads are. Our footprints come from each datasheet's land pattern; KiCad's follow IPC-7351 and its own rules, so a small offset is expected.
"""


def report(parts, rows, fpfinder, kicad_version, n_symbols, n_footprints):
    kinds = sorted({p['kind'] for p in parts})
    total = collections.Counter(p['kind'] for p in parts)
    by = collections.defaultdict(list)
    for r in rows:
        by[r['kind']].append(r)
    L = []
    w = L.append
    w("# KiCad comparison\n")
    w("Compares the part library (`schematical/schematical/Parts/*.json`) with KiCad %s's stock symbol and footprint libraries (`symbols` and `footprints` in KiCad's `SharedSupport`, %s symbols, %s footprints). It is a report only: nothing in the part files was changed. Regenerate it with `python3 scripts/kicad-compare.py`.\n"
      % (kicad_version, format(n_symbols, ','), format(n_footprints, ',')))
    w("**%d of %d parts (%d%%) have a KiCad symbol of the same part number** and are compared below. The other %d have none, because KiCad does not carry symbols for them (see [Parts with no KiCad symbol](#parts-with-no-kicad-symbol)).\n"
      % (len(rows), len(parts), round(100 * len(rows) / len(parts)), len(parts) - len(rows)))
    pc = collections.Counter(pin_status(r) for r in rows)
    fc = collections.Counter(r['fr'][0] for r in rows)
    w("| | " + ' | '.join(PIN_VALUES) + " |\n|---|---|---|---|---|---|")
    w("| **Pins Match** | " + ' | '.join(str(pc[k]) for k in PIN_VALUES) + " |")
    w("\n| | " + ' | '.join(FP_VALUES) + " |\n|---|---|---|---|")
    w("| **Footprint Match** | " + ' | '.join(str(fc[k]) for k in FP_VALUES) + " |\n")
    w("## How to read the table\n")
    w(HOW_TO_READ)

    w("## Pin disagreements worth checking\n")
    w("Parts whose pin **count** is the same as KiCad's symbol (so the pin numbers line up) but the pins still read Partial or No. Each row is a group of parts that share the same symbol and the same disagreement; the first part is an example. These are the places where either our pinout or KiCad's symbol may be wrong, not package-numbering differences.\n")
    groups = collections.OrderedDict()
    for r in sorted((r for r in rows if pin_status(r) in ('Partial', 'No') and r['nk'] == r['np']
                     and not r['pr'].get('ep') and not r['pr'].get('ball')),
                    key=lambda r: (r['kind'], r['sym'], r['name'])):
        key = (r['sym'], tuple((b[0], b[1], b[2]) for b in r['pr']['bad']), tuple(r['pr']['extra']))
        groups.setdefault(key, []).append(r)
    w("| Example part | Parts | Pins Match | KiCad symbol | Disagreement |\n|---|---|---|---|---|")
    for rs in groups.values():
        r = rs[0]
        bad = r['pr']['bad']
        ex = '; '.join('pin %s: ours %s, KiCad %s' % (n, q(short(o)), q(short(k) if k else '—'))
                       for n, o, k, _ in bad[:4]) + (' …+%d' % (len(bad) - 4) if len(bad) > 4 else '')
        w("| %s | %d | %s | %s | %s |" % (esc(r['name']), len(rs), pin_status(r), q(r['sym']), ex))
    w("")

    w("## Summary by kind\n")
    w("| Kind | Parts | With KiCad symbol | Pins: Yes | Names differ | Partial | No | n/a | Footprint: Yes | Close | No |\n|---|---|---|---|---|---|---|---|---|---|---|")
    for k in kinds:
        rs = by[k]
        if not rs:
            w("| %s | %d | 0 | | | | | | | | |" % (k, total[k]))
            continue
        p = collections.Counter(pin_status(r) for r in rs)
        f = collections.Counter(r['fr'][0] for r in rs)
        w("| [%s](#%s) | %d | %d | %s | %s |" % (k, k, total[k], len(rs),
          ' | '.join(str(p[x]) for x in PIN_VALUES), ' | '.join(str(f[x]) for x in FP_VALUES)))
    w("")
    for k in kinds:
        rs = sorted(by[k], key=lambda r: r['name'])
        if not rs:
            continue
        w('<a id="%s"></a>\n' % k)
        w("## %s (`%s`): %d of %d parts\n" % (KIND_TITLES.get(k, k), k, len(rs), total[k]))
        w("| Part Name | Pins Match | Footprint Match | Notes |\n|---|---|---|---|")
        for r in rs:
            w(table_row(r))
        w("")

    w("## Parts with no KiCad symbol\n")
    w("| Kind | Parts without a KiCad symbol | Why |\n|---|---|---|")
    for k in kinds:
        n = total[k] - len(by[k])
        if n:
            w("| %s | %d | %s |" % (k, n, WHY_NO_SYMBOL.get(k, 'no symbol of that part number in KiCad')))
    w("")
    w("### Footprints of the parts with no KiCad symbol\n")
    w("The same footprint comparison, one row per footprint, for every footprint a part without a KiCad symbol uses. Pins Match does not apply (there is no symbol).\n")
    matched = {r['name'] for r in rows}
    use = collections.defaultdict(collections.Counter)
    for p in parts:
        if p['name'] not in matched and p['fp']:
            use[p['fp']][p['kind']] += 1
    w("| Footprint | Used by (parts) | Footprint Match | Notes |\n|---|---|---|---|")
    counts = collections.Counter()
    for f in sorted(use):
        st, k, d, _ = fpfinder.result(f, None)
        counts[st] += 1
        if st == 'None' or k is None:
            note = 'no KiCad footprint in a fitting library has the same pad set'
        elif st == 'No':
            note = 'nothing in KiCad within 0.5 mm; nearest is %s, %.1f mm off' % (q(k), d)
        else:
            note = 'KiCad %s, max pad offset %.2f mm' % (q(k), d)
        w("| %s | %s | %s | %s |" % (esc(f), ', '.join('%s %d' % kv for kv in use[f].most_common(3)), st, note))
    w("\n_%d footprints: Yes %d, Close %d, No %d, none %d._\n"
      % (len(use), counts['Yes'], counts['Close'], counts['No'], counts['None']))
    return '\n'.join(L)


# ------------------------------------------------------------------ main ----

def find_kicad(arg):
    if arg:
        return arg
    for app in (os.path.expanduser('~/Applications/KiCad/KiCad.app'), '/Applications/KiCad/KiCad.app'):
        if os.path.isdir(os.path.join(app, 'Contents', 'SharedSupport', 'symbols')):
            return os.path.join(app, 'Contents', 'SharedSupport')
    sys.exit('KiCad not found: pass --kicad <its SharedSupport directory>')


def kicad_version(shared):
    plist = os.path.join(shared, '..', 'Info.plist')
    try:
        with open(plist, 'rb') as fh:
            return plistlib.load(fh)['CFBundleShortVersionString']
    except (OSError, KeyError, plistlib.InvalidFileException):
        return '(unknown version)'


def main():
    ap = argparse.ArgumentParser(description='Compare the part library with KiCad symbols and footprints.')
    ap.add_argument('--kicad', help="KiCad's SharedSupport directory")
    ap.add_argument('--out', default=os.path.join(REPO, 'planning', 'kicad-compare.md'))
    args = ap.parse_args()
    shared = find_kicad(args.kicad)

    print('reading KiCad symbols…', file=sys.stderr)
    symbols = load_symbols(shared)
    print('reading KiCad footprints…', file=sys.stderr)
    kicad_fps = load_footprints(shared)
    ours, parts = load_ours()
    keys = build_symbol_keys(symbols)
    fpfinder = Footprints(ours, parts, kicad_fps, symbols)

    print('comparing %d parts…' % len(parts), file=sys.stderr)
    rows = []
    for p in parts:
        syms, mil = find_symbols(keys, p['name'], 'military' in p['flags'])
        if not syms:
            continue
        best = None
        for sk in syms:
            r = pin_compare(p['pins'], symbols[sk]['pins'])
            score = (len(r['bad']) + len(r['extra']) + (len(r['miss']) if r.get('ball') else 0),
                     r['conv'], 1 if 'IEEE' in sk else 0, sk)
            if best is None or score < best[0]:
                best = (score, sk, r)
        _, sk, pr = best
        rows.append(dict(name=p['name'], kind=p['kind'], sym=sk, mil=mil, pr=pr, nsyms=len(syms), fp=p['fp'],
                         fr=fpfinder.result(p['fp'], sk) if p['fp'] else ('None', None, None, False),
                         nk=len(symbols[sk]['pins']), np=len(p['pins'])))
    text = report(parts, rows, fpfinder, kicad_version(shared), len(symbols), len(kicad_fps))
    with open(args.out, 'w') as fh:
        fh.write(text)
    print('%d of %d parts have a KiCad symbol; wrote %s' % (len(rows), len(parts), args.out), file=sys.stderr)


if __name__ == '__main__':
    main()
