#!/usr/bin/env python3
"""plan-54-H to K: the footprint reader's worksheet.

usage: python3 -I fp-brief.py <letter> [--family F] [--maker M] [--stems stem,stem,...] [--slice i/n]

For each line of footprints-needed.txt of the letter (the selection by family, maker, stems or the i-th of n equal
slices of the sorted lines), prints:

  == <stem>   <maker> | <series> | <mounting> | <card> | pitch <pitch> | rows <rows> | positions <list> | <status>
     pdf <path>      parts <n> (example <mpn>)
     N=<positions>: pins <P>  <pin list>   (the pinout of the `add` parts with that count: `1=NAME,2=NAME,...`; `seq` / `AB` expanded)
"""
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs  # noqa: E402


def main(argv):
    letter = argv[0]
    fam = maker = None
    stems = None
    sl = None
    i = 1
    while i < len(argv):
        if argv[i] == '--family':
            fam = argv[i + 1]
        elif argv[i] == '--maker':
            maker = argv[i + 1]
        elif argv[i] == '--stems':
            stems = set(argv[i + 1].split(','))
        elif argv[i] == '--slice':
            sl = tuple(int(x) for x in argv[i + 1].split('/'))
        i += 2
    table = specs.load()
    cand = [r for r in specs.candidates() if r['state'] == 'add']
    lines = []
    for line in open(os.path.join(specs.DATA, 'footprints-needed.txt')):
        if line.startswith('#') or not line.strip():
            continue
        f = line.rstrip('\n').split('\t')
        if f[0] != letter or (fam and f[1] != fam) or (maker and f[2] != maker) or (stems and f[13] not in stems):
            continue
        lines.append(f)
    lines.sort(key=lambda f: f[13])
    if sl:
        k, n = sl
        size = -(-len(lines) // n)
        lines = lines[(k - 1) * size:k * size]
    for f in lines:
        print(f"== {f[13]}   {f[2]} | {f[3]} | {f[4]} | {f[5]} | pitch {f[6]} | rows {f[7]} | positions {f[8]} | {f[14]}")
        print(f"   pdf {f[11]}      parts {f[9]} (example {f[10]})")
        parts = [r for r in cand if r['maker'] == f[2] and r['variant'] == f[15] and r['pdf'] == f[11]]
        for N in [int(x) for x in f[8].split(',')]:
            seen = set()
            for r in parts:
                if int(r['positions']) != N:
                    continue
                v, _, _, spec = specs.verdict(r, table.get(specs.key(r)))
                n = specs.pin_count(r, spec)
                names = specs.pin_names(spec['pinout'], n)
                key = (n, spec['pinout'])
                if key in seen:
                    continue
                seen.add(key)
                text = ','.join(f'{j}={nm}' for j, nm in enumerate(names or [], 1)) if names else spec['pinout']
                if len(text) > 400:
                    text = text[:400] + f' ... ({n} pins)'
                print(f"   N={N}: pins {n}  {text}")


main(sys.argv[1:])
