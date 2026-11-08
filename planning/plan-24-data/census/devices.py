#!/usr/bin/env python3
"""Group matched rows into devices (TI base products) and datasheet clusters.

usage: devices.py rows-<G>.tsv > devices-<G>.tsv
Out: cluster  device  pdfs  codes  parts
A device is the DigiKey base product; rows with no base take the longest base of the
same datasheet that prefixes their name, else the name up to its temperature-grade letter.
A cluster is the set of datasheets linked by shared devices (worked on together).
"""
import collections, csv, re, sys

rows = [r for r in csv.DictReader(open(sys.argv[1]), delimiter='\t') if r['match'] in ('own', 'other')]
bases_by_pdf = collections.defaultdict(set)
for r in rows:
    if r['base']:
        bases_by_pdf[r['pdf']].add(r['base'])

# rows with no DigiKey base whose name the rules below split wrongly (checked by hand
# against the datasheet's ordering table)
OVERRIDE = {'ADC121S101QIMF/NOPB': 'ADC121S101', 'ADC121S655CIMM/NOPB': 'ADC121S655',
            'ADC128S052QCMT/NOPB': 'ADC128S052', 'ADC141S628QIMM/NOPB': 'ADC141S628',
            'ADC141S628QIMMX/NOPB': 'ADC141S628', 'ADS1288IRHBR': 'ADS1288',
            'ADS1288IRHBT': 'ADS1288', 'ADS8681WRUMT': 'ADS8681W', 'ADS9817RSHT': 'ADS9817',
            '5962-0051101NXD': 'THS1401',
            # misfiled by DigiKey's base product (spec writers, plan-24-A Corrections)
            'ADS1014QNKSRQ1': 'ADS1014', 'ADS1015QNKSRQ1': 'ADS1015', 'TLC3545IDR': 'TLC3545',
            # two revisions with different register maps or timing under one base product
            'ADS114S06BIPBS': 'ADS114S06B', 'ADS114S06BIPBSR': 'ADS114S06B',
            'ADS114S06BIRHBT': 'ADS114S06B', 'ADS114S08BIPBS': 'ADS114S08B',
            'ADS114S08BIPBSR': 'ADS114S08B', 'ADS114S08BIRHBT': 'ADS114S08B',
            'ADS8588HIPM': 'ADS8588H', 'ADS8588SIPM': 'ADS8588S', 'ADS8588SIPMR': 'ADS8588S',
            'ADS8598HIPM': 'ADS8598H', 'ADS8598SIPM': 'ADS8598S',
            'ADS8684AIDBT': 'ADS8684A', 'ADS8688AIDBT': 'ADS8688A', 'ADS8688AIDBTR': 'ADS8688A',
            'ADS8688ATDBT': 'ADS8688A'}


def base_of(r):
    if r['name'] in OVERRIDE:
        return OVERRIDE[r['name']]
    if r['base']:
        return r['base']
    n = r['name']
    cands = [b for b in bases_by_pdf[r['pdf']] if n.startswith(b)]
    if cands:
        return max(cands, key=len)
    if len(bases_by_pdf[r['pdf']]) == 1:
        return next(iter(bases_by_pdf[r['pdf']]))
    m = re.match(r'^(5962-\d+|[A-Z]+\d+[A-Z]?\d*[A-Z]?\d*?)(?=[ICQMHAEJ]|$)', n)
    return m.group(1) if m else n

dev = collections.defaultdict(lambda: {'pdfs': set(), 'codes': set(), 'parts': []})
for r in rows:
    b = base_of(r)
    d = dev[b]
    d['pdfs'].add(r['pdf']); d['codes'].add(r['code'] + '-' + r['pins']); d['parts'].append(r['name'])
# clusters: union of pdfs sharing a device
parent = {}
def find(x):
    while parent.setdefault(x, x) != x:
        x = parent[x]
    return x
for b, d in dev.items():
    ps = sorted(d['pdfs'])
    for p in ps[1:]:
        parent[find(p)] = find(ps[0])
for b in sorted(dev, key=lambda b: (find(sorted(dev[b]['pdfs'])[0]), b)):
    d = dev[b]
    print('\t'.join([find(sorted(d['pdfs'])[0]), b, ' '.join(sorted(d['pdfs'])),
                     ' '.join(sorted(d['codes'])), ' '.join(sorted(d['parts']))]))
