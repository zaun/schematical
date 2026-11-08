import csv, collections, glob
W = '/Users/justinzaun/Development/logic/.claude/worktrees/P-24'
rows = list(csv.DictReader(open(W + '/planning/plan-24-data/candidates.tsv'), delimiter='\t'))
INS = {'spi', 'i2c', 'parallel', 'parallel+spi', 'i2c+spi'}
seen = {}
for f in glob.glob('/tmp/p24/dk/list-*.tsv'):
    cat = f.split('-')[1]
    for l in open(f):
        p = l.rstrip('\n').split('\t')
        if p[0]:
            seen[p[0]] = (cat, p[1])
G = {'700': 'ADC', '701': 'DAC', '690': 'RTC'}
cand = {r['name']: r for r in rows}
new = [n for n in seen if n not in cand]
print('listed', len(seen), 'not in candidates:', len(new))
for n in new: print('  NEW', n, seen[n])
changed = collections.Counter()
missing = collections.defaultdict(list)
for r in rows:
    g = r['group']
    ins = g == 'RTC' or r['interface'] in INS
    if r['name'] in seen:
        st = seen[r['name']][1]
        if st != r['status']:
            changed[(r['status'], st)] += 1
    elif r['status'] in ('Active', 'Last Time Buy') and ins:
        missing[g].append(r)
print('status changes', changed)
for g, L in missing.items():
    print(g, 'in-scope addable rows not listed:', len(L), 'bits', collections.Counter(r['bits'] for r in L))
    print('   bases', sorted(set(r['base'] or '(none):' + r['name'] for r in L)))
