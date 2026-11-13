#!/usr/bin/env python3
# plan-37-A Phases 3-4: ordering-table verdicts (plan-37-A § 3 rules 4-5).
# usage: python3 -I verdicts.py              fill ordering / verdict / grade
#        python3 -I verdicts.py --verify-records
# Run from the repository root. Reads and rewrites
# planning/plan-37-data/candidates.tsv in place: every in-scope row with a
# `pdf` and no `out` / `skip` verdict gets
#   ordering  the ordering-table line that lists it (whitespace collapsed),
#             or the reason it is out
#   verdict   add | out
#   grade     military | automotive | empty (from the part number)
# TI: the orderable is a PACKAGING INFORMATION row (first token); its Status
# column ACTIVE / LIFEBUY stays, OBSOLETE / NRND / PREVIEW and a sheet marked
# "not recommended for new designs" are out. onsemi: the part number is a
# token of an ORDERING INFORMATION line. Nexperia: the type number (part
# number up to the first ",", packing letter J / X dropped) is the first
# token of a "Table 1. Ordering information" row. Status for onsemi and
# Nexperia is DigiKey's (rule 1).
import csv, re, subprocess, sys
from pathlib import Path

root = Path('.')
cand = root / 'planning/plan-37-data/candidates.tsv'
TI, ON, NX = 'Texas Instruments', 'onsemi', 'Nexperia USA Inc.'
_text = {}


def text(pdf):
    if pdf not in _text:
        r = subprocess.run(['pdftotext', '-layout', str(root / 'datasheets' / pdf), '-'],
                           capture_output=True, text=True)
        _text[pdf] = r.stdout.replace('−', '-').replace('–', '-').replace('‐', '-')
    return _text[pdf]


def collapse(s):
    return re.sub(r'\s+', ' ', s).strip()


def pages(pdf):
    return text(pdf).split('\f')


def page_of(pdf, line):
    for i, p in enumerate(pages(pdf)):
        if line in p:
            return i + 1
    return 0


def ti_row(pdf, mpn):
    """(line, status) of mpn's PACKAGING INFORMATION row, or (None, None)."""
    t = text(pdf)
    start = t.find('PACKAGING INFORMATION')
    if start < 0:
        start = t.find('Orderable part number')
    if start < 0:
        start = 0
    for line in t[start:].splitlines():
        tok = line.split()
        if tok and tok[0] in (mpn, mpn + '.A', mpn + '.B'):
            if tok[0] != mpn:
                continue
            m = re.search(r'\b(ACTIVE|LIFEBUY|LAST|NRND|OBSOLETE|PREVIEW)\b', line, re.I)
            if m:
                return line, m.group(1).upper()
    return None, None


def onsemi_row(pdf, mpn):
    line = onsemi_exact(pdf, mpn)
    # Fairchild-heritage sheets: "Specify by appending the suffix letter X" (tape and reel)
    if line is None and mpn.endswith('X') and re.search(r'appending\s+the\s+suffix\s+letter\s+.X', text(pdf)):
        line = onsemi_exact(pdf, mpn[:-1])
        if line is not None:
            line = line + ' (+ suffix X: tape and reel)'
    return line


def onsemi_exact(pdf, mpn):
    t = text(pdf)
    start = t.upper().find('ORDERING INFORMATION')
    pat = re.compile(r'(^|\s)' + re.escape(mpn) + r'(?=$|[\s*(,†‡])')
    for part in ([t[start:]] if start >= 0 else []) + [t]:
        for line in part.splitlines():
            if pat.search(line):
                return line
    return None


def nx_type(mpn):
    """74HC173D,652 -> 74HC173D; 74HC273D-Q100J -> 74HC273D-Q100; 74LVC574APWJ -> 74LVC574APW."""
    if ',' in mpn:
        return mpn.split(',')[0]
    return mpn[:-1] if mpn[-1:] in ('J', 'X') else mpn


def nexperia_row(pdf, mpn):
    t = text(pdf)
    start = t.find('Ordering information')
    types = [mpn.split(',')[0]]
    if nx_type(mpn) not in types:
        types.append(nx_type(mpn))
    body = t[start:] if start >= 0 else t
    for typ in types:
        for line in body.splitlines():
            tok = line.split()
            if tok and tok[0] == typ:
                return line, typ
    return None, None


def grade(mpn):
    if (mpn.startswith(('SNJ54', '5962-', 'JM38510/', '8400102')) or
            re.match(r'^(SN54|CD54)', mpn)):
        return 'military'
    if re.search(r'-Q1(\b|[^0-9]|$)', mpn) or '-Q100' in mpn or mpn.startswith('NLV'):
        return 'automotive'
    return ''


def nrnd_sheet(pdf):
    first = '\n'.join(pages(pdf)[:2]).lower()
    return 'not recommended for new designs' in first


def run():
    rows = list(csv.DictReader(cand.open(), delimiter='\t'))
    cols = list(rows[0].keys())
    counts = {}
    for r in rows:
        if r['scope'] != 'in':
            continue
        r['grade'] = grade(r['mpn'])
        if r['verdict'] in ('skip',) or (r['verdict'] == 'out' and r['ordering'] == 'stock check'):
            continue
        r['verdict'], r['ordering'] = '', ''
        pdf = r['pdf']
        if not pdf:
            raise SystemExit(f"verdicts.py: {r['mpn']} has no pdf")
        if r['maker'] == TI:
            line, status = ti_row(pdf, r['mpn'])
            if line is None:
                r['verdict'], r['ordering'] = 'out', 'not in ordering table'
            elif status in ('ACTIVE', 'LIFEBUY', 'LAST'):   # Last = Last Time Buy
                if nrnd_sheet(pdf):
                    r['verdict'], r['ordering'] = 'out', 'nrnd (sheet marked not recommended for new designs)'
                else:
                    r['verdict'], r['ordering'] = 'add', collapse(line)
            else:
                r['verdict'] = 'out'
                r['ordering'] = {'OBSOLETE': 'obsolete', 'NRND': 'nrnd'}.get(status, status.lower()) + \
                    ': ' + collapse(line)
        elif r['maker'] == ON:
            line = onsemi_row(pdf, r['mpn'])
            if line is None:
                r['verdict'], r['ordering'] = 'out', 'not in ordering table'
            else:
                r['verdict'], r['ordering'] = 'add', collapse(line)
        else:
            line, typ = nexperia_row(pdf, r['mpn'])
            if line is None:
                r['verdict'], r['ordering'] = 'out', 'not in ordering table'
            else:
                r['verdict'], r['ordering'] = 'add', collapse(line)
        counts[(r['maker'], r['verdict'])] = counts.get((r['maker'], r['verdict']), 0) + 1
    with cand.open('w', newline='') as f:
        w = csv.DictWriter(f, fieldnames=cols, delimiter='\t', lineterminator='\n')
        w.writeheader()
        w.writerows(rows)
    for k in sorted(counts):
        print(k, counts[k])


def verify_records():
    rows = [r for r in csv.DictReader(cand.open(), delimiter='\t') if r['scope'] == 'in']
    sk = (root / 'datasheets/skipped-parts.md').read_text()
    sec = sk[sk.find('## Registers (plan-37)'):]
    nxt = sec.find('\n## ', 5)
    sec = sec if nxt < 0 else sec[:nxt]
    missing = [r['mpn'] for r in rows if r['verdict'] == 'skip' and r['mpn'] not in sec]
    by_pdf = {}
    for r in rows:
        if r['pdf']:
            by_pdf.setdefault(r['pdf'], []).append(r)
    obsolete_left = []
    for pdf, rs in by_pdf.items():
        if all(r['verdict'] == 'out' and r['ordering'].startswith(('obsolete', 'nrnd')) for r in rs):
            if (root / 'datasheets' / pdf).exists():
                obsolete_left.append(pdf)
    for m in missing:
        print('skip row missing:', m)
    for p in obsolete_left:
        print('obsolete PDF left:', p)
    print(f'{len(missing)} skip rows missing from skipped-parts.md, {len(obsolete_left)} obsolete PDFs left')
    return 1 if missing or obsolete_left else 0


def write_skipped():
    """Writes (or replaces) the "## Registers (plan-37)" section of
    datasheets/skipped-parts.md: one row per (device, skip reason, maker)."""
    rows = [r for r in csv.DictReader(cand.open(), delimiter='\t') if r['scope'] == 'in' and r['verdict'] == 'skip']
    groups = {}
    for r in rows:
        groups.setdefault((r['device'], r['kind'], r['ordering'], MAKER_SHORT[r['maker']]), []).append(r['mpn'])
    where = {'register': '`register` / Logic → Registers', 'dff': '`dff` / Logic → Flip-flops & latches',
             'mux': '`mux` / Logic → Registers'}
    why = {'no TI product page': 'no TI product folder (ti.com/product/<folder> 404 for every candidate folder, plan-37-A § 3 rule 3)',
           'only a Rochester copy': 'only a Rochester copy of the datasheet (no ti.com PDF)',
           'no Nexperia data sheet': 'no Nexperia data sheet (DigiKey links a Rochester copy; no assets.nexperia.com PDF found)',
           'no onsemi data sheet': 'no onsemi data sheet (onsemi.com answers "notFound" for it; DigiKey links a Rochester copy or the missing sheet)'}
    lines = ['## Registers (plan-37)', '',
             'In-stock Active / Last Time Buy parts of the plan-37 register devices (DigiKey census of',
             '2026-11-13: `planning/plan-37-data/census/run.sh`, category 706 Flip Flops, makers Texas',
             'Instruments, onsemi, Nexperia; `planning/plan-37-data/candidates.tsv`) left out for want of a',
             'vendor datasheet. Rows missing from their datasheet\'s ordering table, or obsolete there, are',
             '`out` in `candidates.tsv`. Written by `planning/plan-37-data/census/verdicts.py --skipped`.', '',
             '| Part(s) | Maker | Device | Would have gone into | Why skipped |', '|---|---|---|---|---|']
    for (dev, kind, reason, maker), names in sorted(groups.items()):
        lines.append('| %s | %s | %s | %s | %s |' % (', '.join(sorted(names)), maker, dev, where[kind],
                                                    why.get(reason, reason)))
    section = '\n'.join(lines) + '\n'
    path = root / 'datasheets/skipped-parts.md'
    t = path.read_text()
    i = t.find('## Registers (plan-37)')
    if i >= 0:
        j = t.find('\n## ', i + 5)
        t = t[:i] + section + ('' if j < 0 else '\n' + t[j + 1:])
    else:
        t = t.rstrip('\n') + '\n\n' + section
    path.write_text(t)
    print('%d skip rows in %d table rows' % (len(rows), len(groups)))


MAKER_SHORT = {TI: 'TI', ON: 'onsemi', NX: 'Nexperia'}


if __name__ == '__main__':
    if sys.argv[1:] == ['--skipped']:
        write_skipped()
        sys.exit(0)
    if sys.argv[1:] == ['--verify-records']:
        sys.exit(verify_records())
    run()
