"""plan-27-A Phase 4: the specs.tsv skeleton.

usage: python3 -I specs-skeleton.py <candidates.tsv> <specs.tsv>

One row per distinct (maker, series, mounting) of the `add` and `pending`
rows: mpn_regex `.*`, terminals / pins / footprint / note empty (plan-27-B and
C fill them; formats in plan-27-B section 4.3).  Refuses to overwrite a
specs.tsv that already has a filled row.
"""
import csv
import os
import sys

COLS = ["maker", "series", "mounting", "mpn_regex", "terminals", "pins", "footprint", "note"]


def main(src, dst):
    if os.path.exists(dst):
        for r in csv.DictReader(open(dst), delimiter="\t"):
            if r["terminals"] or r["pins"] or r["footprint"]:
                print("refused: specs.tsv already has filled rows")
                return 1
    keys = sorted({(r["maker"], r["series"], r["mounting"])
                   for r in csv.DictReader(open(src), delimiter="\t")
                   if r["decision"] in ("add", "pending")})
    with open(dst, "w") as out:
        out.write("\t".join(COLS) + "\n")
        for k in keys:
            out.write("\t".join([k[0], k[1], k[2], ".*", "", "", "", ""]) + "\n")
    print(len(keys), "rows")
    return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:3]))
