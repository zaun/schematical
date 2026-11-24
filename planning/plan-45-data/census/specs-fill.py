#!/usr/bin/env python3
"""plan-45-B Phase 3: the rated voltage and current each datasheet prints, and the pitch / position checks.

usage: python3 -I specs-fill.py <candidates.tsv> <specs.tsv> [--write] [--list]

For every `add` row and every `candidate` row with a placed PDF (`pdftotext -layout`, cached text):

  volts, amps  the rating the datasheet prints, § 3 rule 3: the IEC rating when it prints both IEC and UL
               values (Phoenix: the headline "Nominal current ..., Rated voltage (III/2) ..." is the IEC
               figure; Weidmuller: "Product data  IEC: <V> V / <A> A"; Wurth: the VDE column, the second of
               "cULus / UL  VDE", or the VDE [IEC 61984] section), else the one it prints (Wurth series sheets
               "CURRENT RATING: 20A"; JST). Empty when the sheet prints none.
  pitch        the sheet's pitch, compared with the row's `pitch` (DigiKey's)
  positions    where the sheet ties the part number to a count (Phoenix "Number of positions", Weidmuller
               "Number of poles", a Wurth table row naming the part), compared with the row's `positions`;
               the JST sheets are checked by scheme-check.py (n = positions)

The values replace DigiKey's `volts` / `amps` in candidates.tsv (and fill specs.tsv's `volts` / `amps` where the
spec row's rows agree; JST VH's two ratings are per part: 10 A standard header, 7 A shrouded `-FB-B`).
Prints the counts, the disagreements (pitch or positions) and, with --list, every row's reading.
--write writes candidates.tsv and specs.tsv; without it nothing is changed.
"""
import collections
import csv
import hashlib
import os
import re
import subprocess
import sys

NO_DATA = 'no technical data in the datasheet'
DISAGREES = 'datasheet disagrees'
ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..'))
CACHE = '/tmp/p45/txt'
NUM = r'(\d+(?:\.\d+)?)'


def text(rel):
    os.makedirs(CACHE, exist_ok=True)
    f = os.path.join(CACHE, hashlib.sha1(rel.encode()).hexdigest() + '.txt')
    if not os.path.exists(f):
        subprocess.run(['pdftotext', '-q', '-layout', os.path.join(ROOT, rel), f], check=False, timeout=300)
    return open(f, errors='replace').read()


def first(rx, t, flags=re.M):
    m = re.search(rx, t, flags)
    return m


def phoenix(t, row):
    m = first(r'[Nn]ominal current:\s*' + NUM + r'\s*A,\s*(?:[Nn]om\. voltage|[Rr]ated voltage \(III/2\)|[Nn]ominal voltage):\s*' + NUM + r'\s*V', t)
    amps = volts = ''
    how = 'headline (IEC)'
    if m:
        amps, volts = m.group(1), m.group(2)
    else:
        a = first(r'^\s*Nominal current IN\s+' + NUM + r'\s*A', t)
        # the IEC rated voltage: overvoltage category / pollution degree III/2 (the headline's), else the
        # one the sheet prints: III/3, then II/2 (a header whose headline names only the current)
        v = (first(r'^\s*Rated voltage \(III/2\)\s+' + NUM + r'\s*V', t) or first(r'^\s*Rated voltage \(III/3\)\s+' + NUM + r'\s*V', t)
             or first(r'^\s*Rated voltage \(II/2\)\s+' + NUM + r'\s*V', t))
        amps, volts, how = (a.group(1) if a else ''), (v.group(1) if v else ''), 'IEC table'
    p = first(r'^\s*Pitch\s+' + NUM + r'\s*mm', t)
    n = first(r'^\s*Number of positions\s+(\d+)', t)
    return amps, volts, how, float(p.group(1)) if p else None, int(n.group(1)) if n else None


def weidmuller(t, row):
    m = first(r'IEC:\s*' + NUM + r'\s*V\s*/\s*' + NUM + r'\s*A', t)
    volts, amps = (m.group(1), m.group(2)) if m else ('', '')
    p = first(r'Pitch in mm \(P\)\s+' + NUM + r'\s*mm', t)
    n = first(r'Number of poles\s+(\d+)', t)
    return amps, volts, 'Product data IEC' if m else 'none', float(p.group(1)) if p else None, int(n.group(1)) if n else None


def wurth(t, row):
    # a VDE [IEC 61984] section first (the sheets that print UL and VDE in separate blocks)
    sec = re.search(r'^\s*VDE\b[^\n]*\n(.*?)(?=^\s*(?:UL|cULus|CSA|GWEPT|Certification)\b|\Z)', t, re.M | re.S)
    amps = volts = ''
    how = 'none'
    if sec:
        a = first(r'Rated Current\s+IR\s+' + NUM + r'\s*A', sec.group(1))
        v = first(r'Rated Voltage\s+VR\s+' + NUM + r'\s*V', sec.group(1))
        if a or v:
            amps, volts, how = (a.group(1) if a else ''), (v.group(1) if v else ''), 'VDE section'
    if how == 'none':
        # a two-column table (cULus / UL, VDE): the VDE column is the second number
        a = first(r'Rated Current\s+IR\s*(?:@\s*' + NUM + r'\s*°C)?\s+' + NUM + r'(?:\s+' + NUM + r')?\s*A\b', t)
        v = first(r'(?:Working|Rated) Voltage\s+(?:VR\s+)?' + NUM + r'(?:\s+' + NUM + r')?\s*V', t)
        if a:
            # groups: 1 the "@ <n> °C" temperature, 2 the first value (cULus / UL), 3 the second (VDE)
            amps, how = (a.group(3) or a.group(2)), 'table second column' if a.group(3) else 'table'
        if v:
            volts = v.group(2) or v.group(1)
            how = how if how != 'none' else 'table'
    if how == 'none':
        # a series sheet: CURRENT RATING: 6A 2A / WORKING VOLTAGE: 300 VAC 250 VAC (cULus, VDE)
        a = first(r'CURRENT RATING:\s*' + NUM + r'\s*A(?:\s+' + NUM + r'\s*A)?', t)
        v = first(r'WORKING VOLTAGE:\s*' + NUM + r'\s*VAC(?:\s+' + NUM + r'\s*VAC)?', t)
        if a:
            amps, how = (a.group(2) or a.group(1)), 'series sheet'
        if v:
            volts = v.group(2) or v.group(1)
            how = how if how != 'none' else 'series sheet'
    p = first(r'^\s*Pitch\s+P\s+' + NUM, t) or first(r'PITCH:\s*' + NUM + r'\s*MM', t, re.M | re.I)
    # the position count where a table row names the part: "<n> <A> mm <B> mm <order code>"
    n = first(r'^\s*(\d{1,2})\s+[\d.]+\s*mm\s+[\d.]+\s*mm\s+' + re.escape(row['mpn']) + r'\b', t)
    return amps, volts, how, float(p.group(1)) if p else None, int(n.group(1)) if n else None


# JST: the series sheet's "Specifications" box (read 2026-11-24): pdf -> (amps, volts)
JST = {'eEH.pdf': ('3', '250'), 'eGH.pdf': ('1.0', '50'), 'ePH.pdf': ('2', '100'), 'eSH.pdf': ('1.0', '50'),
       'eSR_SZ.pdf': ('0.7', '50'), 'eSR_SZ-new.pdf': ('0.7', '50'), 'eVH.pdf': ('10', '250'),
       'eXH.pdf': ('3', '250'), 'eZH.pdf': ('1.0', '50')}


def jst(t, row):
    name = row['pdf'].split('/')[-1]
    amps, volts = JST[name]
    if name == 'eVH.pdf' and row['mpn'].endswith('-FB-B'):
        amps = '7'                                   # "7A ... with the shrouded type header" (p1)
    # the sheet's own check: the title line "<pitch> mm pitch/Wire-to-Board ..."
    p = first(r'^\s*' + NUM + r'\s*mm pitch/', t)
    return amps, volts, 'specification box', float(p.group(1)) if p else None, None


READERS = {'Phoenix Contact': phoenix, 'Weidmuller': weidmuller, 'Wurth Elektronik': wurth, 'JST': jst}


def fmt(x):
    return x


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    cand_path, specs_path = args[0], args[1]
    write, listing = '--write' in sys.argv, '--list' in sys.argv
    rows = list(csv.DictReader(open(cand_path), delimiter='\t'))
    cols = list(rows[0].keys())
    how_count = collections.Counter()
    disagree = []
    stubs = []
    checked = collections.Counter()
    empty = collections.Counter()
    per_spec = collections.defaultdict(set)
    for r in rows:
        if r['state'] not in ('candidate', 'add') or not r['pdf']:
            continue
        t = text(r['pdf'])
        amps, volts, how, pitch, pos = READERS[r['maker']](t, r)
        how_count[(r['maker'], how)] += 1
        if not amps:
            empty[(r['maker'], 'amps')] += 1
        if not volts:
            empty[(r['maker'], 'volts')] += 1
        checked[(r['maker'], 'pitch', pitch is not None)] += 1
        checked[(r['maker'], 'positions', pos is not None)] += 1
        if pitch is not None and abs(pitch - float(r['pitch'])) > 0.01:
            disagree.append((r['mpn'], 'pitch', pitch, r['pitch']))
        if pos is not None and pos != int(r['positions']):
            disagree.append((r['mpn'], 'positions', pos, r['positions']))
        if listing:
            print(f"{r['maker']} | {r['mpn']} | {amps or '-'} A | {volts or '-'} V | {how} | dk {r['amps'] or '-'} A {r['volts'] or '-'} V | pitch {pitch} pos {pos}")
        r['amps'], r['volts'] = amps, volts
        per_spec[(r['maker'], r['variant'], r['pdf'])].add((amps, volts))
        # a placed sheet that prints no rating, no voltage and no pitch is a stub (the Wurth `/components/products/`
        # copy of 950018205000000 and 950018207000000: soldering profile, cautions, disclaimer): no usable PDF
        if not amps and not volts and pitch is None:
            stubs.append(r['mpn'])
            r['state'], r['reason'] = 'skip', NO_DATA
        elif any(d[0] == r['mpn'] for d in disagree):
            r['state'], r['reason'] = 'skip', DISAGREES
    for k, v in sorted(how_count.items()):
        print(f'{k[0]}: {v} rows read by {k[1]}')
    for k, v in sorted(empty.items()):
        print(f'{k[0]}: {v} rows with no {k[1]} printed (stay empty)')
    for maker in sorted({k[0] for k in checked}):
        print(f"{maker}: pitch compared on {checked[(maker, 'pitch', True)]} rows, not printed on {checked[(maker, 'pitch', False)]}; "
              f"positions compared on {checked[(maker, 'positions', True)]} rows, not tied to the part on {checked[(maker, 'positions', False)]}")
    for d in disagree:
        print('DISAGREES', *d)
    for m in stubs:
        print('NO DATA', m)
    print(f'{len(disagree)} pitch / position disagreements, {len(stubs)} sheets with no technical data')
    if write:
        with open(cand_path, 'w') as out:
            out.write('\t'.join(cols) + '\n')
            for r in rows:
                out.write('\t'.join(str(r[c]) for c in cols) + '\n')
        specs = list(csv.DictReader(open(specs_path), delimiter='\t'))
        scols = list(specs[0].keys())
        for s in specs:
            vals = per_spec.get((s['maker'], s['variant'], s['pdf']))
            if vals and len(vals) == 1:
                s['amps'], s['volts'] = next(iter(vals))
            elif vals:
                s['amps'] = '/'.join(sorted({a for a, _ in vals}, key=float)) if all(a for a, _ in vals) else ''
                s['volts'] = '/'.join(sorted({v for _, v in vals}, key=float)) if all(v for _, v in vals) else ''
        with open(specs_path, 'w') as out:
            out.write('\t'.join(scols) + '\n')
            for s in specs:
                out.write('\t'.join(s[c] for c in scols) + '\n')
        print('written')
    sys.exit(1 if disagree else 0)


main()
