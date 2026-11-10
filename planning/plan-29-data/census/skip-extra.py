"""plan-29-B–L: skip add rows for reasons found while reading datasheets.

usage: python3 -I skip-extra.py

work/skip-extra.tsv (mpn, maker, reason): each named add row (of that maker
when `maker` is given) becomes skip with that reason (`out` when the reason starts with "out: ").  Then run specs-prune.py and decide.py --skipped.
"""
import csv
import os

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
cand = os.path.join(DATA, "candidates.tsv")
extra = {(r["mpn"], r["maker"]): r["reason"] for r in csv.DictReader(open(os.path.join(DATA, "work", "skip-extra.tsv")), delimiter="\t")}
rows = list(csv.DictReader(open(cand), delimiter="\t"))
cols = list(rows[0].keys())
n = 0
for r in rows:
    reason = extra.get((r["mpn"], r["maker"])) or extra.get((r["mpn"], ""))
    if reason and r["decision"] == "add":
        if reason.startswith("out: "):
            r["decision"], r["reason"] = "out", reason[5:]
        else:
            r["decision"], r["reason"] = "skip", reason
        n += 1
with open(cand, "w") as out:
    out.write("\t".join(cols) + "\n")
    for r in rows:
        out.write("\t".join(r[c] for c in cols) + "\n")
done = {(r["mpn"], m) for r in rows if r["decision"] in ("skip", "out") for m in (r["maker"], "")}
missing = sorted(k for k in extra if k not in done)
print(f"{n} rows skipped; not found: {missing}")
