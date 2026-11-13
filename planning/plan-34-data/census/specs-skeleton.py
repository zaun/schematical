#!/usr/bin/env python3
"""plan-34-A Phase 4: the specs.tsv skeleton.

usage: python3 -I specs-skeleton.py <candidates.tsv> <specs.tsv>

One row per distinct (maker, series, datasheet) of the eligible rows with a
PDF (`pdf`, the path place.py wrote). The scheme columns are empty:
mpn_regex and fields_checked (plan-34-B), package, pinout and footprint
(plan-34-C, D); note is free text. Refuses to overwrite a specs.tsv that
already has a filled row.
"""
import csv
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

COLS = ["maker", "series", "datasheet", "mpn_regex", "fields_checked", "package", "pinout",
        "footprint", "note"]


def main(src, dst):
    if os.path.exists(dst):
        for r in csv.DictReader(open(dst), delimiter="\t"):
            if any(r[c] for c in COLS[3:]):
                print("refused: specs.tsv already has filled rows")
                return 1
    groups = sorted({(r["maker"], r["series"], r["pdf"])
                     for r in csv.DictReader(open(src), delimiter="\t")
                     if common.eligible(r) and r["pdf"]})
    with open(dst, "w") as out:
        out.write("\t".join(COLS) + "\n")
        for g in groups:
            out.write("\t".join(list(g) + [""] * (len(COLS) - 3)) + "\n")
    print(len(groups), "rows")
    return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:3]))
