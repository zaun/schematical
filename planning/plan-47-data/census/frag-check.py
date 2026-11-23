#!/usr/bin/env python3
"""plan-47-B: check a fragment of specs rows against the download set, without touching any file.

usage: python3 -I frag-check.py <fragment.tsv> [--list]

<fragment.tsv> has the columns of specs.tsv (a header line, then rows keyed by
type, maker, series, pdf, package). Its rows overlay the same keys of specs.tsv;
every download-set row of those keys is judged with scheme-check.py's rules
(facts included) and every fragment row is checked with pinout-check.py's rules.
Prints, per maker and type, `<rows> rows: <ok> ok, <skip> skip, <out> out` with
the skip reasons, then the pinout errors. With --list, each row that is not ok.
Writes nothing.
"""
import collections
import importlib.util
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.append(HERE)
import specs47 as sp  # noqa: E402


def load(name, file):
    spec = importlib.util.spec_from_file_location(name, os.path.join(HERE, file))
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def main(argv):
    listing = '--list' in argv
    paths = [a for a in argv if not a.startswith('--')]
    if len(paths) != 1:
        print(__doc__)
        return 2
    scheme_check = load('scheme_check', 'scheme-check.py')
    pinout_check = load('pinout_check', 'pinout-check.py')
    frag = sp.read(paths[0])
    keys = {(s['type'], s['maker'], s['series'], s['pdf'], s['package']): s for s in frag}
    params = sp.params_by_mpn()
    counts = collections.defaultdict(collections.Counter)
    reasons = collections.defaultdict(collections.Counter)
    seen = set()
    for r in sp.read(sp.CANDIDATES):
        if not sp.in_set(r) or r['type'] not in sp.TYPES:
            continue
        k = sp.key(r)
        if k not in keys:
            continue
        seen.add(k)
        s = keys[k]
        c = counts[(r['company'], r['type'])]
        c['rows'] += 1
        if not s['scheme']:
            c['unchecked'] += 1
            continue
        v, reason = scheme_check.verdict(r, s, True, params.get(r['mpn'], {}))
        c[v] += 1
        if v != 'ok':
            reasons[(r['company'], r['type'])][reason] += 1
            if listing:
                print(v, reason, '|', r['mpn'], '|', r['value'], '|', s['scheme'])
    for k in sorted(counts):
        c = counts[k]
        print(f"{k[0]} {k[1]}: {c['rows']} rows: {c['ok']} ok, {c['skip']} skip, {c['out']} out, "
              f"{c['unchecked']} unchecked")
        for reason, n in sorted(reasons[k].items()):
            print(f"    {reason}: {n}")
    missing = [k for k in keys if k not in seen]
    for k in missing:
        print('fragment row with no download-set row:', '|'.join(k))
    bad = 0
    for s in frag:
        for e in pinout_check.check(s):
            bad += 1
            print(f"pinout {s['type']} {s['maker']} | {s['pdf']} | {s['package']}: {e}")
    print(f"{len(frag)} fragment rows: {bad} pinout errors")
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
