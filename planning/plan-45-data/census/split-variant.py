#!/usr/bin/env python3
"""plan-45-C/D/E: give rows their own variant (footprint family) when a datasheet shows they do not share a land pattern.

usage: python3 -I split-variant.py <old variant> <new variant> <mpn>[,<mpn>...]

A's variant key (maker, series, pitch, orientation, mounting, clamp class, ...) can merge housings that
the maker draws differently under one DigiKey series (the Weidmuller `Omnimate LM` series holds `LM` and
`LM1N` blocks). This sets `variant` of the named `add` rows of candidates.tsv to <new variant> and moves their
specs.tsv rows (same maker and PDF) to it. The rows must all be in <old variant>. Then rerun
`footprint-lines.py candidates.tsv generic-reps.tsv > footprints-needed.txt`.
"""
import csv
import os
import sys

DATA = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def rw(path, fn):
    rows = list(csv.DictReader(open(path), delimiter='\t'))
    cols = list(rows[0].keys())
    n = fn(rows)
    with open(path, 'w') as out:
        out.write('\t'.join(cols) + '\n')
        for r in rows:
            out.write('\t'.join(str(r[c]) for c in cols) + '\n')
    return n


def main():
    if sys.argv[1] == '--sync-specs':          # only the specs.tsv step (the candidates were split already)
        old = new = 'the rows already split'
        n = 0
    else:
        old, new, mpns = sys.argv[1], sys.argv[2], set(sys.argv[3].split(','))

        def cand(rows):
            n = 0
            for r in rows:
                if r['mpn'] in mpns:
                    if r['variant'] != old:
                        sys.exit(f"{r['mpn']} is in {r['variant']}, not {old}")
                    r['variant'] = new
                    n += 1
            return n

        n = rw(os.path.join(DATA, 'candidates.tsv'), cand)
        if n != len(mpns):
            sys.exit(f'{n} of {len(mpns)} rows found')

    # specs.tsv has one row per (maker, variant, PDF) of the rows in play: a PDF that serves several variants (the
    # JST sheets) needs a row per variant, so the spec rows are synced from the candidates, not renamed: a key a row
    # needs and has no spec row gets a copy of a sibling's (same maker and PDF); a spec row no row needs is dropped.
    needed = set()
    for r in csv.DictReader(open(os.path.join(DATA, 'candidates.tsv')), delimiter='\t'):
        if r['state'] in ('candidate', 'add'):
            needed.add((r['maker'], r['variant'], r['pdf']))
    stats = {'copied': 0, 'dropped': 0}

    def spec(rows):
        have = {(s['maker'], s['variant'], s['pdf']) for s in rows}
        for key in sorted(needed - have):
            sib = next((s for s in rows if (s['maker'], s['pdf']) == (key[0], key[2])), None)
            if sib is None:
                sys.exit(f'no spec row to copy for {key}')
            copy = dict(sib)
            copy['variant'] = key[1]
            rows.append(copy)
            stats['copied'] += 1
        keep = [s for s in rows if (s['maker'], s['variant'], s['pdf']) in needed]
        stats['dropped'] = len(rows) - len(keep)
        rows[:] = keep
        return len(keep)

    k = rw(os.path.join(DATA, 'specs.tsv'), spec)
    print(f"{n} rows moved to {new}; specs.tsv: {stats['copied']} spec rows copied, {stats['dropped']} dropped, {k} left")


main()
