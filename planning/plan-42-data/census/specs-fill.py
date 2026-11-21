#!/usr/bin/env python3
"""plan-42-B / C: merge a hand-written maker file into specs.tsv.

usage: python3 -I specs-fill.py <maker file.tsv> [<specs.tsv>]

The maker file has the columns `pdf`, `package`, `series` (may be `*`: every
series of that PDF and package) and any of the fill columns `scheme`,
`label` (B), `pinout`, `pinout_adj`, `regulators`, `analog`, `vref`,
`vin_min`, `dropout`, `adjust_to` (C); other columns (e.g. `note`) are
ignored. Each of its rows fills the columns it has, for every specs.tsv row
with the same pdf and package (and series, unless `*`); a row of the file that
fills no specs row is an error, as is a specs row filled twice with different
values.
"""
import csv
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs  # noqa: E402

FILL = specs.B_COLS + specs.C_COLS


def main(src, dst=specs.SPECS):
    rows = list(csv.DictReader(open(dst), delimiter="\t"))
    reader = csv.DictReader(open(src), delimiter="\t")
    cols = [c for c in FILL if c in reader.fieldnames]
    done = {}
    bad = 0
    for f in reader:
        hit = 0
        for s in rows:
            if s["pdf"] != f["pdf"] or s["package"] != f["package"]:
                continue
            if f["series"] not in ("*", s["series"]):
                continue
            k = specs.key(s)
            new = {c: f[c] for c in cols}
            if k in done and any(done[k].get(c, new[c]) != new[c] for c in cols):
                print("filled twice:", "|".join(k))
                bad += 1
                continue
            done.setdefault(k, {}).update(new)
            s.update(new)
            hit += 1
        if not hit:
            print("fills nothing:", f["pdf"], "|", f["package"], "|", f["series"])
            bad += 1
    specs.write(rows, dst)
    print(len(done), "spec rows filled;", bad, "errors")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:3]))
