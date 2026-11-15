#!/usr/bin/env python3
"""plan-40-B (Corrections 1): mark the spec rows nobody read as `n/r`.

usage: python3 -I mark-unread.py [<specs.tsv>]

Every spec row whose `scheme` is still empty after the reading waves
(need-groups.py printed nothing more to read) gets scheme `n/r`: its rows
are below the pick, and verdicts.py makes them `out`, `below the pick (not
read)`; select.py --report then proves none of them is at or above its
(family, maker)'s floor or in a short share.
"""
import csv
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs  # noqa: E402


def main(path=specs.SPECS):
    rows = list(csv.DictReader(open(path), delimiter="\t"))
    n = 0
    for s in rows:
        if not s["scheme"]:
            s["scheme"] = "n/r"
            n += 1
    with open(path, "w") as out:
        out.write("\t".join(specs.COLS) + "\n")
        for s in rows:
            out.write("\t".join(s[c] for c in specs.COLS) + "\n")
    print(n, "spec rows marked n/r;", len(rows) - n, "read")


if __name__ == "__main__":
    main(*sys.argv[1:2])
