#!/usr/bin/env python3
"""plan-45-C/D/E: the body width of each generated footprint against the width its datasheet prints.

usage: python3 -I fp-widths.py <letter> [<footprints-X.json>]

For every `add` row of the letter's variants (C: fixed blocks, D: pluggable headers, E: JST) whose datasheet prints
the part's overall length, compare it with the `body.width` of the footprint the row will use (id from the
variant's label and the row's positions). Per maker, what is read (`pdftotext -layout`):
  Weidmuller   `Width  <w> mm`              (the Dimensions table)
  Wurth        `Length  L  <w>  mm`         (per-item sheets); the series sheets print a formula and are not read
  Phoenix      `Width  <w> mm` (only some sheets print it; the others give `Dimension a`, the hole span, which
               is (n - 1) x pitch by construction and is not compared)
Prints the rows compared, the mismatches (> 0.05 mm) and `<n> compared, <k> mismatches, <m> not printed`.
"""
import csv
import json
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))


def text(rel):
    return subprocess.run(['pdftotext', '-q', '-layout', os.path.join(ROOT, rel), '-'], capture_output=True, text=True).stdout


def number(tok):
    """'10,16.00' -> 10.16, '15.00' -> 15.0, '40,64.00' -> 40.64 (Wurth's 0008 / 0011 tables print a decimal comma
    and a trailing .00 glued to the tolerance)."""
    parts = tok.replace(',', '.').split('.')
    return float(parts[0] + '.' + parts[1]) if len(parts) >= 2 else float(parts[0])


def printed(maker, t, mpn=''):
    if maker == 'JST':
        # the header tables: "<n> <model(s)> <A> <B> <Q'ty>": B is the second decimal after the part number
        # (the models of one row share it; the model may be one of several on the line)
        for line in t.splitlines():
            m = re.search(r'(?<![A-Za-z0-9])' + re.escape(mpn) + r'(?![A-Za-z0-9-])(.*)', line)
            if m:
                # standalone numeric tokens only: the model name "B4B-ZR-3.4" holds a "3.4" that is not a column
                nums = [float(x) for x in m.group(1).split() if re.fullmatch(r'\d+\.\d+', x)]
                if len(nums) >= 2:
                    return nums[1]
        return None
    if maker == 'Weidmuller':
        m = re.search(r'(?<![A-Za-z(])Width\s+([\d.]+)\s*mm', t)
    elif maker == 'Wurth Elektronik':
        m = re.search(r'Length\s+L\s+([\d.]+)\s*mm', t)
        if not m:      # the 0008 / 0011 tables: "<pins>  <Pl> ±<tol>  <L> ±<tol>  mm  <order code>"
            m2 = re.search(r'^\s*\d+\s+\S+\s*±\s*[\d.]+\s+(\S+)\s*±\s*[\d.]+\s+mm\s+\d{12,}', t, re.M)
            return number(m2.group(1)) if m2 else None
    else:
        m = re.search(r'^\s*Width\s+([\d.]+)\s*mm', t, re.M)
    return float(m.group(1)) if m else None


def span(fp):
    """The distance between the first and last column of holes of a footprint object."""
    if fp['style'] == 'explicit':
        xs = [p['x'] for p in fp['pads']]
        return max(xs) - min(xs)
    return (fp['pins'] - 1) * fp['pitch']


def hole_span(maker, t):
    """Phoenix prints the hole span as `Dimension a`."""
    if maker != 'Phoenix Contact':
        return None
    m = re.search(r'^\s*Dimension a\s+([\d.]+)\s*mm', t, re.M)
    return float(m.group(1)) if m else None


def main():
    letter = sys.argv[1]
    args = [a for a in sys.argv[2:] if not a.startswith('--')]
    path = args[0] if args else os.path.join(DATA, f'footprints-{letter}.json')
    fps = {o['id']: o for o in json.load(open(path))['footprints']}
    labels = {}
    for line in open(os.path.join(DATA, 'footprints-needed.txt')):
        if line.startswith('#') or not line.strip():
            continue
        f = line.rstrip('\n').split('\t')
        if f[0] == letter:
            labels[f[1]] = f[10]
    compared = bad = unprinted = spans = wide = 0
    per_variant = {}
    # variants the table draws with their family's widest outline ("widest": true): the footprint may be wider than a sheet, not narrower
    table = json.load(open(os.path.join(ROOT, 'scripts', 'terminal-footprints.json')))['variants']
    widest = {('' if v['letter'] == 'E' else 'TB-') + v['stem']: bool(v.get('widest')) for v in table}

    def stem_of(fid):
        return fid.rsplit('-', 1)[0]

    for r in csv.DictReader(open(os.path.join(DATA, 'candidates.tsv')), delimiter='\t'):
        if r['state'] != 'add' or r['variant'] not in labels:
            continue
        stem = labels[r['variant']]
        fid = f"{'' if letter == 'E' else 'TB-'}{stem}-{int(r['positions']):02d}"
        pdf_text = text(r['pdf'])
        a = hole_span(r['maker'], pdf_text)
        if a is not None:
            spans += 1
            if abs(span(fps[fid]) - a) > 0.05:
                bad += 1
                print(f"MISMATCH {r['mpn']} {fid}: datasheet hole span a = {a}, footprint {span(fps[fid]):g}")
        w = printed(r['maker'], pdf_text, r['mpn'])
        if w is None:
            unprinted += 1
            continue
        compared += 1
        got = fps[fid]['body']['width']
        per_variant.setdefault(r['variant'], {})[int(r['positions'])] = (r['mpn'], w, got)
        if widest.get(stem_of(fid)) and -0.05 <= got - w <= 3.5:
            wide += 1                       # a variant drawn with its family's widest outline: never narrower than a sheet
        elif abs(got - w) > 0.05:
            bad += 1
            print(f"MISMATCH {r['mpn']} {fid}: datasheet {w}, footprint {got}")
    if '--ends' in sys.argv:       # the smallest and largest position count the sheets print, per variant
        for v, d in per_variant.items():
            ns = sorted(d)
            parts = [f'{n} pos {d[n][0]}: sheet {d[n][1]:g}, footprint {d[n][2]:g}' for n in dict.fromkeys((ns[0], ns[-1]))]
            print(f"{labels[v]} | " + '; '.join(parts))
    print(f'{compared} widths compared ({wide} of them against a "widest" outline), {spans} hole spans compared, '
          f'{bad} mismatches, {unprinted} widths not printed')
    sys.exit(1 if bad else 0)


main()
