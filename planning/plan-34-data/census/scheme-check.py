#!/usr/bin/env python3
"""plan-34-B Phase 1: one spec row's regex against its eligible rows.

usage: python3 -I scheme-check.py <specs.tsv> <candidates.tsv> <spec row number (1 = first data row)> [<regex>]

Prints the row's (maker, series, PDF), then for its eligible rows that are
not printed verbatim: each non-match, each match whose fields disagree with
DigiKey (scheme.py), and the counts. A regex given on the command line is
tried in place of the row's own.
"""
import csv
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402
import scheme  # noqa: E402

specs = list(csv.DictReader(open(sys.argv[1]), delimiter="\t"))
row = specs[int(sys.argv[3]) - 1]
regex = sys.argv[4] if len(sys.argv) > 4 else row["mpn_regex"]
print(row["maker"], "|", row["series"], "|", row["datasheet"], "|", regex)
ok = bad = miss = hits = 0
for r in csv.DictReader(open(sys.argv[2]), delimiter="\t"):
    if not common.eligible(r) or (r["maker"], r["series"], r["pdf"]) != (row["maker"], row["series"], row["datasheet"]):
        continue
    if r.get("hit") == "1":
        hits += 1
        continue
    m = re.fullmatch(regex, r["mpn"]) if regex else None
    if not m:
        miss += 1
        print("  no match", r["mpn"], r["package"], r["tolerance"], r["ohms"])
        continue
    why = scheme.check(m, r, row["fields_checked"])
    if why:
        bad += 1
        print("  disagrees", why, r["mpn"], r["package"], r["tolerance"], r["ohms"])
    else:
        ok += 1
print(f"{ok} match, {bad} disagree, {miss} no match, {hits} printed verbatim")
