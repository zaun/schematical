#!/usr/bin/env python3
"""plan-40-C/D/E: does every land-pattern line have a footprint?

usage: python3 -I fp-check.py <letter> [<footprints-needed.txt>]

Joins the letter's lines of footprints-needed.txt (letter, maker, mounting,
DigiKey Package / Case, groups, supplier packages `a | b`, …) with
planning/plan-40-data/footprint-map.tsv (columns letter, maker, package,
supplier, footprint, label, note: one row per land pattern; `supplier` is
one supplier device package of the line, or `*` for all of them). Every
supplier package of a line needs a map row, and every mapped id must be in
footprints-C.json, -D.json, -E.json or Parts/footprints.json. Prints `<n>
lines, <m> without a footprint` and each missing line or unknown id.
"""
import csv
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
MAP = os.path.join(DATA, "footprint-map.tsv")


def known_ids():
    ids = set(re.findall(r'"id":"([^"]+)"', open(os.path.join(ROOT, "schematical", "schematical", "Parts",
                                                               "footprints.json")).read()))
    for f in ("footprints-C.json", "footprints-D.json", "footprints-E.json"):
        p = os.path.join(DATA, f)
        if os.path.exists(p):
            ids |= {o["id"] for o in json.load(open(p))["footprints"]}
    return ids


def load_map():
    fmap = {}
    if os.path.exists(MAP):
        for r in csv.DictReader(open(MAP), delimiter="\t"):
            fmap[(r["letter"], r["maker"], r["package"], r["supplier"])] = r
    return fmap


def lookup(fmap, letter, maker, package, supplier):
    return fmap.get((letter, maker, package, supplier)) or fmap.get((letter, maker, package, "*"))


def main():
    letter = sys.argv[1]
    needed = sys.argv[2] if len(sys.argv) > 2 else os.path.join(DATA, "footprints-needed.txt")
    ids = known_ids()
    fmap = load_map()
    n = missing = 0
    for line in open(needed):
        if line.startswith("#") or not line.strip():
            continue
        f = line.rstrip("\n").split("\t")
        if f[0] != letter:
            continue
        n += 1
        bad = []
        for sup in [s.strip() for s in f[5].split("|")]:
            m = lookup(fmap, f[0], f[1], f[3], sup)
            if not m or not m["footprint"]:
                bad.append(f"no footprint for {sup}")
            elif m["footprint"] not in ids:
                bad.append(f"unknown id {m['footprint']} for {sup}")
        if bad:
            missing += 1
            print("; ".join(bad) + ":", f[1], "|", f[3], "|", f[5])
    print(f"{n} lines, {missing} without a footprint")
    return 1 if missing else 0


if __name__ == "__main__":
    sys.exit(main())
