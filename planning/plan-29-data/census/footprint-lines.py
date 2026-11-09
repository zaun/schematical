"""plan-29 census, step 6: footprints-needed.txt from a candidates table.

usage: python3 -I footprint-lines.py <candidates.tsv>   (writes to stdout)

One line per (maker, DigiKey supplier device package, mounting) of the
candidate / add / pending rows that have a datasheet PDF: parts, maker,
supplier package, mounting, DigiKey "Package / Case" value(s), sub-type
leaves.  An upper bound on the footprints to draw: packages whose drawings
give the same land pattern share one footprint (plan-29-B section 4).
"""
import collections
import csv
import sys

SHORT = {"Vishay General Semiconductor - Diodes Division": "Vishay",
         "Taiwan Semiconductor Corporation": "TSC", "onsemi": "onsemi"}

rows = [r for r in csv.DictReader(open(sys.argv[1]), delimiter="\t")
        if r["decision"] in ("candidate", "check", "add", "pending") and r["pdf"] == "1"]
groups = collections.defaultdict(lambda: [0, set(), set()])
for r in rows:
    key = (SHORT.get(r["maker"], r["maker"]), r["supplier_package"], r["mounting"])
    groups[key][0] += 1
    groups[key][1].add(r["package"])
    groups[key][2].add(r["leaf"])
pkgs = collections.Counter()
for (maker, sp, mount), (n, _, _) in groups.items():
    pkgs[sp] += n
print("# plan-29 footprint proxy: one line per (maker, DigiKey supplier device package, mounting)")
print("# of the candidate parts that have a datasheet PDF.")
print("# Columns: parts, maker / supplier package, mounting, DigiKey Package / Case value(s), drawer leaves.")
print("# Existing footprints that match: none (footprints.json has no diode package; RES-AXIAL-10.16 is a")
print("# resistor land pattern, not taken from a diode drawing).")
print(f"# {len(groups)} lines, {len(pkgs)} distinct supplier packages, {sum(pkgs.values())} parts.")
for key in sorted(groups, key=lambda k: (-groups[k][0], k)):
    n, cases, leaves = groups[key]
    print(f"{n:5}  {key[0]} / {key[1]}  |  {key[2]}  |  {'; '.join(sorted(cases))}  |  {', '.join(sorted(leaves))}")
