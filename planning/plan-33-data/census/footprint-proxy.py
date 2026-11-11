"""plan-33 planning time: footprints-needed.txt before B decides any bucket.

python3 -I census/footprint-proxy.py candidates.tsv facets.jsonl > footprints-needed.txt

Section 1 (measured lines): one line per (maker, package, body size, lead
spacing) of the rank-1 rows of the listed buckets, with the row count.
Section 2 (upper bounds): the buckets not yet listed, per (type, mount), the
DigiKey size / package values their queries' facets show and the makers
present; the footprints C and D draw are at most (values x makers).
plan-33-B Phase 4 replaces this file with the add rows' groups.
"""
import collections, csv, json, re, sys

def letter(typ, mount):
    chip = mount in ('0402', '0603', '0805', '1206') or (mount == 'SMD' and typ in ('tantalum', 'ta-polymer', 'niobium'))
    return 'C' if chip else 'D'

def mm(text):
    m = re.search(r'\(([^)]*)\)', text or '')
    return m.group(1) if m else (text or '-')

rows = list(csv.DictReader(open(sys.argv[1], encoding='utf-8'), delimiter='\t'))
listed = {r['bucket'].rsplit('|', 1)[0] for r in rows}
lines = collections.Counter((letter(r['type'], r['mount']), r['maker'], r['type'], r['mount'], mm(r['size']), mm(r['spacing']) or '-')
                            for r in rows if r['rank'] == '1')
print('# plan-33 footprints needed (planning time, 2026-11-11)')
print('# section 1: measured, rank-1 rows of the %d listed buckets' % len({r['bucket'] for r in rows}))
print('# letter\tmaker\ttype\tpackage\tsize (mm)\tlead spacing\trank-1 rows')
for k, n in sorted(lines.items()):
    print('\t'.join(k) + f'\t{n}')
print('# section 2: upper bounds for the unlisted queries (facets.jsonl): letter, type, mount, values, makers, bound')
agg = collections.defaultdict(lambda: [set(), set()])
for line in open(sys.argv[2], encoding='utf-8'):
    r = json.loads(line)
    typ, mount, _ = r['label'].split('|')
    sizes = r['facets'].get('Size / Dimension', [])
    if typ in ('tantalum', 'ta-polymer', 'niobium') and mount == 'SMD':
        sizes = r['facets'].get('Package / Case', [])
    for v, n in sizes:
        if n > 0 and v != '-':
            agg[(typ, mount)][0].add(mm(v))
    for m, n in r['facets'].get('Manufacturer', []):
        if n > 0:
            agg[(typ, mount)][1].add(m.split(',')[0])
total = 0
for (typ, mount), (vals, makers) in sorted(agg.items()):
    bound = len(vals) * len(makers); total += bound
    print(f"{letter(typ, mount)}\t{typ}\t{mount}\t{len(vals)} values: {', '.join(sorted(vals))}\t{', '.join(sorted(makers))}\t<= {bound}")
print(f'# section 2 bound total: {total}')
print('# not covered: the unlisted ceramic buckets (Radial 65, Disc 17, Axial 23 of 50); no facet call was left for them on 2026-11-11 (plan-33-A § 2)')
