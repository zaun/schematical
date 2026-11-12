"""plan-33-B Phase 3: drop the specs.tsv rows of PDFs place.py --prune deleted.

usage: python3 -I specs-prune.py [specs.tsv]

Keeps the rows whose `datasheet` is still under datasheets/capacitor/ and
prints how many went and how many kept rows have no scheme (must be 0: every
PDF an `add` row cites was read, plan-33-B section 1).
"""
import csv
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))


def main(path):
    with open(path, newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f, delimiter="\t")
        cols = reader.fieldnames
        rows = list(reader)
    kept = [r for r in rows if os.path.exists(os.path.join(ROOT, r["datasheet"]))]
    with open(path, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, cols, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(kept)
    empty = sum(1 for r in kept if not r["mpn_regex"] or not r["cap_group"] or not r["volt_map"])
    print(f"{len(rows) - len(kept)} rows dropped, {len(kept)} kept, {empty} kept rows without a scheme")
    return 1 if empty else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1] if len(sys.argv) > 1 else os.path.join(DATA, "specs.tsv")))
