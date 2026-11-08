"""plan-27 census, step 3: all.tsv -> candidates.tsv (first-pass decision).

usage: python3 -I select.py <all.tsv> <candidates.tsv>

Applies the plan-27-A section 3 rules that DigiKey's own data can decide, and
leaves `decision` = `candidate` (go on to the datasheet check), `check` (the
DigiKey parameters are blank, so the datasheet decides) or `out` with a reason:

  stock        quantity available is 0
  circuit      Circuit is not SPST-NO / SPST-NC (SPDT, SPST x 2, ...)
  lit          Illumination is Illuminated
  mounting     Panel Mount or Through Hole, Reverse (not one of the in-scope
               mountings: surface mount, through hole, right angle, edge mount)

Status is already Active or Last Time Buy: list.sh asks DigiKey for nothing else.
Phase A then adds the datasheet checks (the part is in the ordering table, the
datasheet is the maker's own) and rewrites `decision` to `add` or `skip`.
"""
import csv
import sys

IN_MOUNTING = {"Surface Mount", "Through Hole", "Through Hole, Right Angle",
               "Surface Mount, Right Angle", "Edge Mount, Right Angle"}


def decide(row):
    if int(row["qty"]) <= 0:
        return "out", "stock"
    if row["circuit"] in ("", "-") and row["mounting"] in ("", "-"):
        return "check", "DigiKey parameters blank"
    if row["circuit"] not in ("SPST-NO", "SPST-NC"):
        return "out", "circuit"
    if row["illumination"] == "Illuminated":
        return "out", "lit"
    if row["mounting"] not in IN_MOUNTING:
        return "out", "mounting"
    return "candidate", ""


def main(src, dst):
    rows = list(csv.DictReader(open(src), delimiter="\t"))
    cols = list(rows[0].keys())
    with open(dst, "w") as out:
        out.write("\t".join(cols) + "\n")
        for row in rows:
            row["decision"], row["reason"] = decide(row)
            out.write("\t".join(row[c] for c in cols) + "\n")
    counts = {}
    for row in rows:
        key = (row["decision"], row["reason"])
        counts[key] = counts.get(key, 0) + 1
    for key in sorted(counts):
        print(counts[key], key)


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
