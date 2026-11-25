#!/usr/bin/env python3
"""plan-55-C § 3.2: a verdict for every candidate row, and the skipped-parts list.

usage: python3 -I verdicts.py [--family F]... [--company C]... [--report]        write verdicts.tsv, print the counts
       python3 -I verdicts.py --write-skipped                                    the `## Power switches (plan-55)` section
       python3 -I verdicts.py --verify-skipped [--family F]... [--company C]...  every skip row is named in it

verdicts.tsv: mpn, family, company, verdict (`add` or `skip`), reason, spec (the `specs.tsv` row: the key joined by
`|` plus `#<k>`, the row's index among the rows of the key), flags (`automotive`, `military`, `enhanced`, comma
separated), pdf. Rules, in order (C § 3.2); a part is `add` only when every one holds:
  1. no PDF (`state` out, reason `no datasheet`)  -> skip, `no datasheet on the maker's site (<host of its link>)`
  2. the spec row of its key (the first whose `ordering` regex fully matches the part number; else the only row
     of the key) is a skip row -> skip, the row's reason (`skip` column: control, select logic, external pass
     element, more than 4 switches, negative supply, no pin table)
  3. the part number is neither printed verbatim in the PDF (`hit` 1) nor fully matched by the row's `ordering`
     regex -> skip, `not in the datasheet's ordering table`
  4. else add.
Rows `out` for another reason (no stock, stock check, duplicate) have no verdict. A verdict read by hand that these
rules get wrong goes in planning/plan-55-data/verdict-overrides.tsv (mpn, family, verdict, reason).
The flags: `automotive` for DigiKey Grade Automotive or a `-Q1` / `Q1` part number, `military` for 5962- and SMD
numbers, `enhanced` for `-EP`.
"""
import collections
import os
import re
import sys
from urllib.parse import urlparse

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

SKIPPED = os.path.join(common.ROOT, 'datasheets', 'skipped-parts.md')
OVERRIDES = os.path.join(common.DATA, 'verdict-overrides.tsv')
SECTION = '## Power switches (plan-55)'
FAMILY_NAME = {'ideal': ('integrated ideal diode', 'Ideal Diodes'),
               'load': ('load switch', 'Load Switches'),
               'limited': ('current-limited / USB power switch', 'Current-Limited')}
COMPANY_NAME = {'ti': 'Texas Instruments', 'analog-devices': 'Analog Devices',
                'monolithic-power-systems': 'Monolithic Power Systems', 'onsemi': 'onsemi',
                'microchip': 'Microchip Technology', 'diodes': 'Diodes Incorporated'}
COLS = ['mpn', 'family', 'company', 'verdict', 'reason', 'spec', 'flags', 'pdf']


def flags_of(r):
    out = []
    m = r['mpn'].upper()
    if r['grade'] == 'Automotive' or re.search(r'-Q1|Q1$|Q1[A-Z]?$', m):
        out.append('automotive')
    if m.startswith('5962') or m.startswith('SMD'):
        out.append('military')
    if '-EP' in m:
        out.append('enhanced')
    return ','.join(out)


def overrides():
    if not os.path.exists(OVERRIDES):
        return {}
    return {(r['mpn'], r['family']): (r['verdict'], r['reason']) for r in common.read_tsv(OVERRIDES)}


def load_specs():
    by_key = collections.defaultdict(list)
    for r in common.read_tsv(common.SPECS):
        by_key[common.spec_key(r)].append(r)
    return by_key


def pick_spec(rows, mpn):
    """(index, row) of the spec row of a key that the part belongs to, or None."""
    for k, s in enumerate(rows):
        if s['ordering'] and re.fullmatch(s['ordering'], mpn, re.I):
            return k, s
    plain = [(k, s) for k, s in enumerate(rows) if not s['ordering']]
    if len(plain) == 1:
        return plain[0]
    if len(rows) == 1:
        return 0, rows[0]
    return None


def verdict(r, by_key, manual):
    if (r['mpn'], r['family']) in manual:
        return manual[(r['mpn'], r['family'])] + ('',)
    if r['state'] == 'out' and r['reason'] == 'no datasheet':
        host = urlparse(r['datasheet_url']).netloc if r['datasheet_url'] else ''
        return 'skip', f"no datasheet on the maker's site ({host or 'none'})", ''
    rows = by_key.get(common.spec_key(r))
    if not rows:
        sys.exit(f"verdicts.py: no spec row for {common.spec_key(r)} ({r['mpn']})")
    picked = pick_spec(rows, r['mpn'])
    if picked is None:
        # several rows, none matches: the part is in no ordering table of the PDF's rows
        if r['hit'] == '1':
            sys.exit(f"verdicts.py: {r['mpn']}: {len(rows)} spec rows of its key, none selects it (add an `ordering`)")
        return 'skip', "not in the datasheet's ordering table", ''
    k, s = picked
    spec = '|'.join(common.spec_key(s)) + f'#{k}'
    if not s['skip'] and not s['pinout']:
        sys.exit(f"verdicts.py: the spec row of {r['mpn']} is not read (no pinout and no skip): {spec}")
    if s['skip']:
        return 'skip', s['skip'], spec
    matched = bool(s['ordering']) and re.fullmatch(s['ordering'], r['mpn'], re.I) is not None
    if r['hit'] != '1' and not matched:
        return 'skip', "not in the datasheet's ordering table", spec
    return 'add', '', spec


def select(args):
    fams = {args[i + 1] for i, a in enumerate(args) if a == '--family'}
    cos = {args[i + 1] for i, a in enumerate(args) if a == '--company'}
    return [r for r in common.read_tsv(common.CANDIDATES)
            if (not fams or r['family'] in fams) and (not cos or r['company'] in cos)
            and (common.eligible(r) or (r['state'] == 'out' and r['reason'] == 'no datasheet'))]


def all_verdicts():
    by_key = load_specs()
    manual = overrides()
    out = []
    for r in select([]):
        v, why, spec = verdict(r, by_key, manual)
        out.append({'mpn': r['mpn'], 'family': r['family'], 'company': r['company'], 'verdict': v, 'reason': why,
                    'spec': spec, 'flags': flags_of(r), 'pdf': r['pdf'], 'stock': r['stock'], 'series': r['series'],
                    'package': r['package']})
    return out


def write_and_report(args):
    rows = all_verdicts()
    common.write_tsv(common.VERDICTS, COLS, rows)
    fams = {args[i + 1] for i, a in enumerate(args) if a == '--family'}
    cos = {args[i + 1] for i, a in enumerate(args) if a == '--company'}
    shown = [r for r in rows if (not fams or r['family'] in fams) and (not cos or r['company'] in cos)]
    counts = collections.Counter((r['family'], r['company'], r['verdict'], r['reason']) for r in shown)
    for k in sorted(counts):
        print(counts[k], *k)
    if '--report' in args:
        for fam in common.FAMILIES:
            adds = [r for r in shown if r['family'] == fam and r['verdict'] == 'add']
            print(f'{fam}: {len(adds)} add, {sum(1 for r in shown if r["family"] == fam and r["verdict"] == "skip")} skip;'
                  f' by company {dict(collections.Counter(r["company"] for r in adds))}')
        print('add pdfs:', len({r['pdf'] for r in shown if r['verdict'] == 'add'}))
        print('rows without a verdict:', sum(1 for r in shown if r['verdict'] not in ('add', 'skip')))


def skip_groups(rows):
    groups = collections.defaultdict(list)
    for r in rows:
        if r['verdict'] == 'skip':
            groups[(r['company'], r['family'], r['reason'].split(' (')[0] if r['reason'].startswith('no datasheet') else r['reason'],
                    os.path.basename(r['pdf']) if r['pdf'] else '')].append(r['mpn'])
    return groups


def write_skipped():
    rows = all_verdicts()
    groups = skip_groups(rows)
    lines = [SECTION, '',
             'In-stock Active power switches (integrated ideal diodes, load switches, current-limited and USB',
             'power switches) of the makers of plan-55 (Texas Instruments, Analog Devices, Monolithic Power',
             'Systems, onsemi, Microchip Technology, Diodes Incorporated, with the makers they bought) that were',
             'looked at for the catalog and left out: the datasheet is not on the maker\'s site, does not print the',
             'part number, or the part is one the `powerswitch` kind cannot express (an I2C / SPI / PWM control, a',
             'power mux with select logic, an external pass element, more than four switches). Written by',
             '`planning/plan-55-data/census/verdicts.py --write-skipped`.',
             '', '| Part(s) | Device | Would have gone into | Why skipped |', '|---|---|---|---|']
    why_text = {
        'control': 'a control other than an enable pin (I²C, SPI, PWM, an analog level or a multi-level setting): outside the `powerswitch` kind',
        'select logic': 'two or more inputs with a select, priority or mode pin (a power mux): outside the `powerswitch` kind',
        'external pass element': 'a gate pin for an external pass transistor: outside the `powerswitch` kind',
        'more than 4 switches': 'more than four switches: outside the `powerswitch` kind',
        'negative supply': 'a supply below ground: outside the `powerswitch` kind',
        'no pin table': 'the datasheet has no pin table or figure the pinout can be read from',
        "not in the datasheet's ordering table": "the part number is not printed in the datasheet (or its ordering table): no pinout can be tied to it",
        'no datasheet': "no datasheet on the maker's site that a script can fetch (the DigiKey link is a distributor copy, a missing document or a page behind a bot check)",
    }
    for (company, fam, why, pdf), mpns in sorted(groups.items()):
        name, leaf = FAMILY_NAME[fam]
        doc = f', `{pdf}`' if pdf else ''
        lines.append(f"| {', '.join(sorted(mpns))} | {COMPANY_NAME[company]} {name}{doc} | `powerswitch` / "
                     f"Power → Power Switches → {leaf} | {why_text.get(why, why)} |")
    text = open(SKIPPED).read()
    at = text.find(SECTION)
    if at >= 0:
        end = text.find('\n## ', at + len(SECTION))
        text = text[:at] + text[end + 1:] if end >= 0 else text[:at]
    text = text.rstrip('\n') + '\n\n' + '\n'.join(lines) + '\n'
    open(SKIPPED, 'w').write(text)
    print(sum(len(v) for v in groups.values()), 'skip rows in', len(groups), 'table rows')


def verify_skipped(args):
    rows = [r for r in all_verdicts()]
    fams = {args[i + 1] for i, a in enumerate(args) if a == '--family'}
    cos = {args[i + 1] for i, a in enumerate(args) if a == '--company'}
    text = open(SKIPPED).read()
    at = text.find(SECTION)
    section = text[at:] if at >= 0 else ''
    end = section.find('\n## ', len(SECTION))
    section = section[:end] if end >= 0 else section
    missing = [r['mpn'] for r in rows if r['verdict'] == 'skip' and (not fams or r['family'] in fams)
               and (not cos or r['company'] in cos) and r['mpn'] not in section]
    for m in missing:
        print('missing', m)
    print(f'{len(missing)} skip rows missing from skipped-parts.md')
    return 1 if missing else 0


if __name__ == '__main__':
    a = sys.argv[1:]
    if '--write-skipped' in a:
        write_skipped()
    elif '--verify-skipped' in a:
        sys.exit(verify_skipped(a))
    else:
        write_and_report(a)
