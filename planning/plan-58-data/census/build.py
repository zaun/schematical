#!/usr/bin/env python3
"""plan-58-A: raw census pages -> candidates.tsv.

usage: python3 -I build.py <raw-dir> <candidates.tsv>

Reads <raw-dir>/<maker>_<circuit>.json (list.sh), one row per (maker, manufacturer part number): DigiKey lists one
product per packaging (Bulk, Tape & Reel, Cut Tape, Tray), so rows with the same part number are merged; the
packagings are joined with "/" and the stock is the largest "In Stock" line. Columns:

  maker mpn series circuit function timing current vac vdc actuator mounting termination features
  stock packaging datasheet detail decision reason

`decision` (plan-58-A § 4.1; the rules are DigiKey's own columns, no datasheet is read here):
  out   no stock on DigiKey's own "In Stock" line (Marketplace and Factory lines are not stock)
  out   Mounting Type is Panel Mount or Panel Mount, Snap-In (owner's decision, plan-58)
  add   Switch Function is On-On, On-Off-On, On-Off or Off-On and the actuator is not Illuminated
  out   any other function (momentary, line-voltage selector, On-Off-On/On-On-On, ...), or an illuminated actuator
  check Switch Function is blank ("-"): the datasheet decides (plan-58-A)
"""
import csv
import glob
import json
import os
import sys

MAKER = {'C&K': 'C&K', 'Littelfuse/C&K': 'C&K', 'NKK Switches': 'NKK', 'CIT Relay and Switch': 'CIT',
         'E-Switch': 'E-Switch'}
LATCHING = {'On-On', 'On-Off-On', 'On-Off', 'Off-On'}
COLS = ['maker', 'mpn', 'series', 'circuit', 'function', 'timing', 'current', 'vac', 'vdc', 'actuator',
        'mounting', 'termination', 'features', 'stock', 'packaging', 'datasheet', 'detail', 'decision', 'reason']


def text(cell):
    v = cell['value']
    if isinstance(v, list):
        return '/'.join(x.get('value', x.get('label', '')) for x in v)
    return v.get('value', v.get('label', ''))


def main(raw, out):
    rows = {}
    for path in sorted(glob.glob(os.path.join(raw, '*.json'))):
        for p in json.load(open(path)).get('p', []):
            c = [text(x) if x['type'] not in ('compare', 'productDetail', 'qtyAvailable') else '' for x in p]
            detail = p[1]['value']
            maker = MAKER[p[0]['value']['manufacturer']['name']]
            # DigiKey's own stock only: the "In Stock" line; Marketplace and Factory lines are not stock (AGENTS.md).
            stock = sum(int(q['quantity'].replace(',', '')) for q in p[2]['value'] if q['label'] == 'In Stock')
            key = (maker, detail['productNumber'])
            row = rows.setdefault(key, {
                'maker': maker, 'mpn': detail['productNumber'], 'series': c[5], 'circuit': c[8], 'timing': c[9],
                'function': c[10], 'current': c[11], 'vac': c[12], 'vdc': c[13], 'actuator': c[14],
                'mounting': c[18], 'termination': c[19], 'features': c[20], 'stock': 0, 'packaging': [],
                'datasheet': detail.get('datasheetUrl', ''), 'detail': 'https://www.digikey.com' + detail['detailUrl']})
            row['stock'] = max(row['stock'], stock)
            for pk in c[6].split('/'):
                if pk and pk not in row['packaging']:
                    row['packaging'].append(pk)
    with open(out, 'w', newline='') as f:
        w = csv.writer(f, delimiter='\t', lineterminator='\n')
        w.writerow(COLS)
        for key in sorted(rows):
            r = rows[key]
            if r['stock'] < 1:
                r['decision'], r['reason'] = 'out', 'no DigiKey stock (Marketplace or Factory stock only)'
            elif r['mounting'].startswith('Panel Mount'):
                r['decision'], r['reason'] = 'out', 'panel mount (no PCB land pattern; owner, 2026-11-24)'
            elif r['function'] in LATCHING and r['actuator'] != 'Illuminated':
                r['decision'], r['reason'] = 'add', ''
            elif r['function'] == '-':
                r['decision'], r['reason'] = 'check', 'DigiKey gives no Switch Function'
            elif r['function'] in LATCHING:
                r['decision'], r['reason'] = 'out', 'illuminated actuator'
            else:
                r['decision'], r['reason'] = 'out', 'function ' + r['function']
            r['packaging'] = '/'.join(r['packaging'])
            w.writerow([r[c] for c in COLS])
    print(len(rows), 'parts ->', out)


main(*sys.argv[1:3])
