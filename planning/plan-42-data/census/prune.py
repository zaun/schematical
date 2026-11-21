#!/usr/bin/env python3
"""plan-42-B Phase 5: the PDFs that stay in datasheets/regulator/.

usage: python3 -I prune.py [--dry-run]

Keeps each PDF an `add` row of candidates.tsv uses (plan-42-D / E / F may cite
a kept PDF as the source of a footprint drawing; they add no PDF). Deletes
every other file of datasheets/regulator/<maker>/ and rewrites datasheets.tsv
to the kept files' lines. Prints the counts.
"""
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

DATA = common.DATA


def main():
    dry = "--dry-run" in sys.argv
    files = set()
    for slug in common.SLUG.values():
        d = os.path.join(common.TREE, slug)
        for name in os.listdir(d) if os.path.isdir(d) else []:
            files.add(os.path.join("datasheets", "regulator", slug, name))
    keep = {r["pdf"] for r in common.load(os.path.join(DATA, "candidates.tsv"))
            if r["reason"] == "add" and r["pdf"]}
    missing = sorted(p for p in keep if p not in files)
    for p in missing:
        print("missing (used but not in the tree):", p)
    drop = sorted(files - keep)
    print(f"{len(files)} files; {len(keep)} used by add rows; keep {len(keep & files)}, delete {len(drop)}")
    if dry:
        return 1 if missing else 0
    for p in drop:
        os.remove(os.path.join(common.ROOT, p))
    lines = open(common.PLACED).read().splitlines()
    kept = [x for x in lines if x.startswith("path\t") or x.split("\t")[0] in keep]
    with open(common.PLACED, "w") as out:
        out.write("\n".join(kept) + "\n")
    print("datasheets.tsv:", len(kept) - 1, "lines")
    return 1 if missing else 0


if __name__ == "__main__":
    sys.exit(main())
