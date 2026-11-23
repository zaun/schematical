#!/usr/bin/env python3
"""plan-47-C section 3.4: the skip list and the obsolete PDFs, from the final pick.

usage: python3 -I skip-list.py skips      the `## Inductors, fuses, relays and buzzers (plan-47)` section for
                                          datasheets/skipped-parts.md (stdout): one table row per (maker,
                                          series, reason) of the `skip` rows of candidates.tsv
       python3 -I skip-list.py obsolete   the PDFs all of whose download-set rows are `out`, reason `obsolete`:
                                          prints their `datasheets/obsolete-not-included.md` table rows and,
                                          on stderr, the `place.py` paths to delete

`out` rows (obsolete, more than two terminals, not a plain relay) are not skips: they are counted at the end of
the section. A skip row's part numbers are listed in full up to 12, else the first 12 and `and N more`.
"""
import collections
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs47 as sp  # noqa: E402

MAKER = {'bourns': 'Bourns', 'murata': 'Murata', 'wurth': 'Würth Elektronik', 'littelfuse': 'Littelfuse',
         'eaton': 'Eaton', 'schurter': 'SCHURTER', 'belfuse': 'Bel Fuse', 'te': 'TE Connectivity',
         'franjobaim': 'Franjobaim', 'panasonic': 'Panasonic', 'samesky': 'Same Sky', 'pui': 'PUI Audio',
         'mallory': 'Mallory Sonalert'}
WHAT = {'inductor': ('fixed inductor', '`inductor` / Discrete → Inductors'),
        'fuse': ('fuse', '`fuse` / Discrete → Fuses'),
        'ptc': ('PTC resettable fuse', '`fuse` / Discrete → Resettable fuses'),
        'relay': ('relay', '`relay` / Switches & Relays → Relay'),
        'buzzer': ('buzzer', '`buzzer` / Displays & Indicators → Buzzers')}
WHY = {
    "not in the datasheet's ordering table": "Not in the datasheet's ordering table (the part number is not printed in the PDF and the series' ordering code does not allow it)",
    'fact not in datasheet': "A DigiKey figure (rating, tolerance, time or range) disagrees with the datasheet; not corrected by hand",
    'fact not given': 'DigiKey leaves a figure the kind needs blank (current, trip current, response time or voltage rating)',
    'no pinout in the datasheet': 'The datasheet gives no terminal or pad drawing for this part (an application note, a catalog sheet, or another variant)',
    "no datasheet on the maker's site": "No datasheet on the maker's site",
}


def rows_of(state):
    return [r for r in sp.read(sp.CANDIDATES) if r['state'] == state and r.get('verdict')]


def skips():
    groups = collections.defaultdict(list)
    for r in rows_of('skip'):
        groups[(r['type'], r['company'], r['series'], r['reason'])].append(r['mpn'])
    print('## Inductors, fuses, relays and buzzers (plan-47)\n')
    print('In-stock parts of the top three makers of each type (DigiKey, 2026-11-16 to 2026-11-26) that passed the '
          'selection rules but not the datasheet checks of plan-47-B and C. They are still made; a later plan may '
          "add them. Their PDFs are not in the tree unless another part of the same datasheet is added.\n")
    print('| Part(s) | Device | Would have gone into | Why skipped |')
    print('|---|---|---|---|')
    for (typ, company, series, reason), mpns in sorted(groups.items(), key=lambda kv: (kv[0][0], kv[0][1], kv[0][2], kv[0][3])):
        mpns = sorted(mpns)
        shown = ', '.join(mpns[:12]) + (f', and {len(mpns) - 12} more' if len(mpns) > 12 else '')
        kind, where = WHAT[typ]
        series_text = f' {series}' if series and series != '-' else ''
        print(f"| {shown} | {MAKER[company]}{series_text} {kind} | {where} | {WHY.get(reason, reason)} |")
    out = collections.Counter((r['type'], r['reason']) for r in rows_of('out'))
    print('\n`out` rows (not skips, counted here): ' + '; '.join(f'{t} {n} ({why})' for (t, why), n in sorted(out.items())) + '.')


def obsolete():
    by_pdf = collections.defaultdict(list)
    for r in sp.read(sp.CANDIDATES):
        if sp.in_set(r) and r.get('verdict') and r['pdf']:
            by_pdf[r['pdf']].append(r)
    evidence = {r['pdf']: r['evidence'] for r in sp.read(os.path.join(sp.DATA, 'obsolete-series.tsv'))}
    for pdf, rs in sorted(by_pdf.items()):
        if all(r['verdict'] == 'out' and r['vreason'] == 'obsolete' for r in rs):
            fam = pdf.split('/')[1]
            what = {'inductor': 'Discrete → Inductors', 'fuse': 'Discrete → Fuses / Resettable fuses',
                    'relay': 'Switches & Relays → Relay', 'buzzer': 'Displays & Indicators → Buzzers'}[fam]
            series = ', '.join(sorted({r['series'] for r in rs}))
            print(f"| `{pdf.split('/', 2)[2]}` ({series}) | {fam} | {what} | {evidence.get(pdf, 'obsolete')}; "
                  f"{len(rs)} parts, every one marked not recommended for new designs by the datasheet; "
                  f"plan-47-B (obsolete-series.tsv) |")
            print(pdf, file=sys.stderr)


if __name__ == '__main__':
    {'skips': skips, 'obsolete': obsolete}[sys.argv[1]]()
