#!/usr/bin/env python3
"""plan-34-C/D: does every land-pattern line have a footprint?

usage: python3 -I fp-check.py <category> [<footprints-needed.txt>]

Joins the category's lines of footprints-needed.txt (category, maker,
datasheet, package, body) with planning/plan-34-data/footprint-map.tsv
(the same five columns, then footprint, part_package, pinout) and checks
each mapped footprint id against footprints-C.json, footprints-D.json and
Parts/footprints.json. Prints `<n> lines, <m> without a footprint` and each
missing line; a map row whose id is in none of the files is `unknown id`.
"""
import csv
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
cat = sys.argv[1]
needed = sys.argv[2] if len(sys.argv) > 2 else os.path.join(DATA, "footprints-needed.txt")
ids = set(re.findall(r'"id":"([^"]+)"', open(os.path.join(ROOT, "schematical", "schematical", "Parts",
                                                           "footprints.json")).read()))
for f in ("footprints-C.json", "footprints-D.json"):
    p = os.path.join(DATA, f)
    if os.path.exists(p):
        ids |= {o["id"] for o in json.load(open(p))["footprints"]}
fmap = {}
mp = os.path.join(DATA, "footprint-map.tsv")
if os.path.exists(mp):
    for r in csv.DictReader(open(mp), delimiter="\t"):
        fmap[(r["category"], r["maker"], r["datasheet"], r["package"], r["body"])] = r
n = missing = 0
for line in open(needed):
    if line.startswith("#"):
        continue
    f = line.rstrip("\n").split("\t")
    if f[0] != cat:
        continue
    n += 1
    m = fmap.get(tuple(f[:5]))
    if not m or not m["footprint"]:
        missing += 1
        print("no footprint:", "\t".join(f))
    elif m["footprint"] not in ids:
        missing += 1
        print("unknown id", m["footprint"] + ":", "\t".join(f))
print(f"{n} lines, {missing} without a footprint")
sys.exit(1 if missing else 0)
