"""plan-33-A § 3: rank each bucket's candidates; write candidates.tsv.

usage: python3 -I select.py <raw.jsonl[.gz]>... <candidates.tsv>

Reads bucket-parts.mjs output (one line per bucket, its 50 most-stocked
parts) and writes one row per (bucket, part) with the bucket's pick order in
`rank` (1 = tried first). The order (§ 3 rule 2):

  1. not automotive (no AEC-Q200 in Ratings) before automotive;
  2. ceramic "other" buckets: dielectric X7R, then X5R, then the rest;
  3. ceramic: rated voltage at or above 50 V, lowest first; then below
     50 V, highest first (electrolytic and tantalum buckets fix the voltage);
  4. more DigiKey stock first;
  5. part number.

A part is `out` before ranking (reason in `reason`) when it is military
(Ratings or Grade naming MIL) or a wet tantalum (Features). Every other row
starts as `candidate`; plan-33-A/B set `add`, `alt`, `skip` or `out`.
"""
import csv, gzip, json, re, sys

COLUMNS = ['bucket', 'type', 'mount', 'key', 'capacitance', 'farads', 'rank', 'mpn',
           'maker', 'status', 'stock', 'series', 'voltage', 'dielectric', 'tolerance',
           'package', 'mounting', 'size', 'height', 'spacing', 'ratings', 'size_code',
           'datasheet_url', 'flags', 'decision', 'reason', 'pdf', 'hit', 'dead']
UNIT = {'pF': 1e-12, 'nF': 1e-9, 'µF': 1e-6, 'mF': 1e-3, 'F': 1.0}
MAKER = {'KEMET': 'KEMET', 'Cal-Chip Electronics, Inc.': 'Cal-Chip Electronics',
         'KYOCERA AVX': 'KYOCERA AVX'}
DIELECTRIC_RANK = {'X7R': 0, 'X5R': 1}


def farads(text):
    m = re.fullmatch(r'([\d.]+)\s*(pF|nF|µF|mF|F)', text.strip())
    return float(m.group(1)) * UNIT[m.group(2)] if m else None


def volts(text):
    m = re.match(r'\s*([\d.]+)\s*V', text or '')
    return float(m.group(1)) if m else 0.0


def key(typ, bucket_key, p):
    automotive = 'AEC-Q200' in p['ratings']
    dielectric = DIELECTRIC_RANK.get(p['dielectric'], 2) if bucket_key == 'other' else 0
    v = volts(p['voltage'])
    voltage = (0, v) if typ != 'ceramic' or v >= 50 else (1, -v)
    return (automotive, dielectric, voltage, -(p.get('stock') or 0), p['mpn'])


def main(args):
    *raws, out = args
    rows = []
    for raw in raws:
        opener = gzip.open if raw.endswith('.gz') else open
        for line in opener(raw, 'rt', encoding='utf-8'):
            b = json.loads(line)
            typ, mount, bucket_key = b['label'].split('|')
            kept, gone = [], []
            for p in b['products']:
                p = {k: (v if v is not None else '') for k, v in p.items()}
                why = ''
                if re.search(r'MIL', p['ratings'] or ''):
                    why = 'military'
                elif 'Wet Tantalum' in (p.get('description') or ''):
                    why = 'wet tantalum'
                (gone if why else kept).append((p, why))
            kept.sort(key=lambda pw: key(typ, bucket_key, pw[0]))
            for rank, (p, why) in enumerate(kept + gone, 1):
                rows.append({
                    'bucket': f"{b['label']}|{b['capacitance']}", 'type': typ, 'mount': mount,
                    'key': bucket_key, 'capacitance': b['capacitance'],
                    'farads': f"{farads(b['capacitance'].split(';')[0]):.4g}",
                    'rank': rank if not why else '', 'mpn': p['mpn'],
                    'maker': MAKER.get(p['maker'], p['maker']), 'status': p['status'],
                    'stock': p.get('stock') or 0, 'series': p['series'], 'voltage': p['voltage'],
                    'dielectric': p['dielectric'], 'tolerance': p['tolerance'],
                    'package': p['package'], 'mounting': p['mounting'], 'size': p['size'],
                    'height': p['height'], 'spacing': p['spacing'], 'ratings': p['ratings'],
                    'size_code': p['sizeCode'], 'datasheet_url': p['datasheet'],
                    'flags': 'automotive' if 'AEC-Q200' in (p['ratings'] or '') else '',
                    'decision': 'out' if why else 'candidate', 'reason': why,
                    'pdf': '', 'hit': '', 'dead': ''})
    with open(out, 'w', newline='', encoding='utf-8') as f:
        w = csv.DictWriter(f, COLUMNS, delimiter='\t', lineterminator='\n')
        w.writeheader()
        w.writerows(rows)
    buckets = len({r['bucket'] for r in rows})
    out_rows = sum(r['decision'] == 'out' for r in rows)
    print(f'{buckets} buckets, {len(rows)} rows, {out_rows} out')


if __name__ == '__main__':
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    main(sys.argv[1:])
