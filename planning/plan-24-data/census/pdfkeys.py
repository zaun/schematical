"""Derive the TI datasheet name for each in-scope addable row."""
import csv, collections, re, sys, urllib.parse
W = '/Users/justinzaun/Development/logic/.claude/worktrees/P-24'
rows = list(csv.DictReader(open(W + '/planning/plan-24-data/candidates.tsv'), delimiter='\t'))
INS = {'spi', 'i2c', 'parallel', 'parallel+spi', 'i2c+spi'}
grp = sys.argv[1]

def key(r):
    u = urllib.parse.unquote(urllib.parse.unquote(r["datasheet_url"]))
    m = re.search(r'ti\.com/lit/(?:gpn|ds/symlink|ds)/([a-z0-9\-_.]+?)(?:\.pdf)?(?:$|[?&#/])', u, re.I)
    if m:
        return m.group(1).lower(), 'ti'
    if r['base']:
        return r['base'].lower(), 'base'
    return None, u

out = collections.defaultdict(list)
other = []
for r in rows:
    if r['group'] != grp or r['status'] not in ('Active', 'Last Time Buy'):
        continue
    if grp != 'RTC' and r['interface'] not in INS:
        continue
    k, src = key(r)
    if k:
        out[k].append(r['name'])
    else:
        other.append((r['name'], r['base'], src))
with open(f'/tmp/p24/pdfkeys-{grp}.tsv', 'w') as o:
    for k, L in sorted(out.items()):
        o.write(k + '\t' + ' '.join(L) + '\n')
print(len(out), 'keys;', sum(map(len, out.values())), 'rows; no key', len(other))
for x in other: print('  ', x)
