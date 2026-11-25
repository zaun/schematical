#!/usr/bin/env python3
"""plan-47 census: candidates.tsv from the listing.
usage: select.py <raw dir> <candidates.tsv> [--urls url-check.tsv]
                 [--stock stock-check.tsv] [--keep old-candidates.tsv] [--report]

Scope, duplicates and URLs are common.py's. --urls makes a row whose
maker-site URL did not answer a PDF `out` (reason `no datasheet`) and takes
the URL that answered. --stock makes a row the part-by-part check did not
find in stock `out` (reason `stock check`) and sets stock_checked. --keep
copies the pdf, hit and dead columns of an earlier file (plan-47-A fills
them). Each (type, company) share is 200 rows (common.PER_MAKER) over the
classes of common.QUOTA; inside a class the rows go round-robin over the
series (each series' most-stocked row first; the series with more stock
first in a round). The first quota rows of a class are `candidate`, the next
quarter `reserve`, the rest `out` (`below quota`). The download set is every
candidate and reserve row."""
import collections
import csv
import sys

sys.path.insert(0, __import__('os').path.dirname(__file__))
import common

COLUMNS = ['mpn', 'company', 'type', 'class', 'series', 'status', 'stock', 'value', 'form', 'package',
           'supplier_package', 'size', 'mounting', 'grade', 'detail', 'category', 'piece', 'cut', 'maker', 'description',
           'via', 'url', 'state', 'reason', 'rank', 'pdf', 'hit', 'dead', 'stock_checked', 'verdict', 'vreason']


def grade(p):
    text = ' '.join(p.get(k, '') for k in ('Ratings', 'Grade', 'Qualification'))
    if 'Military' in text or 'MIL-' in text:
        return 'Military'
    if 'AEC-Q200' in text or 'Automotive' in text:
        return 'AEC-Q200'
    return ''


def detail(r):
    """What the footprint also depends on, per type."""
    p = r['params']
    if r['type'] == 'relay':
        return '; '.join(x for x in (p.get('Coil Type', ''), p.get('Termination Style', '')) if x and x != '-')
    if r['type'] == 'ptc':
        return p.get('Lead Spacing', '').replace('-', '') if p.get('Lead Spacing', '-') != '-' else ''
    if r['type'] == 'buzzer':
        return p.get('Termination', '')
    if r['type'] == 'inductor':
        return p.get('Shielding', '')
    return p.get('Fuse Type', '')


def round_robin(rows):
    by = collections.defaultdict(list)
    for r in sorted(rows, key=lambda r: (-r['stock'], r['mpn'])):
        by[r['series']].append(r)
    order = sorted(by, key=lambda s: (-by[s][0]['stock'], s))
    out = []
    depth = 0
    while len(out) < len(rows):
        for s in order:
            if depth < len(by[s]):
                out.append(by[s][depth])
        depth += 1
    return out


def main():
    args = sys.argv[1:]
    opts = {}
    for flag in ('--urls', '--stock', '--keep'):
        if flag in args:
            i = args.index(flag)
            opts[flag] = args[i + 1]
            del args[i:i + 2]
    report = '--report' in args
    args = [a for a in args if a != '--report']
    raw, out_path = args
    rows = common.in_scope(common.read_raw(raw))
    common.resolve_urls(rows)
    if '--urls' in opts:
        final = {}
        for l in list(open(opts['--urls'], encoding='utf-8'))[1:]:
            f = l.rstrip('\n').split('\t')
            if f[2] == '1':
                final[f[1]] = f[5] or f[1]
        for r in rows:
            if r['state']:
                continue
            if r['url'] in final:
                r['url'] = final[r['url']]
            else:
                r['state'], r['reason'] = 'out', 'no datasheet'
    checked = {}
    if '--stock' in opts:
        for l in list(open(opts['--stock'], encoding='utf-8'))[1:]:
            f = l.rstrip('\n').split('\t')
            checked[f[0]] = f[1] == '1'
        for r in rows:
            if not r['state'] and checked.get(r['mpn']) is False:
                r['state'], r['reason'] = 'out', 'stock check'
    keep = {}
    if '--keep' in opts:
        for f in csv.DictReader(open(opts['--keep'], encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE):
            keep[f['mpn']] = f
    picked = collections.Counter()
    for t in common.TYPES:
        for c in common.MAKERS[t]:
            order = {cls: round_robin([r for r in rows if not r['state'] and r['type'] == t
                                       and r['company'] == c and r['class'] == cls])
                     for cls, _ in common.QUOTA[t]}
            quota = {cls: min(q, len(order[cls])) for cls, q in common.QUOTA[t]}
            spare = common.PER_MAKER - sum(quota.values())
            for cls, _ in common.QUOTA[t]:
                more = min(spare, len(order[cls]) - quota[cls])
                quota[cls] += more
                spare -= more
            for cls, _ in common.QUOTA[t]:
                reserve = (quota[cls] + 3) // 4
                for i, r in enumerate(order[cls]):
                    r['rank'] = i + 1
                    if i < quota[cls]:
                        r['state'] = 'candidate'
                        picked[(t, c, cls)] += 1
                    elif i < quota[cls] + reserve:
                        r['state'] = 'reserve'
                    else:
                        r['state'], r['reason'] = 'out', 'below quota'
    with open(out_path, 'w', encoding='utf-8', newline='') as f:
        f.write('\t'.join(COLUMNS) + '\n')
        for r in sorted(rows, key=lambda r: (common.TYPES.index(r['type']), r['company'], r['class'], r['state'] == 'out',
                                             r.get('rank', 0), r['mpn'])):
            p = r['params']
            k = keep.get(r['mpn'], {})
            vals = [r['mpn'], r['company'], r['type'], r['class'], r['series'], r['status'], r['stock'], common.value(r),
                    common.form(r), p.get('Package / Case', ''), p.get('Supplier Device Package', ''),
                    p.get('Size / Dimension', ''), p.get('Mounting Type', ''), grade(p), detail(r), r['category'], r['piece'],
                    r['cut'], r['maker'], r['description'], r['via'], r['url'], r['state'], r['reason'],
                    r.get('rank', ''), k.get('pdf', ''), k.get('hit', ''), k.get('dead', ''),
                    ('1' if r['mpn'] in checked else ''), k.get('verdict', ''), k.get('vreason', '')]
            f.write('\t'.join(str(v).replace('\t', ' ') for v in vals) + '\n')
    if report:
        print('%d parts listed' % len(rows))
        state = collections.Counter(r['state'] for r in rows)
        why = collections.Counter(r['reason'] for r in rows if r['state'] == 'out')
        print('candidates: %d; reserve: %d; from a cut piece: %d of the candidates, %d of the download set' % (
            state['candidate'], state['reserve'], sum(r['state'] == 'candidate' and r['cut'] for r in rows),
            sum(r['state'] in ('candidate', 'reserve') and r['cut'] for r in rows)))
        print('out: ' + ', '.join('%s %d' % kv for kv in why.most_common()))
        for t in common.TYPES:
            for c in common.MAKERS[t]:
                print('%s %s: %s = %d' % (t, c, ', '.join('%s %d' % (cls, picked[(t, c, cls)]) for cls, _ in common.QUOTA[t]),
                                          sum(picked[(t, c, cls)] for cls, _ in common.QUOTA[t])))
        dl = [r for r in rows if r['state'] in ('candidate', 'reserve')]
        print('download set: %d rows; %d distinct URLs' % (len(dl), len({r['url'] for r in dl})))


if __name__ == '__main__':
    main()
