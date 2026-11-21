#!/usr/bin/env python3
"""plan-42-B Phase 4: the datasheet status read by hand into candidates.tsv.

usage: python3 -I dead-merge.py [<candidates.tsv>]

Each planning/plan-42-data/dead-*.tsv (columns mpn, dead 0/1, page, words:
the reader's verdict on the PDF's own status wording for that part) sets
`dead` of its rows in candidates.tsv; rows the files do not name keep
textscan.py's value. Prints how many rows each file set, and each PDF whose
download-set rows with that PDF are all dead (B lists those in
datasheets/obsolete-not-included.md).
"""
import collections
import csv
import glob
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402
import specs  # noqa: E402


def main(cand=specs.CANDIDATES):
    rows = common.load(cand)
    cols = list(rows[0].keys())
    common.init(rows)
    read = {}
    for path in sorted(glob.glob(os.path.join(common.DATA, "dead-*.tsv"))):
        n = 0
        for r in csv.DictReader(open(path), delimiter="\t"):
            read[r["mpn"]] = r["dead"].strip()
            n += 1
        print(os.path.basename(path), n, "rows")
    changed = collections.Counter()
    for r in rows:
        if r["mpn"] in read and common.eligible(r) and r["dead"] != read[r["mpn"]]:
            changed[f"{r['dead'] or '-'} -> {read[r['mpn']]}"] += 1
            r["dead"] = read[r["mpn"]]
    with open(cand, "w") as out:
        out.write("\t".join(cols) + "\n")
        for r in rows:
            out.write("\t".join(r[c] for c in cols) + "\n")
    print("changed:", dict(changed))
    by_pdf = collections.defaultdict(list)
    for r in rows:
        if common.eligible(r) and r["pdf"]:
            by_pdf[r["pdf"]].append(r["dead"] == "1")
    for pdf, flags in sorted(by_pdf.items()):
        if all(flags):
            print("all dead:", pdf, len(flags), "rows")


if __name__ == "__main__":
    main(*sys.argv[1:2])
