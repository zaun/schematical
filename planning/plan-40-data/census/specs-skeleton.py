#!/usr/bin/env python3
"""plan-40-A Phase 3: the specs.tsv skeleton.

usage: python3 -I specs-skeleton.py <candidates.tsv> <specs.tsv>

One row per distinct (maker, series, pdf, DigiKey Package / Case) of the
eligible rows with a PDF (`pdf`, the path place.py wrote): `rows` the
number of eligible rows in the group, `example` its most-stocked part
number. The scheme columns `scheme`, `label`, `pinout`, `elements` are empty
(plan-40-B fills them). Run again over an existing specs.tsv it merges: a
group's existing scheme columns are kept, a new group gets empty ones, and a
row whose group is gone is dropped and named.
"""
import collections
import csv
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

COLS = ["maker", "series", "pdf", "package", "rows", "example", "scheme", "label", "pinout", "elements"]
KEEP = ["scheme", "label", "pinout", "elements"]


def main(src, dst):
    old = {}
    if os.path.exists(dst):
        for r in csv.DictReader(open(dst), delimiter="\t"):
            old[(r["maker"], r["series"], r["pdf"], r["package"])] = r
    groups = collections.defaultdict(list)
    for r in csv.DictReader(open(src), delimiter="\t"):
        if common.eligible(r) and r["pdf"]:
            groups[(r["maker"], r["series"], r["pdf"], r["package"])].append(r)
    with open(dst, "w") as out:
        out.write("\t".join(COLS) + "\n")
        for g in sorted(groups):
            rs = groups[g]
            example = max(rs, key=lambda r: (int(r["stock"] or 0), r["mpn"]))["mpn"]
            keep = [old[g][c] for c in KEEP] if g in old else [""] * len(KEEP)
            out.write("\t".join(list(g) + [str(len(rs)), example] + keep) + "\n")
    for g in sorted(set(old) - set(groups)):
        print("dropped", "|".join(g))
    print(len(groups), "rows;", sum(1 for g in groups if g in old), "kept from before")
    return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:3]))
