#!/usr/bin/env python3
"""plan-34 census: pick the resistors to add from the top.js listings.

usage: python3 -I select.py <raw dir> <candidates.tsv> [--verdicts <tsv>] [--report]

--verdicts: plan-34-B's datasheet verdicts, a TSV with columns mpn, verdict
(add | skip), reason, datasheet (the PDF path). A skip row is never picked
(decision skip, its reason); a picked row with verdict add becomes add; with
--verdicts every picked row must have one (else the run fails and names it).
Without --verdicts (plan-34-A) picked rows are 'candidate'.

Rules (plan-34-A § 3), in order:
  1. scope: Features/Ratings/Grade/Qualification naming "Automotive" or
     "AEC-Q" -> out (automotive); Features "Current Sense" or Composition
     "Metal Element" -> out (current-sense); everything else is a candidate.
     Features "Military" -> flag military.
  2. "the same thing": key = (category, ohms, tolerance, size), where size is
     the chip's Package / Case, the through-hole part's Package / Case plus
     its power, and an array's Package / Case plus circuit type plus element
     count. At most TWO parts per key are kept: the most-stocked one, and
     the most-stocked one of another maker if the key has one, else the
     next most-stocked (ties: part number). The rest -> out (duplicate).
  3. quota per category, highest stock first: chip 1500, through-hole 400,
     arrays 100 (§ 3). The rest -> out (below quota).
Writes one row per listed part, decision 'candidate' (kept) or 'out' with a
reason. --report prints the counts and the cap check of § 5 (plan-34-A).
"""
import csv
import glob
import json
import os
import re
import sys

QUOTA = {'52': 1500, '53': 400, '50': 100}
CATEGORY = {'52': 'chip', '53': 'through-hole', '50': 'array'}
BUCKETS = [(0, '< 1 kΩ'), (1e3, '1 kΩ – 5 kΩ'), (5e3, '5 kΩ – 10 kΩ'),
           (1e4, '10 kΩ – 50 kΩ'), (5e4, '50 kΩ – 100 kΩ'),
           (1e5, '100 kΩ – 500 kΩ'), (5e5, '500 kΩ – 1 MΩ'), (1e6, '≥ 1 MΩ')]
UNIT = {'': 1, 'k': 1e3, 'M': 1e6, 'G': 1e9, 'm': 1e-3}


def ohms(text):
    """'100 kOhms', '4.7k', '0 Ohms', '1M' -> float; None if not a number."""
    m = re.match(r'^\s*([0-9.]+)\s*([kMGm]?)', text or '')
    if not m:
        return None
    return float(m.group(1)) * UNIT[m.group(2)]


def bucket(value):
    label = BUCKETS[0][1]
    for low, name in BUCKETS:
        if value >= low:
            label = name
    return label


def load(raw):
    rows = []
    seen = set()
    for path in sorted(glob.glob(os.path.join(raw, '5[023]-*.jsonl'))):
        for line in open(path):
            d = json.loads(line)
            if d['mpn'] in seen:
                continue  # a re-listing (recheck.sh) finds it again
            seen.add(d['mpn'])
            p = d['params']
            cat = d['category']
            res = p.get('Resistance') or p.get('Resistance (Ohms)') or ''
            tol = p.get('Tolerance', '')
            value = 0.0 if tol == 'Jumper' else ohms(res)
            pkg = p.get('Package / Case', '')
            if cat == '52':
                size = pkg
            elif cat == '53':
                size = f"{pkg} {p.get('Power (Watts)', '')}"
            else:
                size = f"{pkg} {p.get('Circuit Type', '')} x{p.get('Number of Resistors', '')}"
            text = ' '.join(p.get(k, '') for k in ('Features', 'Ratings', 'Grade', 'Qualification'))
            reason = ''
            if 'Automotive' in text or 'AEC-Q' in text:
                reason = 'automotive'
            elif 'Current Sense' in p.get('Features', '') or p.get('Composition') == 'Metal Element':
                reason = 'current-sense'
            elif value is None:
                reason = 'resistance not a number'
            rows.append({
                'mpn': d['mpn'], 'maker': d['maker'], 'category': CATEGORY[cat], 'cat': cat,
                'piece': d['piece'], 'stock': d['stock'], 'status': d['status'],
                'ohms': '' if value is None else f'{value:g}', 'value': value,
                'bucket': '' if value is None else bucket(value), 'tolerance': tol,
                'power': p.get('Power (Watts)') or p.get('Power Per Element', ''),
                'composition': p.get('Composition', ''), 'package': pkg,
                'supplier_package': p.get('Supplier Device Package', ''),
                'size_dim': p.get('Size / Dimension', ''),
                'circuit': p.get('Circuit Type', ''), 'elements': p.get('Number of Resistors', ''),
                'pins': p.get('Number of Pins', ''), 'series': d['series'],
                'military': 'military' if 'Military' in p.get('Features', '') else '',
                'datasheet_url': d['datasheet'], 'key': (cat, value, tol, size),
                'decision': 'out' if reason else 'candidate', 'reason': reason,
            })
    return rows


def main():
    raw, out = sys.argv[1], sys.argv[2]
    rows = load(raw)
    verdicts = {}
    if '--verdicts' in sys.argv:
        path = sys.argv[sys.argv.index('--verdicts') + 1]
        for v in csv.DictReader(open(path), delimiter='\t'):
            verdicts[v['mpn']] = v
    for r in rows:
        v = verdicts.get(r['mpn'])
        r['pdf'] = v['datasheet'] if v else ''
        if r['decision'] == 'candidate' and v and v['verdict'] == 'skip':
            r['decision'], r['reason'] = 'skip', v['reason']
    live = sorted((r for r in rows if r['decision'] == 'candidate'),
                  key=lambda r: (-r['stock'], r['mpn']))
    # Two per key: the most-stocked part, then the most-stocked part of
    # another maker when the key has one, else the next part (§ 3 rule 3).
    groups = {}
    for r in live:
        groups.setdefault(r['key'], []).append(r)
    for parts in groups.values():
        first = parts[0]
        other = next((r for r in parts[1:] if r['maker'] != first['maker']), None)
        second = other or (parts[1] if len(parts) > 1 else None)
        for r in parts:
            if r is not first and r is not second:
                r['decision'], r['reason'] = 'out', 'duplicate'
    taken = {c: 0 for c in QUOTA}
    floor = {}
    for r in live:
        if r['decision'] != 'candidate':
            continue
        if taken[r['cat']] >= QUOTA[r['cat']]:
            r['decision'], r['reason'] = 'out', 'below quota'
            continue
        taken[r['cat']] += 1
        floor[r['cat']] = r['stock']
        if verdicts:
            v = verdicts.get(r['mpn'])
            if v is None:
                sys.exit(f"select.py: picked {r['mpn']} has no verdict")
            r['decision'] = 'add'
    cols = ['mpn', 'maker', 'category', 'piece', 'stock', 'status', 'ohms', 'bucket',
            'tolerance', 'power', 'composition', 'package', 'supplier_package', 'size_dim',
            'circuit', 'elements', 'pins', 'series', 'military', 'datasheet_url',
            'pdf', 'decision', 'reason']
    with open(out, 'w') as f:
        f.write('\t'.join(cols) + '\n')
        for r in sorted(rows, key=lambda r: (r['cat'], r['maker'], r['mpn'])):
            f.write('\t'.join(str(r[c]).replace('\t', ' ') for c in cols) + '\n')
    if '--report' in sys.argv:
        from collections import Counter
        print(f'{len(rows)} parts listed')
        print('decision', Counter((r['decision'], r['reason']) for r in rows))
        print('kept per category/maker', Counter((r['category'], r['maker']) for r in rows if r['decision'] in ('candidate', 'add')))
        print('kept per bucket', Counter((r['category'], r['bucket']) for r in rows if r['decision'] in ('candidate', 'add')))
        print('floor (stock of the last kept part)', floor)
        # § 5: a piece cut at 300 is complete for this selection when its
        # 300th part's stock is below the category's floor.
        # A base piece that recheck.sh re-listed (its label starts a piece
        # label of a <cat>-<maker>-re-<n>.err) is superseded by the re-listing.
        relisted = {}
        for err in glob.glob(os.path.join(raw, '5[023]-*-re-*.err')):
            stem = os.path.basename(err).split('-re-')[0]
            for line in open(err):
                m = re.match(r'piece (.*): ', line)
                if m:
                    relisted.setdefault(stem, []).append(m.group(1))
        for err in sorted(glob.glob(os.path.join(raw, '5[023]-*.err'))):
            cat = os.path.basename(err)[:2]
            stem = os.path.basename(err)[:-4]
            for line in open(err):
                m = re.match(r'piece (.*): 300 of (\d+) \(cut at 300: (\d+)\)', line)
                if m and '-re-' not in stem and any(
                        l == m.group(1) or l.startswith(m.group(1) + '/') for l in relisted.get(stem, [])):
                    continue
                if m:
                    ok = int(m.group(3)) < floor.get(cat, 0)
                    print(f"cap {os.path.basename(err)} {m.group(1)}: 300th stock {m.group(3)}, floor {floor.get(cat)} -> {'complete' if ok else 'NEEDS DEEPER LISTING'}")


if __name__ == '__main__':
    main()
