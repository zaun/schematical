"""plan-29-B–L: skip add rows for reasons found while reading datasheets.

usage: python3 -I skip-extra.py

work/skip-extra.tsv (mpn, reason): each named add row becomes skip with that
reason.  Then run specs-prune.py and decide.py --skipped.
"""
import csv
import os

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
cand = os.path.join(DATA, "candidates.tsv")
extra = {r["mpn"]: r["reason"] for r in csv.DictReader(open(os.path.join(DATA, "work", "skip-extra.tsv")), delimiter="\t")}
rows = list(csv.DictReader(open(cand), delimiter="\t"))
cols = list(rows[0].keys())
n = 0
for r in rows:
    if r["mpn"] in extra and r["decision"] == "add":
        r["decision"], r["reason"] = "skip", extra[r["mpn"]]
        n += 1
with open(cand, "w") as out:
    out.write("\t".join(cols) + "\n")
    for r in rows:
        out.write("\t".join(r[c] for c in cols) + "\n")
missing = sorted(set(extra) - {r["mpn"] for r in rows if r["decision"] == "skip"})
print(f"{n} rows skipped; not found: {missing}")
