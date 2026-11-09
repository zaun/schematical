"""plan-29 census, step 3: all.tsv -> cand.tsv (first-pass decision).

usage: python3 -I select.py <all.tsv> <cand.tsv>

Applies the plan-29-A section 3 rules that DigiKey's own data can decide.
`decision` becomes `candidate` (go on to the datasheet check), `check`
(DigiKey's mounting is blank, so the datasheet decides) or `out` with a
reason:

  stock     quantity available is 0
  mounting  not Surface Mount / Surface Mount, Wettable Flank / Through Hole
            (chassis, stud, press-fit and DIN-rail parts are out of scope)
  sic       Technology is SiC (Silicon Carbide) Schottky (out of scope)

Status is already Active or Last Time Buy: list.sh asks DigiKey for nothing
else.  Adds two columns: `leaf` (the drawer function, plan-29-A section 3)
and `flags` (automotive / military from DigiKey's Grade).
"""
import csv
import sys

IN_MOUNTING = {"Surface Mount", "Surface Mount, Wettable Flank", "Through Hole"}


def leaf(row):
    if row["category"] in ("zener", "zener-array"):
        return "Zener"
    if row["category"] == "bridge":
        return "Bridge"
    tech = row["technology"]
    if "Schottky" in tech or "Super Barrier" in tech or "FERD" in tech:
        return "Schottky"
    speed = row["speed"]
    if speed.startswith("Small Signal"):
        return "Switching"
    if "Fast" in speed or "Recovery =<" in speed:
        return "Fast recovery"
    return "Rectifier"


def flags(row):
    out = []
    if row["grade"] == "Automotive":
        out.append("automotive")
    if row["grade"] == "Military":
        out.append("military")
    return ",".join(out)


def decide(row):
    if int(row["qty"]) <= 0:
        return "out", "stock"
    if "SiC" in row["technology"]:
        return "out", "sic"
    if row["mounting"] in ("", "-"):
        return "check", "DigiKey mounting blank"
    if row["mounting"] not in IN_MOUNTING:
        return "out", "mounting"
    return "candidate", ""


def main(src, dst):
    rows = list(csv.DictReader(open(src), delimiter="\t"))
    cols = list(rows[0].keys()) + ["leaf", "flags"]
    with open(dst, "w") as out:
        out.write("\t".join(cols) + "\n")
        for row in rows:
            row["decision"], row["reason"] = decide(row)
            row["leaf"], row["flags"] = leaf(row), flags(row)
            out.write("\t".join(row[c] for c in cols) + "\n")
    counts = {}
    for row in rows:
        key = (row["decision"], row["reason"])
        counts[key] = counts.get(key, 0) + 1
    for key in sorted(counts):
        print(counts[key], key)


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
