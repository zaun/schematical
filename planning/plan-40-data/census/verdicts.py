#!/usr/bin/env python3
"""plan-40-B section 3.2: a verdict for every eligible row.

usage: python3 -I verdicts.py <candidates.tsv> <specs.tsv> <verdicts.tsv>
       python3 -I verdicts.py --write-skipped [<candidates.tsv>]
       python3 -I verdicts.py --verify-skipped [<candidates.tsv>]

Per eligible row (common.eligible; after the final pick, every row whose
reason is add or skip): no PDF -> skip, `no datasheet on the maker's site
(<host of its link, or none>)`; `dead` 1 -> out, `datasheet status`; its
spec row not read (`scheme` n/r) -> out, `below the pick (not read)`; not
printed verbatim and not matched by its spec row's scheme -> skip, `not in
the datasheet's ordering table`; its spec row's pinout `-` -> skip, `no
pinout in the datasheet`; else add. Writes mpn, verdict, reason, pdf.

--write-skipped: writes (or replaces) the `## Transistors (plan-40)`
section of datasheets/skipped-parts.md: one table row per (maker, family,
reason) group of the final `skip` rows that the pick would have taken
(stock at or above the lowest stock of the (family, maker)'s `add` rows),
listing the part numbers.

--verify-skipped: every such `skip` row is named in that section.
"""
import collections
import csv
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402
import specs  # noqa: E402

SKIPPED = os.path.join(common.ROOT, "datasheets", "skipped-parts.md")
SECTION = "## Transistors (plan-40)"
# A spec row the pick cannot reach is not read (plan-40-B Corrections 1);
# select.py --report proves none of its rows is at or above its floor.
NOT_READ = "below the pick (not read)"
LEAF = {
    "bjt": "BJT", "bjt-array": "BJT (array)", "bjt-prebiased": "BJT (pre-biased)",
    "bjt-prebiased-array": "BJT (pre-biased array)", "mosfet": "MOSFET", "mosfet-array": "MOSFET (array)",
}


def verdict(r, s):
    if not r["pdf"]:
        return "skip", f"no datasheet on the maker's site ({common.host(r['datasheet_url']) or 'none'})"
    if r["dead"] == "1":
        return "out", "datasheet status"
    if s is not None and s["scheme"] == "n/r":
        return "out", NOT_READ
    if s is None or not s["scheme"]:
        sys.exit(f"verdicts.py: no spec row with a scheme for {specs.key(r)} ({r['mpn']})")
    if not specs.in_table(r, s):
        return "skip", "not in the datasheet's ordering table"
    if s["pinout"] == "-" or not s["pinout"]:
        return "skip", "no pinout in the datasheet"
    return "add", ""


def main(cand, spec_path, out):
    table = specs.load(spec_path)
    counts = collections.Counter()
    with open(out, "w") as f:
        f.write("mpn\tverdict\treason\tpdf\n")
        for r in csv.DictReader(open(cand), delimiter="\t"):
            if not common.eligible(r):
                continue
            v, why = verdict(r, table.get(specs.key(r)))
            counts[(v, why)] += 1
            f.write("\t".join([r["mpn"], v, why, r["pdf"]]) + "\n")
    for k in sorted(counts):
        print(counts[k], *k)


def final_skips(rows):
    floor = {}
    for r in rows:
        if r["reason"] == "add":
            k = (r["family"], r["maker"])
            floor[k] = min(floor.get(k, 1 << 62), int(r["stock"]))
    return [r for r in rows if r["reason"].startswith("skip")
            and int(r["stock"]) >= floor.get((r["family"], r["maker"]), 1 << 62)]


def write_skipped(cand):
    rows = list(csv.DictReader(open(cand), delimiter="\t"))
    groups = collections.defaultdict(list)
    for r in final_skips(rows):
        why = r["reason"].split(":", 1)[1].strip() if ":" in r["reason"] else r["reason"]
        groups[(r["maker"], r["family"], why)].append(r["mpn"])
    lines = [SECTION, "",
             "In-stock Active / Last Time Buy BJTs and MOSFETs of the makers of plan-40",
             "(BJTs: onsemi, Nexperia, Diodes Incorporated; MOSFETs: onsemi, Infineon,",
             "Vishay Siliconix) that the pick would have taken (stock at or above the stock",
             "of the last part picked for their family and maker) but that were skipped for",
             "their datasheet. Written by `planning/plan-40-data/census/verdicts.py --write-skipped`.",
             "", "| Part(s) | Device | Would have gone into | Why skipped |", "|---|---|---|---|"]
    for (maker, fam, why), mpns in sorted(groups.items()):
        family = "MOSFET" if fam.startswith("mosfet") else "BJT"
        lines.append(f"| {', '.join(sorted(mpns))} | {maker} {LEAF[fam]} | `transistor` / "
                     f"Discrete → Transistors → {family} | {why} |")
    text = open(SKIPPED).read()
    at = text.find(SECTION)
    if at >= 0:
        end = text.find("\n## ", at + len(SECTION))
        text = text[:at] + text[end + 1:] if end >= 0 else text[:at]
    text = text.rstrip("\n") + "\n\n" + "\n".join(lines) + "\n"
    open(SKIPPED, "w").write(text)
    print(sum(len(v) for v in groups.values()), "skip rows in", len(groups), "table rows")


def verify_skipped(cand):
    rows = list(csv.DictReader(open(cand), delimiter="\t"))
    text = open(SKIPPED).read()
    at = text.find(SECTION)
    section = text[at:] if at >= 0 else ""
    end = section.find("\n## ", len(SECTION))
    section = section[:end] if end >= 0 else section
    missing = [r["mpn"] for r in final_skips(rows) if r["mpn"] not in section]
    for m in missing:
        print("missing", m)
    print(f"{len(missing)} skip rows missing from skipped-parts.md")
    return 1 if missing else 0


if __name__ == "__main__":
    if sys.argv[1] == "--write-skipped":
        write_skipped(sys.argv[2] if len(sys.argv) > 2 else specs.CANDIDATES)
        sys.exit(0)
    if sys.argv[1] == "--verify-skipped":
        sys.exit(verify_skipped(sys.argv[2] if len(sys.argv) > 2 else specs.CANDIDATES))
    main(*sys.argv[1:4])
