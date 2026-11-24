#!/usr/bin/env python3
"""plan-55-F/G: does every land-pattern line have a footprint?

usage: python3 -I fp-check.py <letter> [--list] [--map <extra map tsv>]... [--json <extra footprints json>]...

Joins the letter's lines of footprints-needed.txt (letter, company, package, supplier package, add rows, ...) with the
footprint maps (planning/plan-55-data/footprint-map.tsv, every footprint-map-*.tsv, and any --map file: columns
letter, company, package, supplier_package, pins, footprint, source (`reuse` or `new`), drawing, pdf (the datasheet
file name the drawing is from, or `*`: a row for one PDF wins over a `*` row), note, and optionally `mpn` (a regex that
must fully match the part number, case-insensitive: for a PDF whose parts of one package label come in two packages;
a row with an `mpn` wins over one without); `supplier_package` is the line's Supplier Device Package or `*`; `pins` is
the pinout's pin count or `*`). A line needs a map row for every (PDF, pin
count) its `add` rows have; the mapped id must be in Parts/footprints.json, a footprints-F.json / footprints-G.json of
this directory, or a --json file, and the footprint's `pins` must equal the pin count. Prints `<n> lines, <m> without a
footprint` and each missing line, unknown id or pin-count mismatch; --list also prints the footprint chosen per line.
"""
import collections
import csv
import glob
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import common  # noqa: E402

DATA = common.DATA
ROOT = common.ROOT


def footprint_pins(extra):
    """{id: pins} of Parts/footprints.json, every footprints-[FG]*.json here and the --json files."""
    out = {}
    paths = [os.path.join(ROOT, 'schematical', 'schematical', 'Parts', 'footprints.json')]
    paths += sorted(glob.glob(os.path.join(DATA, 'footprints-[FG]*.json'))) + extra
    for p in paths:
        for o in json.load(open(p))['footprints']:
            out.setdefault(o['id'], o['pins'])
    return out


def load_map(extra):
    fmap = collections.defaultdict(list)
    paths = [os.path.join(DATA, 'footprint-map.tsv')] + sorted(glob.glob(os.path.join(DATA, 'footprint-map-*.tsv'))) + extra
    for path in paths:
        if os.path.exists(path):
            for r in csv.DictReader(open(path), delimiter='\t'):
                fmap[(r['letter'], r['company'], r['package'], r['supplier_package'], r['pins'], r.get('pdf') or '*')].append(r)
    return fmap


def lookup(fmap, letter, company, package, supplier, pins, pdf='*', mpn=None):
    for d in (pdf, '*'):
        for sup in (supplier, '*'):
            for p in (str(pins), '*'):
                rows = fmap.get((letter, company, package, sup, p, d), [])
                for m in rows:
                    if m.get('mpn') and mpn is not None and re.fullmatch(m['mpn'], mpn, re.I):
                        return m
                for m in rows:
                    if not m.get('mpn'):
                        return m
    return None


def line_pins():
    """{(company, package, supplier): set of (pdf file name, pin count, mpn)} over the `add` rows."""
    specs = collections.defaultdict(list)
    for s in common.read_tsv(common.SPECS):
        specs[common.spec_key(s)].append(s)
    cand = {(r['family'], r['mpn']): r for r in common.read_tsv(common.CANDIDATES)}
    out = collections.defaultdict(set)
    for v in common.read_tsv(common.VERDICTS):
        if v['verdict'] != 'add':
            continue
        c = cand[(v['family'], v['mpn'])]
        spec = specs[common.spec_key(c)][int(v['spec'].rpartition('#')[2])]
        out[(v['company'], c['package'], c['supplier_package'])].add((os.path.basename(c['pdf']), len(json.loads(spec['pinout'])), c['mpn']))
    return out


def main():
    a = sys.argv[1:]
    letter = a[0]
    listing = '--list' in a
    maps = [a[i + 1] for i, x in enumerate(a) if x == '--map']
    jsons = [a[i + 1] for i, x in enumerate(a) if x == '--json']
    pins_of = footprint_pins(jsons)
    fmap = load_map(maps)
    counts = line_pins()
    n = missing = 0
    for line in open(os.path.join(DATA, 'footprints-needed.txt')):
        if line.startswith('#') or not line.strip():
            continue
        f = line.rstrip('\n').split('\t')
        if f[0] != letter:
            continue
        n += 1
        bad = []
        seen = set()
        for pdf, pc, mpn in sorted(counts[(f[1], f[2], f[3])]):
            m = lookup(fmap, f[0], f[1], f[2], f[3], pc, pdf, mpn)
            if not m or not m['footprint']:
                msg = f'no footprint for {pc} pins ({pdf}, {mpn})'
            elif m['footprint'] not in pins_of:
                msg = f"unknown id {m['footprint']} ({pdf})"
            elif pins_of[m['footprint']] != pc:
                msg = f"{m['footprint']} has {pins_of[m['footprint']]} pins, the pinout {pc} ({pdf})"
            else:
                if listing and (pdf, pc, m['footprint']) not in seen:
                    print('ok', f[1][:3], '|', f[2], '|', f[3], '|', pc, '|', pdf, '->', m['footprint'])
                seen.add((pdf, pc, m['footprint']))
                continue
            if msg not in bad:
                bad.append(msg)
        if bad:
            missing += 1
            print('; '.join(bad) + ':', f[1], '|', f[2], '|', f[3])
    print(f'{n} lines, {missing} without a footprint')
    return 1 if missing else 0


if __name__ == '__main__':
    sys.exit(main())
