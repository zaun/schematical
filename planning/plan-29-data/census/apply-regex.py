"""plan-29-B–G: copy the ordering keys of work/regex-<X>.tsv into specs.tsv.

usage: python3 -I apply-regex.py work/regex-B.tsv ...

Each line (maker category datasheet supplier_package mpn_regex note) sets
the mpn_regex of the spec row with the same four key columns, and appends
its note to the row's note.  Every line must name an existing spec row with
an empty mpn_regex, and every regex must compile.
"""
import csv
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SPECS = os.path.join(os.path.dirname(HERE), "specs.tsv")
KEY = ("maker", "category", "datasheet", "supplier_package")


def main(paths):
    rows = list(csv.DictReader(open(SPECS), delimiter="\t"))
    cols = list(rows[0].keys())
    index = {tuple(r[k] for k in KEY): r for r in rows}
    bad = 0
    n = 0
    for p in paths:
        for line in csv.DictReader(open(p), delimiter="\t"):
            k = tuple(line[c] for c in KEY)
            r = index.get(k)
            if r is None or r["mpn_regex"]:
                print(f"{p}: no empty spec row for {k}")
                bad += 1
                continue
            re.compile(line["mpn_regex"])
            r["mpn_regex"] = line["mpn_regex"]
            r["note"] = "; ".join(x for x in (r["note"], "key " + line["note"]) if x)
            n += 1
    with open(SPECS, "w") as out:
        out.write("\t".join(cols) + "\n")
        for r in rows:
            out.write("\t".join(r[c] for c in cols) + "\n")
    left = sum(1 for r in rows if not r["mpn_regex"])
    print(f"{n} keys applied, {bad} bad lines, {left} spec rows without a key")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
