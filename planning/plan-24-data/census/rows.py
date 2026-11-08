#!/usr/bin/env python3
"""Join the census (candidates.tsv) with the datasheets' ordering tables.

usage: rows.py <group> <canon.tsv> <poa.tsv> <pdfkeys.tsv> > rows-<group>.tsv
Out (TSV): group name base status interface pkg_digikey pdf poa_status pkgtype code pins match
match: own (in its own datasheet's table), other (another plan-24 datasheet lists it),
none (in no table), out (out of scope), status (not Active/LTB).
"""
import csv, collections, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
CAND = os.path.join(os.path.dirname(HERE), 'candidates.tsv')
INS = {'spi', 'i2c', 'parallel', 'parallel+spi', 'i2c+spi'}
grp, canon_f, poa_f, keys_f = sys.argv[1:5]
canon = dict(l.rstrip('\n').split('\t') for l in open(canon_f))
# rows whose DigiKey datasheet link is not TI's (Rochester / DigiKey copies): the TI
# datasheet found by hand under the base name (ti.com/lit/ds/symlink/<name>.pdf)
key_of = {'5962-0051101NXD': 'ths1401.pdf', 'ADC121S021CIMF-TI': 'adc121s021.pdf',
          'ADC121S655CIMM/NOPB': 'adc121s655.pdf', 'ADC3541IRSBT': 'adc3541.pdf',
          'ADC3542IRSBT': 'adc3541.pdf', 'ADS112S14IRTER': 'ads112s14.pdf',
          'ADS114S18IPBSR': 'ads114s18.pdf', 'ADS124S18IPBSR': 'ads124s18.pdf',
          'ADS1286U-TI': 'ads1286.pdf', 'ADS7824U/1K': 'ads7824.pdf',
          'ADS7881IPFBT': 'ads7881.pdf', 'TLC0834CDR': 'tlc0834.pdf',
          'TLV5580IDWR': 'tlv5580.pdf', 'TLV5580IPWR': 'tlv5580.pdf',
          '5962-9955701QPA': 'tlv5618a.pdf', '5962-9955702Q2A': 'tlc5618a.pdf',
          '5962-9955702QPA': 'tlc5618a.pdf', 'DAC1221E/2K5': 'dac1221.pdf',
          'DAC161S055CISQ/NOPB': 'dac161s055.pdf', 'DAC43701DSGRQ1': 'dac43701-q1.pdf',
          'DAC53701DSGRQ1': 'dac53701-q1.pdf', 'DAC715UK-TI': 'dac715.pdf',
          'DAC7617U-TI': 'dac7617.pdf', 'DAC7641Y': 'dac7641.pdf', 'DAC811KU-1': 'dac811.pdf',
          'DAC811KU-1/1K': 'dac811.pdf', 'BQ3285ESS-N': 'bq3285e.pdf',
          'BQ3285ESS-TR1': 'bq3285e.pdf', 'DP8570AV/NOPB': 'dp8570a.pdf',
          'DP8573AV/NOPB': 'dp8573a.pdf'}
for l in open(keys_f):
    k, parts = l.rstrip('\n').split('\t')
    for p in parts.split():
        key_of[p] = k + '.pdf'
poa = collections.defaultdict(dict)   # pdf -> part -> row
where = collections.defaultdict(list)  # part -> pdfs
for l in open(poa_f):
    pdf, part, st, pt, code, pins = l.rstrip('\n').split('\t')
    poa[pdf][part] = (st, pt, code, pins)
    where[part].append(pdf)

def variants(n):
    yield n
    for s in ('-TI',):
        if n.endswith(s):
            yield n[:-len(s)]

rows = list(csv.DictReader(open(CAND), delimiter='\t'))
w = csv.writer(sys.stdout, delimiter='\t', lineterminator='\n')
w.writerow(['group', 'name', 'base', 'status', 'interface', 'pkg_digikey', 'pdf', 'poa_status',
            'pkgtype', 'code', 'pins', 'match', 'dk_interface'])
for r in rows:
    if r['group'] != grp:
        continue
    n = r['name']
    if r['status'] not in ('Active', 'Last Time Buy'):
        m = 'status'
    elif grp != 'RTC' and r['interface'] not in INS:
        m = 'out'
    else:
        m = None
    pdf = canon.get(key_of.get(n, ''), '')
    hit = None
    if m is None:
        for v in variants(n):
            if pdf and v in poa[pdf]:
                hit, m = (pdf, poa[pdf][v]), 'own'
                break
            if where.get(v):
                p2 = where[v][0]
                hit, m = (p2, poa[p2][v]), 'other'
                break
        if hit is None:
            m = 'none'
    if hit:
        pdf = hit[0]
        st, pt, code, pins = hit[1]
    else:
        st = pt = code = pins = ''
    w.writerow([grp, n, r['base'], r['status'], r['interface'], r['supplier_pkg'], pdf, st, pt,
                code, pins, m, r['digikey_interface'] or '-'])
