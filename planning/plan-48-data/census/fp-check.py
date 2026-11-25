#!/usr/bin/env python3
"""plan-48-C/D: does every land-pattern line have a footprint?
usage: python3 -I fp-check.py <letter> [<footprints-needed.txt>]

Joins the letter's lines of footprints-needed.txt (columns letter, company,
mounting, DigiKey Package / Case, supplier packages `a | b`, pins, …) with
planning/plan-48-data/footprint-map.tsv and the per-letter
footprint-map-<L>.tsv files (columns letter, company, package, supplier,
footprint, note, pdf: one row per land pattern; `supplier` is one supplier
device package of the line, or `*` for all of them). Every supplier package
of a line needs a map row, and every mapped id must be in footprints-C.json,
footprints-D.json or Parts/footprints.json. Prints `<n> lines, <m> without a
footprint` and each missing line or unknown id."""
import csv
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
MAP = os.path.join(DATA, 'footprint-map.tsv')


def known_ids():
    ids = set(re.findall(r'"id": *"([^"]+)"', open(os.path.join(ROOT, 'schematical', 'schematical', 'Parts',
                                                                  'footprints.json')).read()))
    for f in ('footprints-C.json', 'footprints-D.json'):
        p = os.path.join(DATA, f)
        if os.path.exists(p):
            ids |= {o['id'] for o in json.load(open(p))['footprints']}
    return ids


def load_map():
    fmap = {}
    paths = [MAP] + [os.path.join(DATA, 'footprint-map-%s.tsv' % L) for L in 'CD']
    for path in paths:
        if os.path.exists(path):
            for r in csv.DictReader(open(path, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE):
                fmap.setdefault((r['letter'], r['company'], r['package'], r['supplier']), []).append(r)
    return fmap


def lookup(fmap, letter, company, package, supplier):
    return fmap.get((letter, company, package, supplier)) or fmap.get((letter, company, package, '*'))


def pad_names(fid, cache={}):
    """The designators of a footprint's numbered pads (fp-show.py's expansion)."""
    if fid not in cache:
        import importlib.util
        spec = importlib.util.spec_from_file_location('fps', os.path.join(HERE, 'fp-show.py'))
        fps = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(fps)
        o = fps.find(fid, fps.FILES)
        cache[fid] = {p[0] for p in fps.pads(o) if p[0] not in ('NULL', 'centre', '0')}
    return cache[fid]


def designators(s):
    sys.path.insert(0, HERE)
    import importlib.util
    spec = importlib.util.spec_from_file_location('pc', os.path.join(HERE, 'pinout-check.py'))
    pc = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(pc)
    if s['scheme'] in pc.SCHEMES:
        return {str(i) for i in range(1, len(pc.SCHEMES[s['scheme']]) + 1)}
    return {d for d, _ in pc.parse_pinout(s['pinout'])}


def compare_pins(letter, fmap):
    """Every spec row's pin designators equal the pad names of its footprint."""
    bad = 0
    needed = set()
    for line in open(os.path.join(DATA, 'footprints-needed.txt'), encoding='utf-8'):
        f = line.rstrip('\n').split('\t')
        if f[0] == letter:
            needed.add((f[1], f[3], f[4]))
    for s in csv.DictReader(open(os.path.join(DATA, 'specs.tsv'), encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE):
        if (s['company'], s['package'], s['supplier_package']) not in needed or s['pinout'] == '-':
            continue
        ms = lookup(fmap, letter, s['company'], s['package'], s['supplier_package'])
        own = [m for m in (ms or []) if m['pdf'] == s['pdf']]
        use = own or [m for m in (ms or []) if m['pdf'] in ('*', '')]
        want = designators(s)
        for m in use:
            have = pad_names(m['footprint'])
            if want != have:
                bad += 1
                print('ball names differ:' if not all(d.isdigit() for d in want) else 'pin numbers differ:',
                      s['pdf'].split('/')[-1], '|', s['package'], '|', s['circuits'], 'circuits ->', m['footprint'],
                      'spec only', sorted(want - have), 'footprint only', sorted(have - want))
    return bad


def main():
    letter = sys.argv[1]
    needed = sys.argv[2] if len(sys.argv) > 2 else os.path.join(DATA, 'footprints-needed.txt')
    ids = known_ids()
    fmap = load_map()
    n = missing = 0
    for line in open(needed, encoding='utf-8'):
        if line.startswith('#') or not line.strip():
            continue
        f = line.rstrip('\n').split('\t')
        if f[0] != letter:
            continue
        n += 1
        bad = []
        for sup in [s.strip() for s in f[4].split('|')]:
            ms = lookup(fmap, f[0], f[1], f[3], sup)
            if not ms:
                bad.append('no footprint for ' + sup)
                continue
            for m in ms:
                if m['footprint'] not in ids:
                    bad.append('unknown id %s for %s' % (m['footprint'], sup))
        if bad:
            missing += 1
            print('; '.join(bad) + ':', f[1], '|', f[3], '|', f[4])
    print('%d lines, %d without a footprint' % (n, missing))
    bad = compare_pins(letter, fmap)
    print('%d spec rows whose pin designators differ from their footprint' % bad)
    return 1 if missing or bad else 0


if __name__ == '__main__':
    sys.exit(main())
