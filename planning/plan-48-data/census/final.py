#!/usr/bin/env python3
"""plan-48-B section 3.3: the final pick and the two lists.
usage: python3 -I final.py --pinouts            rows of the pick whose spec row has pinout `-` -> verdict-overrides.tsv (skip)
       python3 -I final.py --apply              state candidate -> add for every row with verdict add
       python3 -I final.py --write-skipped      the `## Timers and op-amps (plan-48)` section of datasheets/skipped-parts.md
       python3 -I final.py --write-obsolete     one row per PDF whose listed rows are all `datasheet status` (obsolete-not-included.md)
       python3 -I final.py --verify-skipped     every skip row is named in that section
Run after place.py and specs-skeleton.py of a settled pick (select.py is run
again only through the plan-48-A loop: it rewrites `state`)."""
import collections
import csv
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.abspath(os.path.join(HERE, '..'))
ROOT = os.path.abspath(os.path.join(DATA, '..', '..'))
CAND = os.path.join(DATA, 'candidates.tsv')
SPECS = os.path.join(DATA, 'specs.tsv')
VERD = os.path.join(DATA, 'verdicts.tsv')
OVER = os.path.join(DATA, 'verdict-overrides.tsv')
SKIPPED = os.path.join(ROOT, 'datasheets', 'skipped-parts.md')
OBSOLETE = os.path.join(ROOT, 'datasheets', 'obsolete-not-included.md')
SECTION = '## Timers and op-amps (plan-48)'
PICKED = ('candidate', 'add')
MAKER = {'ti': 'Texas Instruments', 'adi': 'Analog Devices / Maxim', 'st': 'STMicroelectronics', 'renesas': 'Renesas'}
SKIP_REASONS = ("not in the datasheet's ordering table", 'no pinout in the datasheet', 'no datasheet')


def read(p):
    return list(csv.DictReader(open(p, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE))


def write(p, rows):
    cols = list(rows[0].keys())
    with open(p, 'w', encoding='utf-8', newline='') as f:
        f.write('\t'.join(cols) + '\n')
        for r in rows:
            f.write('\t'.join(r[c] for c in cols) + '\n')


def pinouts():
    specs = {(s['type'], s['company'], s['pdf'], s['package'], s['supplier_package'], s['circuits']): s
             for s in read(SPECS)}
    have = {r['mpn'] for r in read(OVER)}
    add = []
    for r in read(CAND):
        if r['state'] not in PICKED:
            continue
        s = specs.get((r['type'], r['company'], r['pdf'], r['package'], r['supplier_package'], r['circuits']))
        if s is None:
            sys.exit('final.py: no spec row for %s' % r['mpn'])
        if s['pinout'] == '-' and r['mpn'] not in have:
            add.append((r['mpn'], s))
    with open(OVER, 'a', encoding='utf-8') as f:
        for mpn, s in add:
            f.write('%s\tskip\tno pinout in the datasheet\tspecs.tsv row %s | %s | %s circuits has pinout -\n' % (
                mpn, s['pdf'].split('/')[-1], s['package'], s['circuits']))
    print('%d rows added to verdict-overrides.tsv (pinout -)' % len(add))
    for mpn, _ in add:
        print('  ', mpn)


def apply():
    verd = {r['mpn']: r for r in read(VERD)}
    rows = read(CAND)
    n = 0
    for r in rows:
        if r['state'] in PICKED:
            if verd.get(r['mpn'], {}).get('verdict') != 'add':
                sys.exit('final.py: %s is picked without verdict add' % r['mpn'])
            r['state'] = 'add'
            n += 1
    write(CAND, rows)
    c = collections.Counter((r['type'], r['company']) for r in rows if r['state'] == 'add')
    print('%d rows are state add: %s' % (n, sorted(c.items())))


def skip_rows():
    out = []
    for r in read(CAND):
        if r['state'] == 'out' and r['reason'] in SKIP_REASONS:
            out.append(r)
    return out


def write_skipped():
    groups = collections.defaultdict(list)
    for r in skip_rows():
        if r['reason'] == 'no datasheet':
            why = "no datasheet on the maker's site (the DigiKey link is a Rochester Electronics copy, the guessed ti.com / analog.com link answers 404, or there is no link)"
        elif r['reason'] == 'no pinout in the datasheet':
            why = "the datasheet shows no pinout for the package, or the device is not a plain op-amp (no IN+ / IN- / OUT)"
        else:
            why = "the part number is not in the datasheet's ordering table (plan-48-A § 4 rule 2)"
        groups[(r['company'], r['type'], why)].append(r['mpn'])
    lines = [SECTION, '',
             'In-stock Active / Last Time Buy 555-type timers and common op-amps of the top three',
             'makers of each type at DigiKey (timers: Texas Instruments, Analog Devices / Maxim,',
             'Renesas; op-amps: Texas Instruments, Analog Devices / Maxim, STMicroelectronics) that',
             "were picked but left out for their datasheet. The next most-stocked part of the maker",
             'took the place of an op-amp. Written by `planning/plan-48-data/census/final.py --write-skipped`.',
             '', '| Part(s) | Device | Would have gone into | Why skipped |', '|---|---|---|---|']
    for (company, typ, why), mpns in sorted(groups.items()):
        kind = '`timer` / Analog → Timers' if typ == 'timer' else '`opamp` / Analog → Op-amps'
        dev = '%s %s' % (MAKER[company], '555-type timers' if typ == 'timer' else 'op-amps')
        lines.append('| %s | %s | %s | %s |' % (', '.join(sorted(mpns)), dev, kind, why))
    text = open(SKIPPED, encoding='utf-8').read()
    at = text.find(SECTION)
    if at >= 0:
        end = text.find('\n## ', at + len(SECTION))
        text = text[:at] + text[end + 1:] if end >= 0 else text[:at]
    text = text.rstrip('\n') + '\n\n' + '\n'.join(lines) + '\n'
    open(SKIPPED, 'w', encoding='utf-8').write(text)
    print('%d skip rows in %d table rows' % (sum(len(v) for v in groups.values()), len(groups)))


def verify_skipped():
    text = open(SKIPPED, encoding='utf-8').read()
    at = text.find(SECTION)
    section = text[at:] if at >= 0 else ''
    end = section.find('\n## ', len(SECTION))
    section = section[:end] if end >= 0 else section
    rows = skip_rows()
    missing = [r['mpn'] for r in rows if r['mpn'] not in section]
    for m in missing:
        print('missing', m)
    print('%d skip rows, %d missing from skipped-parts.md' % (len(rows), len(missing)))
    return 1 if missing else 0


def write_obsolete():
    by_pdf = collections.defaultdict(list)
    for r in read(CAND):
        if r['state'] == 'out' and r['reason'] == 'datasheet status':
            by_pdf[(r['type'], r['company'], r['final'])].append(r)
    alive = {r['final'] for r in read(CAND) if r['state'] in PICKED}
    text = open(OBSOLETE, encoding='utf-8').read()
    n = 0
    for (typ, company, final), rs in sorted(by_pdf.items()):
        if final in alive:
            continue
        name = final.rsplit('/', 1)[-1]
        grp = 'Analog → Timers' if typ == 'timer' else 'Analog → Op-amps'
        line = '| `%s` | %s | %s | %d parts, every one marked obsolete or not recommended for new designs by the datasheet; %s; plan-48-B |\n' % (
            name, typ, grp, len(rs), final)
        if name not in text:
            text = text.rstrip('\n') + '\n' + line
            n += 1
    open(OBSOLETE, 'w', encoding='utf-8').write(text)
    print('%d PDFs added to obsolete-not-included.md' % n)


if __name__ == '__main__':
    a = sys.argv[1] if len(sys.argv) > 1 else ''
    if a == '--pinouts':
        pinouts()
    elif a == '--apply':
        apply()
    elif a == '--write-skipped':
        write_skipped()
    elif a == '--write-obsolete':
        write_obsolete()
    elif a == '--verify-skipped':
        sys.exit(verify_skipped())
    else:
        sys.exit(__doc__)
