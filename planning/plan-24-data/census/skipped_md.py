#!/usr/bin/env python3
"""Write the plan-24 sections of datasheets/skipped-parts.md from parts.tsv.

usage: skipped_md.py <GROUP>...   (ADC, DAC, RTC). Replaces any earlier section of the same
title. One table row per skip reason, parts listed, devices / bases listed.
"""
import collections, csv, os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
MD = os.path.join(ROOT, 'datasheets', 'skipped-parts.md')
TITLE = {'ADC': 'TI ADCs', 'DAC': 'TI DACs', 'RTC': 'TI RTCs'}
CAT = {'ADC': (700, '`converter` / Interface → ADC'), 'DAC': (701, '`converter` / Interface → DAC'),
       'RTC': (690, '`rtc` / Interface → RTC')}
cand = {r['name']: r for r in csv.DictReader(open(os.path.join(DATA, 'candidates.tsv')), delimiter='\t')}
rows = list(csv.DictReader(open(os.path.join(DATA, 'parts.tsv')), delimiter='\t'))
text = open(MD).read()
for g in sys.argv[1:]:
    by = collections.OrderedDict()
    for r in rows:
        if r['group'] == g and r['decision'] == 'skip':
            why = re.sub(r'^not in the ordering table of \S+', 'not in its datasheet\'s ordering table', r['reason'])
            by.setdefault(why, []).append(r)
    cat, into = CAT[g]
    head = f'## {TITLE[g]} (plan-24, 2026-11-07)'
    sec = [head, '',
           f'In-stock TI parts of DigiKey category {cat} (`node scripts/digikey.js --in-stock`, '
           f'Texas Instruments; census and re-check 2026-11-07) left out when plan-24 added the '
           f'`{"rtc" if g == "RTC" else "converter"}` kind (one row per part in '
           f'`planning/plan-24-data/parts.tsv`).', '',
           '| Part(s) | Device | Would have gone into | Why skipped |', '|---|---|---|---|']
    for why, rs in sorted(by.items(), key=lambda x: -len(x[1])):
        parts = ', '.join(sorted(r['name'] for r in rs))
        devs = sorted({r['device'] or cand.get(r['name'], {}).get('base') or '—' for r in rs})
        devs = ', '.join(d for d in devs if d) or '—'
        sec.append(f'| {parts} | {devs} | {into} | {why[0].upper() + why[1:]} |')
    sec.append('')
    block = '\n'.join(sec) + '\n'
    if head in text:
        start = text.index(head)
        nxt = text.find('\n## ', start + 1)
        text = text[:start] + block + (text[nxt + 1:] if nxt >= 0 else '')
    else:
        text = text.rstrip('\n') + '\n\n' + block
    print(g, sum(len(v) for v in by.values()), 'skipped rows in', len(by), 'reasons')
open(MD, 'w').write(text)
