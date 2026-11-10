"""plan-29-A Phase 5: the specs.tsv skeleton.

usage: python3 -I specs-skeleton.py <candidates.tsv> <specs.tsv>

One row per distinct (maker, category, datasheet, supplier package) of the
`add` and `pending` rows; `datasheet` is the PDF's path (datasheets/diode/
datasheets.tsv).  mpn_regex is `.*` for a group with no pending row, empty
for the others (B-G write the ordering-code key, plan-29-B section 4.5); the
drawing columns (package pinout pins footprint vr_io_checked note) are empty.
Refuses to overwrite a specs.tsv that already has a filled row.
"""
import csv
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
PLACED = os.path.join(ROOT, "datasheets", "diode", "datasheets.tsv")
COLS = ["maker", "category", "datasheet", "supplier_package", "mpn_regex", "package", "pinout",
        "pins", "footprint", "vr_io_checked", "note"]


def main(src, dst):
    if os.path.exists(dst):
        for r in csv.DictReader(open(dst), delimiter="\t"):
            if r["package"] or r["pins"] or r["footprint"]:
                print("refused: specs.tsv already has filled rows")
                return 1
    paths = {r["url"]: r["path"] for r in csv.DictReader(open(PLACED), delimiter="\t")}
    groups = {}
    for r in csv.DictReader(open(src), delimiter="\t"):
        if r["decision"] not in ("add", "pending"):
            continue
        k = (r["maker"], r["category"], paths[r["datasheet_url"]], r["supplier_package"])
        groups[k] = groups.get(k, False) or r["decision"] == "pending"
    with open(dst, "w") as out:
        out.write("\t".join(COLS) + "\n")
        for k in sorted(groups):
            out.write("\t".join(list(k) + ["" if groups[k] else ".*"] + [""] * 6) + "\n")
    print(len(groups), "rows")
    return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:3]))
