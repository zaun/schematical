#!/usr/bin/env python3
"""plan-45-B Phase 1: write the JST ordering schemes into specs.tsv.

usage: python3 -I jst-schemes.py <specs.tsv>

One regular expression per (JST PDF, variant) over the datasheet's numbering rule, with the position
count as the group `n` (scheme-check.py full-matches it against the part number). Read from the PDFs
(`pdftotext -layout`, 2026-11-24): the "Model number allocation" page of each (the header's model
number: header type B top / S side, circuits, series, clinch, boss, colour, packaging, suction tape)
and the part-number tables (pages 2-5). The `note` column records where each rule is printed.
A row whose scheme is already there keeps it unless --force.
"""
import csv
import sys

# (variant, pdf) -> (scheme, note)
SCHEMES = {
    ('JST|EH|2.50|H|tht', 'eEH.pdf'): (r'S(?P<n>\d+)B-EH', 'EH side entry S*B-EH: tables p2-4, allocation p6 ("S…Side entry type", no suffix)'),
    ('JST|EH|2.50|V|tht', 'eEH.pdf'): (r'B(?P<n>\d+)B-EH-A', 'EH top entry B*B-EH-A (Type A): tables p2-4, allocation p6'),
    ('JST|GH|1.25|H|smd', 'eGH.pdf'): (r'SM(?P<n>\d+)B-GHS-TB', 'GH side entry SM*B-GHS-TB: table p3-5, allocation p6 (SM…SMT side entry)'),
    ('JST|GH|1.25|V|smd', 'eGH.pdf'): (r'BM(?P<n>\d+)B-GHS-TBT', 'GH top entry BM*B-GHS-TBT: tables p3-5 print the T (suction tape: T…Yes, allocation p6)'),
    ('JST|PH|2.00|H|smd', 'ePH.pdf'): (r'S(?P<n>\d+)B-PH-SM4-TB', 'PH SMT side entry S*B-PH-SM4-TB: table p4, allocation p5'),
    ('JST|PH|2.00|H|tht', 'ePH.pdf'): (r'S(?P<n>\d+)B-PH-K-S', 'PH through-hole side entry S*B-PH-K-S (clinched, colour S): table p3, allocation p5'),
    ('JST|PH|2.00|V|smd', 'ePH.pdf'): (r'B(?P<n>\d+)B-PH-SM4-TBT?', 'PH SMT top entry B*B-PH-SM4-TB, and with suction tape B*B-PH-SM4-TBT ("T…Yes", allocation p5; the T form is not printed in the tables p4, the TB form is)'),
    ('JST|PH|2.00|V|tht', 'ePH.pdf'): (r'B(?P<n>\d+)B-PH-K-S', 'PH through-hole top entry B*B-PH-K-S: table p3, allocation p5'),
    ('JST|SH/SR|1.00|H|smd', 'eSH.pdf'): (r'SM(?P<n>\d+)B-SRSS-TB', 'SH/SR SMT side entry SM*B-SRSS-TB: tables p2-3, allocation p4'),
    ('JST|SH/SR|1.00|H|smd', 'eSR_SZ.pdf'): (r'SM(?P<n>\d+)B-SRSS-TB', 'SR SMT side entry SM*B-SRSS-TB: tables p2-3, allocation p4'),
    ('JST|SH/SR|1.00|V|smd', 'eSH.pdf'): (r'BM(?P<n>\d+)B-SRSS-TB', 'SH/SR SMT top entry BM*B-SRSS-TB: tables p2-3, allocation p4'),
    ('JST|SH/SR|1.00|V|smd', 'eSR_SZ-new.pdf'): (r'BM(?P<n>\d+)B-SRSS-TB', 'SR SMT top entry BM*B-SRSS-TB (newer revision): tables p2-3, allocation p3'),
    ('JST|SH/SR|1.00|V|smd', 'eSR_SZ.pdf'): (r'BM(?P<n>\d+)B-SRSS-TBT?', 'SR SMT top entry BM*B-SRSS-TB, and with suction tape BM*B-SRSS-TBT ("T…Yes", allocation p4; the TB forms are printed in the tables p2-3)'),
    ('JST|VH|3.96|H|tht', 'eVH.pdf'): (r'B(?P<n>\d+)PS-VH', 'VH side entry without stopper B*PS-VH: tables p2-4 (the S*P-VH "with stopper" column is not a candidate)'),
    ('JST|VH|3.96|V|tht', 'eVH.pdf'): (r'B(?P<n>\d+)P-VH(?:-FB-B|-B)?', 'VH top entry B*P-VH, B*P-VH-B (colour) and B*P-VH-FB-B (flange, the table p4 prints "B2P-VH-FB-BNote2"): tables p2-4, allocation p5-6'),
    ('JST|XH|2.50|H|smd', 'eXH.pdf'): (r'S(?P<n>\d+)B-XH-SM4-TB', 'XH SMT side entry S*B-XH-SM4-TB: table p6, allocation p9'),
    ('JST|XH|2.50|H|tht', 'eXH.pdf'): (r'S(?P<n>\d+)B-XH-A(?:-1)?', 'XH side entry S*B-XH-A and S*B-XH-A-1 (both printed in the table p5, circuits 2-15): allocation p9'),
    ('JST|XH|2.50|V|tht', 'eXH.pdf'): (r'B(?P<n>\d+)B-XH-A(?:M)?', 'XH top entry B*B-XH-A, with boss B*B-XH-AM ("M…With boss", allocation p9): table p3-4'),
    ('JST|ZH|1.50|H|smd', 'eZH.pdf'): (r'S(?P<n>\d+)B-ZR-SM4A-TF', 'ZH SMT SM4 side entry S*B-ZR-SM4A-TF: table p5, allocation p6'),
    ('JST|ZH|1.50|H|tht', 'eZH.pdf'): (r'S(?P<n>\d+)B-ZR(?:-3\.4)?', 'ZH side entry S*B-ZR and S*B-ZR-3.4 (soldering length 3.4 mm): tables p3-4, allocation p6'),
    ('JST|ZH|1.50|V|smd', 'eZH.pdf'): (r'B(?P<n>\d+)B-ZR-SM4-TFT?', 'ZH SMT SM4 top entry B*B-ZR-SM4-TF, and with suction tape B*B-ZR-SM4-TFT ("T…Yes", allocation p6)'),
    ('JST|ZH|1.50|V|tht', 'eZH.pdf'): (r'B(?P<n>\d+)B-ZR(?:-3\.4)?', 'ZH top entry B*B-ZR and B*B-ZR-3.4: tables p3-4, allocation p6'),
}


def main():
    path = sys.argv[1]
    force = '--force' in sys.argv
    rows = list(csv.DictReader(open(path), delimiter='\t'))
    cols = list(rows[0].keys())
    done = 0
    seen = set()
    for r in rows:
        if r['maker'] != 'JST':
            continue
        key = (r['variant'], r['pdf'].split('/')[-1])
        if key not in SCHEMES:
            sys.exit(f'no scheme for {key}')
        seen.add(key)
        if r['scheme'] and not force:
            continue
        r['scheme'], r['note'] = SCHEMES[key]
        done += 1
    if set(SCHEMES) - seen:
        sys.exit(f'schemes without a spec row: {sorted(set(SCHEMES) - seen)}')
    with open(path, 'w') as out:
        out.write('\t'.join(cols) + '\n')
        for r in rows:
            out.write('\t'.join(r[c] for c in cols) + '\n')
    print(f'{done} JST spec rows filled, {len(seen)} schemes')


main()
