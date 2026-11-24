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
    old, new, mpns = sys.argv[1], sys.argv[2], set(sys.argv[3].split(','))
    pdfs = set()

    def cand(rows):
        n = 0
        for r in rows:
            if r['mpn'] in mpns:
                if r['variant'] != old:
                    sys.exit(f"{r['mpn']} is in {r['variant']}, not {old}")
                r['variant'] = new
                pdfs.add((r['maker'], r['pdf']))
                n += 1
        return n

    n = rw(os.path.join(DATA, 'candidates.tsv'), cand)
    if n != len(mpns):
        sys.exit(f'{n} of {len(mpns)} rows found')

    def spec(rows):
        k = 0
        for s in rows:
            if s['variant'] == old and (s['maker'], s['pdf']) in pdfs:
                s['variant'] = new
                k += 1
        return k

    k = rw(os.path.join(DATA, 'specs.tsv'), spec)
    print(f'{n} rows and {k} spec rows moved to {new}')


main()
