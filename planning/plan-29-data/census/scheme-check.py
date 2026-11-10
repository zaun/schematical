"""plan-29-B section 4.5: decide `pending` rows from the datasheets' ordering-code keys.

usage: python3 -I scheme-check.py [--maker M ...] [--category C ...] [--tsv candidates.tsv]
                                  [--specs specs.tsv] [--dry-run]

specs.tsv (plan-29-A Phase 5) has one row per (maker, category, datasheet,
supplier_package) of the add and pending rows; `datasheet` is the PDF's path
under datasheets/diode/ (datasheets/diode/datasheets.tsv maps the row's
datasheet_url to it), `mpn_regex` the ordering-code key B-G write from what
the datasheet prints.  A `pending` row of the named makers / categories (all
when none is named) becomes `add` when its part number re.fullmatch-es its
spec row's mpn_regex, else `skip` (reason "not in the datasheet's ordering
table (<datasheet file> <note>)").  A pending row whose spec row has an empty
mpn_regex is listed and left pending (exit 1).
"""
import argparse
import csv
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
PLACED = os.path.join(ROOT, "datasheets", "diode", "datasheets.tsv")


def path_of_url():
    return {r["url"]: r["path"] for r in csv.DictReader(open(PLACED), delimiter="\t")}


def spec_key(r, paths):
    return (r["maker"], r["category"], paths.get(r["datasheet_url"], r["datasheet_url"]), r["supplier_package"])


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--maker", action="append", default=[])
    ap.add_argument("--category", action="append", default=[])
    ap.add_argument("--tsv", default=os.path.join(DATA, "candidates.tsv"))
    ap.add_argument("--specs", default=os.path.join(DATA, "specs.tsv"))
    ap.add_argument("--dry-run", action="store_true")
    a = ap.parse_args()
    rows = list(csv.DictReader(open(a.tsv), delimiter="\t"))
    cols = list(rows[0].keys())
    paths = path_of_url()
    specs = {}
    for s in csv.DictReader(open(a.specs), delimiter="\t"):
        specs[(s["maker"], s["category"], s["datasheet"], s["supplier_package"])] = s
    added = skipped = left = 0
    for r in rows:
        if r["decision"] != "pending":
            continue
        if a.maker and not any(r["maker"].startswith(m) for m in a.maker):
            continue
        if a.category and r["category"] not in a.category:
            continue
        s = specs.get(spec_key(r, paths))
        if s is None or not s["mpn_regex"]:
            print(f"no key: {r['maker']} / {r['category']} / {r['supplier_package']}: {r['mpn']}")
            left += 1
            continue
        prefix = (r["reason"] + "; ") if r["reason"] else ""
        source = os.path.basename(s["datasheet"]) + (" " + s["note"] if s["note"] else "")
        if re.fullmatch(s["mpn_regex"], r["mpn"]):
            r["decision"], r["reason"] = "add", prefix + "ordering key " + os.path.basename(s["datasheet"])
            added += 1
        else:
            r["decision"] = "skip"
            r["reason"] = prefix + "not in the datasheet's ordering table (" + source + ")"
            skipped += 1
    if not a.dry_run:
        with open(a.tsv, "w") as out:
            out.write("\t".join(cols) + "\n")
            for r in rows:
                out.write("\t".join(r[c] for c in cols) + "\n")
    print(f"{added} add, {skipped} skip, {left} still pending")
    return 1 if left else 0


if __name__ == "__main__":
    sys.exit(main())
