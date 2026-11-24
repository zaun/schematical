#!/usr/bin/env python3
"""plan-51 census: the counts the plan cites.  usage: python3 -I tally.py [--candidates out.tsv]"""
import collections, sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common

per_maker = collections.Counter()
shape = collections.Counter()
variants = set()
datasheets = set()
groups = set()
cands = []
for maker in common.MAKERS:
    try:
        listing = list(common.rows(maker))
    except FileNotFoundError:
        continue
    for r in listing:
        s = common.scope(r)
        if not s:
            continue
        per_maker[common.MAKERS[maker]] += 1
        if maker in (54, 1212, 123):
            shape[s] += 1
            variants.add((maker, r['series']) + s)
            groups.add((maker, r['series'], s[0], s[3], s[2]))
            datasheets.add(r['datasheet'])
            cands.append((common.MAKERS[maker], r['mpn'], r['series'], *s, r['stock'], r['status'], r['datasheet']))
print('in scope per maker:', per_maker.most_common())
print('three makers:', sum(per_maker[common.MAKERS[m]] for m in (54, 1212, 123)))
print('(maker, series, family, mounting, spacing) groups:', len(groups))
print('(maker, series, family, pins, spacing, mounting) variants:', len(variants))
print('distinct datasheet URLs:', len(datasheets))
print('per shape:', sorted(shape.items()))
if len(sys.argv) > 2 and sys.argv[1] == '--candidates':
    with open(sys.argv[2], 'w') as f:
        # Columns 11-14 (plan-51-B): the downloaded PDF, the row's state ('candidate', or 'out'
        # with the reason), and the date of the last in-stock check. B fills them in; running this
        # again starts them empty, so never re-run it over a filled file.
        f.write('maker\tmpn\tseries\tfamily\tpins\tspacing\tmounting\tstock\tstatus\tdatasheet'
                '\tpdf\tstate\treason\tstock_checked\n')
        for c in sorted(cands):
            f.write('\t'.join(str(x) for x in c) + '\t\tcandidate\t\t\n')
