"""plan-27-B section 4.3: decide `pending` rows from the datasheets' part-number keys.

usage: python3 -I scheme-check.py --maker M [--maker M2 ...] [--tsv candidates.tsv] [--schemes schemes.tsv] [--dry-run]

schemes.tsv: header `maker series regex source`; `maker` is the full
DigiKey maker name (as in candidates.tsv), `series` the DigiKey series; several
rows of one (maker, series) are alternatives.  A `pending` row of a named maker
whose (maker, series) has a scheme becomes `add` when its part number
re.fullmatch-es one of them, else `skip` (reason "not in the datasheet's
ordering table (<source>)").  Prints every accepted and rejected part number.
A pending row with no scheme for its series is listed and left pending (exit 1).
"""
import argparse
import csv
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--maker", action="append", required=True)
    ap.add_argument("--tsv", default=os.path.join(DATA, "candidates.tsv"))
    ap.add_argument("--schemes", default=os.path.join(DATA, "schemes.tsv"))
    ap.add_argument("--dry-run", action="store_true")
    a = ap.parse_args()
    with open(a.tsv) as f:
        rows = list(csv.DictReader(f, delimiter="\t"))
    cols = list(rows[0].keys())
    schemes = {}
    with open(a.schemes) as f:
        for s in csv.DictReader(f, delimiter="\t"):
            schemes.setdefault((s["maker"], s["series"]), []).append(s)
    added = skipped = left = 0
    for r in rows:
        maker = next((m for m in a.maker if r["maker"].startswith(m)), None)
        if maker is None or r["decision"] != "pending":
            continue
        options = schemes.get((r["maker"], r["series"]))
        if not options:
            print(f"no scheme: {maker} / {r['series']}: {r['mpn']}")
            left += 1
            continue
        hit = next((s for s in options if re.fullmatch(s["regex"], r["mpn"])), None)
        prefix = (r["reason"] + "; ") if r["reason"] else ""
        if hit:
            r["decision"], r["reason"] = "add", prefix + "key " + hit["source"]
            print(f"add   {r['mpn']}  ({hit['source']})")
            added += 1
        else:
            r["decision"] = "skip"
            r["reason"] = prefix + "not in the datasheet's ordering table (" + "; ".join(s["source"] for s in options) + ")"
            print(f"skip  {r['mpn']}  ({r['series']})")
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
