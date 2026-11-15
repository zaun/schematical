#!/usr/bin/env python3
"""plan-40-B: merge a hand-written maker file into specs.tsv.

usage: python3 -I specs-fill.py <maker file.tsv> [<specs.tsv>]

The maker file has the columns `pdf`, `package`, `series` (may be `*`: every
series of that PDF and package), `scheme`, `label`, `pinout`, `elements`
(extra columns, e.g. `note`, are ignored). Each of its rows fills the scheme
columns of every specs.tsv row with the same pdf and package (and series,
unless `*`); a row of the file that fills no specs row is an error, as is a
specs row filled twice with different values.
"""
import csv
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs  # noqa: E402

FILL = ["scheme", "label", "pinout", "elements"]


def main(src, dst=specs.SPECS):
    rows = list(csv.DictReader(open(dst), delimiter="\t"))
    done = {}
    bad = 0
    for f in csv.DictReader(open(src), delimiter="\t"):
        hit = 0
        for s in rows:
            if s["pdf"] != f["pdf"] or s["package"] != f["package"]:
                continue
            if f["series"] not in ("*", s["series"]):
                continue
            k = specs.key(s)
            new = [f[c] for c in FILL]
            if k in done and done[k] != new:
                print("filled twice:", "|".join(k))
                bad += 1
                continue
            done[k] = new
            for c, v in zip(FILL, new):
                s[c] = v
            hit += 1
        if not hit:
            print("fills nothing:", f["pdf"], "|", f["package"], "|", f["series"])
            bad += 1
    with open(dst, "w") as out:
        out.write("\t".join(specs.COLS) + "\n")
        for s in rows:
            out.write("\t".join(s[c] for c in specs.COLS) + "\n")
    print(len(done), "spec rows filled;", bad, "errors")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:3]))
