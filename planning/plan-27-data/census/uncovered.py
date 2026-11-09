"""plan-27-B/C: which add / pending rows no reader has covered yet.

usage: python3 -I uncovered.py [--tsv candidates.tsv] [--maker M]... [--rows]

A row is covered when its (maker, series) has a scheme (pending rows) and an
`add` row (or a pending row its scheme accepts) matches exactly one spec row
or is listed in a work/<slug>/cannot-draw.tsv (mpn, reason)
of its (maker, series, mounting), in the work/<slug>/ files of its maker
(planning/plan-27-data/work/*/schemes.tsv, specs.tsv). Prints per maker and
series: the uncovered count, and the part numbers with --rows.
"""
import argparse
import collections
import csv
import glob
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)


def read(path):
    return list(csv.DictReader(open(path), delimiter="\t")) if os.path.exists(path) else []


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--tsv", default=os.path.join(DATA, "candidates.tsv"))
    ap.add_argument("--maker", action="append", default=[])
    ap.add_argument("--rows", action="store_true")
    a = ap.parse_args()
    schemes = collections.defaultdict(list)
    specs = collections.defaultdict(list)
    for d in glob.glob(os.path.join(DATA, "work", "*")):
        for s in read(os.path.join(d, "schemes.tsv")):
            schemes[(s["maker"], s["series"])].append(s["regex"])
        for s in read(os.path.join(d, "specs.tsv")):
            specs[(s["maker"], s["series"], s["mounting"])].append(s["mpn_regex"])
    checks = {}
    cannot = set()
    for d in glob.glob(os.path.join(DATA, "work", "*")):
        for c in read(os.path.join(d, "checks.tsv")):
            checks[c["mpn"]] = c
        cannot.update(c["mpn"] for c in read(os.path.join(d, "cannot-draw.tsv")))
    out = collections.defaultdict(list)
    for r in read(a.tsv):
        if r["decision"] not in ("add", "pending"):
            continue
        if a.maker and not any(r["maker"].startswith(m) for m in a.maker):
            continue
        if r["mpn"] in cannot:
            continue  # a reader found no drawing: covered (becomes skip at merge)
        mounting = r["mounting"]
        if r["reason"].startswith("check"):
            c = checks.get(r["mpn"])
            if c is None:
                out[(r["maker"], r["series"])].append(r["mpn"] + " (check row unread)")
                continue
            if c["decision"] == "out":
                continue
            mounting = c["mounting"]
        if r["decision"] == "pending":
            regs = schemes.get((r["maker"], r["series"]))
            if not regs:
                out[(r["maker"], r["series"])].append(r["mpn"] + " (no scheme)")
                continue
            if not any(re.fullmatch(x, r["mpn"]) for x in regs):
                continue  # a skip: covered
        n = sum(1 for x in specs.get((r["maker"], r["series"], mounting), []) if re.fullmatch(x, r["mpn"]))
        if n != 1:
            out[(r["maker"], r["series"])].append(f"{r['mpn']} ({n} spec rows, {mounting})")
    total = 0
    for k in sorted(out):
        total += len(out[k])
        print(f"{len(out[k]):4}  {k[0].split(' (')[0]} / {k[1]}")
        if a.rows:
            for m in out[k]:
                print("        " + m)
    print(f"{total} uncovered rows")


if __name__ == "__main__":
    main()
