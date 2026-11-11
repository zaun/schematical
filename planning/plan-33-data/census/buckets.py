"""plan-33: count the buckets (one kept part each) in buckets.mjs output.

python3 -I buckets.py <buckets.jsonl> [<buckets.tsv>]
With a second path, also writes one line per bucket: label, capacitance
(the DigiKey value ids of that capacitance, joined by ';'), the input to
bucket-parts.mjs.
A bucket is (type, package, value[, dielectric class | voltage]) with a
capacitance on the E6 series (1.0 1.5 2.2 3.3 4.7 6.8) inside the type's range.
"""
import json, re, sys, collections

E6 = (10, 15, 22, 33, 47, 68)
UNIT = {'pF': 1e-12, 'nF': 1e-9, 'µF': 1e-6, 'mF': 1e-3, 'F': 1.0}
RANGE = {'ceramic': (10e-12, 100e-6), 'aluminum': (0.1e-6, 4700e-6),
         'al-polymer': (0.1e-6, 4700e-6), 'tantalum': (0.1e-6, 1000e-6),
         'ta-polymer': (0.1e-6, 1000e-6), 'niobium': (0.1e-6, 1000e-6)}

def farads(name):
    m = re.fullmatch(r'([\d.]+)\s*(pF|nF|µF|mF|F)', name.strip())
    return float(m.group(1)) * UNIT[m.group(2)] if m else None

def e6(f):
    if not f: return False
    exp = 0
    while f * 10 ** exp < 9.999: exp += 1
    while f * 10 ** exp >= 99.99: exp -= 1
    return round(f * 10 ** exp) in E6 and abs(f * 10 ** exp - round(f * 10 ** exp)) < 1e-6

def main(path, tsv=None):
    # One capacitance can carry several DigiKey value ids ("0.01 µF",
    # "10 nF", "10000 pF"): a bucket is (label, farads), its ids joined by ";".
    buckets = {}
    for line in open(path, encoding='utf-8'):
        r = json.loads(line)
        typ = r['label'].split('|')[0]
        lo, hi = RANGE[typ]
        for v, n in r['values']:
            f = farads(v)
            if n > 0 and e6(f) and lo * 0.999 <= f <= hi * 1.001:
                buckets.setdefault((r['label'], float(f'{f:.3g}')), []).append((v, n))
    per = collections.Counter(); values = collections.defaultdict(set)
    for (label, f), ids in buckets.items():
        typ, pkg, _ = label.split('|')
        per[(typ, pkg)] += 1
        values[typ].add(f)
    total = 0
    for typ in RANGE:
        rows = {k[1]: n for k, n in per.items() if k[0] == typ}
        s = sum(rows.values()); total += s
        print(f'{typ}: {s} buckets {rows}; {len(values[typ])} distinct E6 values')
    multi = sum(len(ids) > 1 for ids in buckets.values())
    print(f'total {total} buckets ({multi} with more than one DigiKey value id)')
    if tsv:
        with open(tsv, 'w', encoding='utf-8') as f:
            f.write('label\tcapacitance\n')
            for (label, _), ids in sorted(buckets.items(), key=lambda kv: (kv[0][0], kv[0][1])):
                f.write(f"{label}\t{';'.join(v for v, _ in ids)}\n")


main(*sys.argv[1:3])
