#!/usr/bin/env python3
"""plan-40-B Phase 5: the PDFs that stay in datasheets/transistor/.

usage: python3 -I prune.py [--dry-run]

Keeps each PDF an `add` row of candidates.tsv uses, and each PDF of
datasheets/transistor/ that the `note` of a footprint the final map uses
names (by path or by file name: the drawing a land pattern was taken from),
except a PDF listed in datasheets/obsolete-not-included.md. Deletes every
other file of datasheets/transistor/<maker>/ and rewrites datasheets.tsv to
the kept files' lines. Prints the counts.
"""
import csv
import json
import os
import re
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

DATA = common.DATA


def used_footprints():
    sys.path.insert(0, common.HERE)
    import importlib.util
    spec = importlib.util.spec_from_file_location("fpcheck", os.path.join(common.HERE, "fp-check.py"))
    fpc = importlib.util.module_from_spec(spec)
    sys.argv, keep = sys.argv[:1], sys.argv
    spec.loader.exec_module(fpc)
    sys.argv = keep
    fmap = fpc.load_map()
    ids = set()
    for line in open(os.path.join(DATA, "footprints-needed.txt")):
        if line.startswith("#") or not line.strip():
            continue
        f = line.rstrip("\n").split("\t")
        for sup in [s.strip() for s in f[5].split("|")]:
            m = fpc.lookup(fmap, f[0], f[1], f[3], sup)
            if m:
                ids.add(m["footprint"])
    return ids


def main():
    dry = "--dry-run" in sys.argv
    files = {}
    for slug in common.SLUG.values():
        d = os.path.join(common.TREE, slug)
        for name in os.listdir(d) if os.path.isdir(d) else []:
            files[os.path.join("datasheets", "transistor", slug, name)] = name
    keep = {r["pdf"] for r in csv.DictReader(open(os.path.join(DATA, "candidates.tsv")), delimiter="\t")
            if r["reason"] == "add" and r["pdf"]}
    adds = len(keep)
    ids = used_footprints()
    notes = ""
    for L in "CDE":
        p = os.path.join(DATA, f"footprints-{L}.json")
        if os.path.exists(p):
            notes += " ".join(o.get("note", "") for o in json.load(open(p))["footprints"] if o["id"] in ids)
    cited = {path for path, name in files.items() if path in notes or re.search(r"\b" + re.escape(name), notes)}
    obsolete_md = open(os.path.join(common.ROOT, "datasheets", "obsolete-not-included.md")).read()
    obsolete = {path for path, name in files.items() if f"`{name}`" in obsolete_md and "plan-40-B" in
                obsolete_md[obsolete_md.find(f"`{name}`"):obsolete_md.find(f"`{name}`") + 400]}
    only_cited = (cited - obsolete) - keep
    keep = (keep | cited) - obsolete
    drop = sorted(set(files) - keep)
    print(f"{len(files)} files; {adds} used by add rows; {len(cited - obsolete)} cited by used footprints "
          f"({len(only_cited)} of them only there); {len(obsolete)} obsolete; keep {len(keep)}, delete {len(drop)}")
    missing = sorted(p for p in keep if p not in files)
    for p in missing:
        print("missing (used but not in the tree):", p)
    if dry:
        return 1 if missing else 0
    for p in drop:
        os.remove(os.path.join(common.ROOT, p))
    lines = open(common.PLACED).read().splitlines()
    kept = [l for l in lines if l.startswith("path\t") or l.split("\t")[0] in keep]
    with open(common.PLACED, "w") as out:
        out.write("\n".join(kept) + "\n")
    print("datasheets.tsv:", len(kept) - 1, "lines")
    return 1 if missing else 0


if __name__ == "__main__":
    sys.exit(main())
