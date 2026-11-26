import json, sys, collections, os
d = json.load(open(sys.argv[1]))
root = '/Users/justinzaun/Development/logic/'
rows = []
for t in d['targets']:
    if t['name'] != 'schematical.app': continue
    for f in t['files']:
        p = f['path'].replace(root, '')
        if not p.endswith('.swift') or not p.startswith('schematical/schematical/'): continue
        miss = f['executableLines'] - f['coveredLines']
        rows.append((p, miss, f['executableLines']))
tot = sum(r[2] for r in rows); miss = sum(r[1] for r in rows)
print(f'files {len(rows)} executable {tot} uncovered {miss} cover {100*(tot-miss)/tot:.2f}%')
by = collections.defaultdict(lambda: [0,0,0])
for p,m,e in rows:
    k = p.split('/')[2] if p.count('/')>2 else '(root)'
    by[k][0]+=m; by[k][1]+=e; by[k][2]+=1
for k,(m,e,n) in sorted(by.items(), key=lambda x:-x[1][0]):
    print(f'{m:7d} {e:7d} {n:4d} {k}')
with open(sys.argv[2],'w') as out:
    for p,m,e in sorted(rows, key=lambda r:-r[1]):
        out.write(f'{m}\t{e}\t{p}\n')
