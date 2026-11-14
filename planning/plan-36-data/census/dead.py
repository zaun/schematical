"""plan-36-A Phase 4 (§ 3 rule 4): apply the by-hand reading of the `dead` = 1 rows.

usage: python3 -I dead.py <cand.tsv> <dead-verdicts.tsv>

dead-verdicts.tsv: `pdf mpn_regex verdict reason evidence`, one line per
reading. Every in-scope `add` row of that PDF whose part number matches the
regex in full (case aside) gets `verdict` (`out`) and `reason` (`datasheet:
NRND`, `datasheet: end of life`, ...). A `keep` line records a dead = 1 row
that was read and stays `add` (the mark belongs to another part of the
table). Prints the rows changed, then every dead = 1 add row that no line
covers (it must be 0).
"""
import csv
import re
import sys


def main(tsv, path):
    rows = list(csv.DictReader(open(tsv), delimiter="\t"))
    rules = [(r["pdf"], re.compile(r["mpn_regex"], re.I), r["verdict"], r["reason"])
             for r in csv.DictReader(open(path), delimiter="\t")]
    changed = 0
    uncovered = []
    for r in rows:
        if r["scope"] != "in" or r["verdict"] not in ("add", "skip"):
            continue
        rule = next((x for x in rules if x[0] == r["pdf"] and x[1].fullmatch(r["mpn"])), None)
        if rule is None:
            if r["dead"] == "1" and r["verdict"] == "add":
                uncovered.append(r["mpn"])
            continue
        if rule[2] == "out":
            r["verdict"], r["reason"] = "out", rule[3]
            changed += 1
            print("out", r["mpn"], rule[3])
    cols = list(rows[0].keys())
    with open(tsv, "w") as out:
        out.write("\t".join(cols) + "\n")
        for r in rows:
            out.write("\t".join(r[c] for c in cols) + "\n")
    print(f"{changed} rows out for datasheet status; {len(uncovered)} dead = 1 add rows not read: {uncovered}")
    return 1 if uncovered else 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:3]))
