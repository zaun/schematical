"""plan-29-B–G: drop the spec rows no add row uses (their rows all became skip).

usage: python3 -I specs-prune.py
"""
import csv
import os

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
paths = {r["url"]: r["path"] for r in csv.DictReader(open(os.path.join(ROOT, "datasheets", "diode", "datasheets.tsv")), delimiter="\t")}
used = {(r["maker"], r["category"], paths.get(r["datasheet_url"], ""), r["supplier_package"])
        for r in csv.DictReader(open(os.path.join(DATA, "candidates.tsv")), delimiter="\t")
        if r["decision"] in ("add", "pending")}
spec = os.path.join(DATA, "specs.tsv")
rows = list(csv.DictReader(open(spec), delimiter="\t"))
cols = list(rows[0].keys())
keep = [s for s in rows if (s["maker"], s["category"], s["datasheet"], s["supplier_package"]) in used]
with open(spec, "w") as out:
    out.write("\t".join(cols) + "\n")
    for s in keep:
        out.write("\t".join(s[c] for c in cols) + "\n")
print(f"{len(rows) - len(keep)} spec rows dropped, {len(keep)} kept")
