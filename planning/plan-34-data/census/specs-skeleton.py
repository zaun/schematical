#!/usr/bin/env python3
"""plan-34-A Phase 4: the specs.tsv skeleton.

usage: python3 -I specs-skeleton.py <candidates.tsv> <specs.tsv>

One row per distinct (maker, series, datasheet) of the eligible rows with a
PDF (`pdf`, the path place.py wrote). The scheme columns are empty:
mpn_regex and fields_checked (plan-34-B), package, pinout and footprint
(plan-34-C, D); note is free text. Run again over an existing specs.tsv it
merges: a group's existing row is kept as it is, a new group gets an empty
row, and a row whose group is gone is dropped and named (plan-34-A
§ Corrections 6: Phases 3–4 are re-run after the stock check).
"""
import csv
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

COLS = ["maker", "series", "datasheet", "mpn_regex", "fields_checked", "package", "pinout",
        "footprint", "note"]


def main(src, dst):
    old = {}
    if os.path.exists(dst):
        for r in csv.DictReader(open(dst), delimiter="\t"):
            old[(r["maker"], r["series"], r["datasheet"])] = [r[c] for c in COLS]
    groups = sorted({(r["maker"], r["series"], r["pdf"])
                     for r in csv.DictReader(open(src), delimiter="\t")
                     if common.eligible(r) and r["pdf"]})
    with open(dst, "w") as out:
        out.write("\t".join(COLS) + "\n")
        for g in groups:
            out.write("\t".join(old.get(g, list(g) + [""] * (len(COLS) - 3))) + "\n")
    for g in sorted(set(old) - set(groups)):
        print("dropped", "|".join(g))
    print(len(groups), "rows;", sum(1 for g in groups if g in old), "kept from before")
    return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:3]))
