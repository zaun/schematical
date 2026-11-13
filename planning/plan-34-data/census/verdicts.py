#!/usr/bin/env python3
"""plan-34-B § 3.2: a verdict for every eligible row.

usage: python3 -I verdicts.py <candidates.tsv> <specs.tsv> <verdicts.tsv>
       python3 -I verdicts.py --verify-skipped [<candidates.tsv>]

Per eligible row (common.eligible): no PDF -> skip, `no datasheet on the
maker's site (<host of its link, or none>)`; read as out in
census/dead-verdicts.tsv -> out, `datasheet status: <word>`; printed
verbatim (`hit` 1) -> add; else its (maker, series, PDF) spec row's
mpn_regex and field check (scheme.py) -> add or skip. Writes mpn, verdict,
reason, datasheet (the PDF path).

--verify-skipped: every `skip` row whose stock is at or above its
category's floor (the stock of the last picked row) is named in
datasheets/skipped-parts.md's "Resistors (plan-34)" section.
"""
import csv
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402
import scheme  # noqa: E402

DEAD = os.path.join(common.HERE, "dead-verdicts.tsv")
CANDIDATES = os.path.join(common.DATA, "candidates.tsv")
SKIPPED = os.path.join(common.ROOT, "datasheets", "skipped-parts.md")


def dead_verdicts():
    if not os.path.exists(DEAD):
        return {}
    return {r["mpn"]: r for r in csv.DictReader(open(DEAD), delimiter="\t")}


def main(cand, specs, out):
    regex = {}
    for r in csv.DictReader(open(specs), delimiter="\t"):
        regex[(r["maker"], r["series"], r["datasheet"])] = (r["mpn_regex"], r["fields_checked"])
    dead = dead_verdicts()
    counts = {}
    with open(out, "w") as f:
        f.write("mpn\tverdict\treason\tdatasheet\n")
        for r in csv.DictReader(open(cand), delimiter="\t"):
            if not common.eligible(r):
                continue
            if not r["pdf"]:
                v, why = "skip", f"no datasheet on the maker's site ({common.host(r['datasheet_url']) or 'none'})"
            elif r["mpn"] in dead and dead[r["mpn"]]["verdict"] == "out":
                v, why = "out", f"datasheet status: {dead[r['mpn']]['word']}"
            elif r.get("hit") == "1":
                v, why = "add", ""
            else:
                key = (r["maker"], r["series"], r["pdf"])
                if not regex.get(key, ("", ""))[0]:
                    sys.exit(f"verdicts.py: no mpn_regex for {key} ({r['mpn']})")
                v, why = scheme.verdict(r, *regex[key])
            counts[(v, why)] = counts.get((v, why), 0) + 1
            f.write("\t".join([r["mpn"], v, why, r["pdf"]]) + "\n")
    for k in sorted(counts):
        print(counts[k], *k)


def verify_skipped(cand):
    rows = list(csv.DictReader(open(cand), delimiter="\t"))
    floor = {}
    for r in rows:
        if r["decision"] == "add":
            floor[r["category"]] = min(floor.get(r["category"], 1 << 62), int(r["stock"]))
    text = open(SKIPPED).read()
    at = text.find("Resistors (plan-34)")
    section = text[at:] if at >= 0 else ""
    missing = [r["mpn"] for r in rows if r["decision"] == "skip"
               and int(r["stock"]) >= floor.get(r["category"], 1 << 62) and r["mpn"] not in section]
    for m in missing:
        print("missing", m)
    print(f"{len(missing)} skip rows missing from skipped-parts.md")
    return 1 if missing else 0


if __name__ == "__main__":
    if sys.argv[1] == "--verify-skipped":
        sys.exit(verify_skipped(sys.argv[2] if len(sys.argv) > 2 else CANDIDATES))
    main(*sys.argv[1:4])
