"""plan-27 census, step 6: footprints-needed.txt from candidates.tsv.

usage: python3 -I footprint-lines.py <candidates.tsv>   (writes to stdout)

One line per (maker, DigiKey series, mounting) of the candidate rows that have
a datasheet PDF: parts, maker / series, mounting, DigiKey outline(s),
termination style(s).  An upper bound on the footprints to draw: series that
share a land pattern share one footprint (plan-27-B section 4.2).
"""
import collections
import csv
import sys

rows = [r for r in csv.DictReader(open(sys.argv[1]), delimiter="\t")
        if r["decision"] in ("candidate", "add", "pending") and r["pdf"] == "1"]
groups = collections.defaultdict(lambda: [0, set(), set()])
for r in rows:
    key = (r["maker"].split(" (")[0].split(" Inc")[0], r["series"], r["mounting"])
    groups[key][0] += 1
    groups[key][1].add(r["outline"])
    groups[key][2].add(r["termination"])
print("# plan-27 footprint proxy: one line per (maker, DigiKey series, mounting) of the candidate parts that have a datasheet PDF.")
print("# Columns: parts, maker / series, mounting, DigiKey outline(s), termination style(s).")
print("# Existing footprints that match: none (the only SW- footprints in footprints.json are the 15 DIP-switch ones).")
print("# This is an upper bound on the footprints to draw: series that share a land pattern share one footprint (plan-27-B section 4.2).")
for key in sorted(groups):
    n, outlines, terms = groups[key]
    print(f"{n:5}  {key[0]} / {key[1]}  |  {key[2]}  |  {'; '.join(sorted(outlines))}  |  {'; '.join(sorted(terms))}")
