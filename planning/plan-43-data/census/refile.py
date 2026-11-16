#!/usr/bin/env python3
"""plan-43-A § 4: put the PDFs the reference-board lookup downloaded where
they belong.

usage: python3 -I planning/plan-43-data/census/refile.py [--dry-run]   (from the repository root)

Reads candidates.tsv (column `pdf`: the path at planning time) and, for every
PDF git does not track yet:
  where plan-43  -> the kind's datasheet directory (DEST), under the short
                    maker name when that directory is split by maker
  where plan-42, later -> datasheets/reference-boards/<family>/<maker dir>/
  where skipped  -> deleted
A PDF that parts of two destinations share goes to the first in the order
plan-43, plan-42, later.  The datasheets.tsv lines of the moved files move
with them (kind directories that have a tracked datasheets.tsv; else
datasheets/reference-boards/datasheets.tsv); lines of deleted files and of
plan-43 files in directories without a tracked datasheets.tsv are dropped
(their URL is in targets.tsv).  Untracked datasheets.tsv files and
directories left empty are removed.  A second run changes nothing.
"""
import csv, os, subprocess, sys
DATA = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROOT = os.path.dirname(os.path.dirname(DATA))
DRY = '--dry-run' in sys.argv
# kind -> (directory, split by maker)
DEST = {'diode': ('datasheets/diode', True), 'transistor': ('datasheets/transistor', True),
        'switch': ('datasheets/switch', True), 'connector': ('datasheets/connector-usb', False),
        'buffer': ('datasheets/buffer', False), 'oscillator': ('datasheets/osc', False),
        'rtc': ('datasheets/rtc', False), 'converter': (None, False)}
MAKER = {'Diodes Incorporated': 'diodes', 'Diotec Semiconductor': 'diotec', 'Infineon Technologies': 'infineon',
         'Nexperia USA Inc.': 'nexperia', 'onsemi': 'onsemi', 'Panjit International Inc.': 'panjit',
         'Shenzhen Slkormicro Semicon Co., Ltd.': 'slkor', 'Vishay General Semiconductor - Diodes Division': 'vishay',
         'Alpha & Omega Semiconductor Inc.': 'aos', 'GOODWORK': 'goodwork', 'MCC (Micro Commercial Components)': 'mcc',
         'Texas Instruments': 'ti', 'Vishay Siliconix': 'vishay-siliconix', 'CIT Relay and Switch': 'tactile/cit',
         'GCT': 'tactile/gct', 'CTS Electrocomponents': 'cts'}

def tracked(path):
    return subprocess.run(['git', 'ls-files', '--error-unmatch', path], cwd=ROOT, capture_output=True).returncode == 0

def main():
    rows = list(csv.DictReader(open(os.path.join(DATA, 'candidates.tsv')), delimiter='\t'))
    order = {'plan-43': 0, 'plan-42': 1, 'later': 2, 'skipped': 3}
    rows.sort(key=lambda r: order[r['where']])
    plan, counts = {}, {}
    for r in rows:
        src = r['pdf']
        if src in plan or not os.path.exists(os.path.join(ROOT, src)) or tracked(src): continue
        name = os.path.basename(src)
        if r['where'] == 'skipped': dst = None
        elif r['where'] == 'plan-43':
            base, split = DEST[r['family']]
            if base is None: base = os.path.dirname(os.path.dirname(src))   # adc / dac, as filed
            dst = os.path.join(base, MAKER[r['maker']], name) if split else os.path.join(base, name)
        else:
            dst = os.path.join('datasheets/reference-boards', r['family'], src.split('/')[2], name)
        plan[src] = dst
        key = 'deleted' if dst is None else ('kept in place' if dst == src else
              'to reference-boards' if dst.startswith('datasheets/reference-boards') else 'to a kind directory')
        counts[key] = counts.get(key, 0) + 1
    clash = [d for d in set(plan.values()) if d and list(plan.values()).count(d) > 1]
    clash += [d for s, d in plan.items() if d and d != s and os.path.exists(os.path.join(ROOT, d))]
    if clash: sys.exit('destination clash: %s' % sorted(clash))
    # datasheets.tsv lines
    lines, files = {}, {}
    for top in sorted(os.listdir(os.path.join(ROOT, 'datasheets'))):
        tsv = os.path.join('datasheets', top, 'datasheets.tsv')
        if not os.path.exists(os.path.join(ROOT, tsv)): continue
        body = open(os.path.join(ROOT, tsv)).read().split('\n')
        keep = [l for l in body if l.split('\t')[0] not in plan]
        for l in body:
            if l.split('\t')[0] in plan: lines[l.split('\t')[0]] = l
        files[tsv] = (body, keep)
    add = {}
    for src, dst in plan.items():
        if dst is None or src not in lines: continue
        top = 'reference-boards' if dst.startswith('datasheets/reference-boards') else dst.split('/')[1]
        tsv = os.path.join('datasheets', top, 'datasheets.tsv')
        if top != 'reference-boards' and not tracked(tsv): continue
        add.setdefault(tsv, []).append(dst + lines[src][len(src):])
    for key in sorted(counts): print('%4d %s' % (counts[key], key))
    print('%4d datasheets.tsv lines carried' % sum(len(v) for v in add.values()))
    if DRY: return
    for src, dst in plan.items():
        if dst is None: os.remove(os.path.join(ROOT, src))
        elif dst != src:
            os.makedirs(os.path.dirname(os.path.join(ROOT, dst)), exist_ok=True)
            os.rename(os.path.join(ROOT, src), os.path.join(ROOT, dst))
    for tsv in set(files) | set(add):
        body, keep = files.get(tsv, ([], ['path\turl\tbytes\tsha256', '']))
        rows_out = [l for l in keep if l] + sorted(add.get(tsv, []))
        path = os.path.join(ROOT, tsv)
        if len(rows_out) <= 1 and not tracked(tsv):
            if os.path.exists(path): os.remove(path)
        elif rows_out != [l for l in body if l]:
            os.makedirs(os.path.dirname(path), exist_ok=True)
            open(path, 'w').write('\n'.join(rows_out) + '\n')
    for top, dirs, names in os.walk(os.path.join(ROOT, 'datasheets'), topdown=False):
        if not dirs and not names: os.rmdir(top)
        elif not names and all(not os.path.exists(os.path.join(top, d)) for d in dirs): os.rmdir(top)

main()
