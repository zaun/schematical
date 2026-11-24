#!/usr/bin/env python3
"""plan-45-B Phase 3: the final pick, the generic representatives and footprints-needed.txt.

usage: python3 -I final.py [--write]          decide the JST rows, write generic-reps.tsv and footprints-needed.txt
       python3 -I final.py --verify-skipped   every `skip` row is named in datasheets/skipped-parts.md

The pick (§ 4). The terminal rows were judged by verdicts.py (`add`) and specs-fill.py (`skip` for a stub sheet or
a pitch / position the sheet contradicts). Here every JST row still `candidate` becomes `add` once
scheme-check.py passes (one scheme, `n` = positions) and specs-fill.py has not skipped it. No row stays
`candidate`.

generic-reps.tsv (pitch, variant, reason): for each of the six terminal pitches the `add` fixed-block variant
(class screw, spring or push-in; mounting tht) with the largest summed stock; for JST XH and PH their top-entry
through-hole variants. footprint-lines.py then draws these variants at every position count 2..12.
"""
import collections
import csv
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.abspath(os.path.join(HERE, '..'))
ROOT = os.path.abspath(os.path.join(DATA, '..', '..'))
CAND = os.path.join(DATA, 'candidates.tsv')
SPECS = os.path.join(DATA, 'specs.tsv')
REPS = os.path.join(DATA, 'generic-reps.tsv')
NEEDED = os.path.join(DATA, 'footprints-needed.txt')
SKIPPED = os.path.join(ROOT, 'datasheets', 'skipped-parts.md')
SECTION = '## Terminal blocks and JST headers (plan-45)'
PITCHES = ['3.50', '3.81', '5.00', '5.08', '7.50', '7.62']
JST_REPS = [('2.50', 'JST|XH|2.50|V|tht', 'JST XH, top entry, through hole'),
            ('2.00', 'JST|PH|2.00|V|tht', 'JST PH, top entry, through hole')]


def read(path):
    return list(csv.DictReader(open(path), delimiter='\t'))


def write(path, rows):
    cols = list(rows[0].keys())
    with open(path, 'w') as out:
        out.write('\t'.join(cols) + '\n')
        for r in rows:
            out.write('\t'.join(str(r[c]) for c in cols) + '\n')


def reps(rows):
    out = []
    add = [r for r in rows if r['state'] == 'add']
    for p in PITCHES:
        stock = collections.defaultdict(int)
        n = collections.defaultdict(int)
        for r in add:
            if (r['group'] == 'terminal' and r['class'] in ('screw', 'spring', 'push-in') and r['mounting'] == 'tht'
                    and abs(float(r['pitch']) - float(p)) < 0.001):
                stock[r['variant']] += int(r['stock'] or 0)
                n[r['variant']] += 1
        if not stock:
            sys.exit(f'no add fixed through-hole variant at {p} mm')
        best = sorted(stock, key=lambda v: (-stock[v], v))[0]
        out.append((p, best, f'fixed block, summed stock {stock[best]} of {len(stock)} add fixed through-hole variants at {p} mm'))
    for p, v, why in JST_REPS:
        if not any(r['variant'] == v for r in add):
            sys.exit(f'no add rows in {v}')
        out.append((p, v, why))
    return out


def decide(write_files):
    rows = read(CAND)
    if subprocess.run([sys.executable, '-I', '-B', os.path.join(HERE, 'scheme-check.py'), SPECS, CAND],
                      stdout=subprocess.DEVNULL).returncode != 0:
        sys.exit('scheme-check.py fails: the JST rows are not all covered by one scheme')
    n = 0
    for r in rows:
        if r['state'] == 'candidate':
            r['state'], r['reason'] = 'add', ''
            n += 1
    left = [r['mpn'] for r in rows if r['state'] == 'candidate']
    assert not left, left
    chosen = reps(rows)
    if write_files:
        write(CAND, rows)
        with open(REPS, 'w') as out:
            out.write('pitch\tvariant\treason\n')
            for p, v, why in chosen:
                out.write(f'{p}\t{v}\t{why}\n')
        with open(NEEDED, 'w') as out:
            out.write(subprocess.run([sys.executable, '-I', '-B', os.path.join(HERE, 'footprint-lines.py'), CAND, REPS],
                                     capture_output=True, text=True, check=True).stdout)
    c = collections.Counter(r['state'] for r in rows if r['state'] in ('add', 'skip'))
    print(f"{n} JST rows decided; {c['add']} add, {c['skip']} skip; {len(chosen)} generic representatives")


def verify_skipped():
    rows = read(CAND)
    text = open(SKIPPED).read()
    i = text.find(SECTION)
    if i < 0:
        sys.exit(f'{SECTION!r} is not in {SKIPPED}')
    j = text.find('\n## ', i + 1)
    section = text[i:j if j > 0 else len(text)]
    skip = [r['mpn'] for r in rows if r['state'] == 'skip']
    missing = [m for m in skip if not re.search(r'(?<![A-Za-z0-9])' + re.escape(m) + r'(?![A-Za-z0-9])', section)]
    for m in missing:
        print('missing', m)
    print(f'{len(skip)} skip rows, {len(missing)} missing from skipped-parts.md')
    sys.exit(1 if missing else 0)


if '--verify-skipped' in sys.argv:
    verify_skipped()
else:
    decide('--write' in sys.argv)
