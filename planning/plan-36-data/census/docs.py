"""plan-36-A Phase 4: the datasheets/skipped-parts.md section and the
obsolete-not-included.md lines, from candidates.tsv.

usage: python3 -I docs.py <cand.tsv> <pdf index.tsv> <dead-verdicts.tsv>

Prints two Markdown blocks to stdout: the skipped-parts.md section (one table
line per (reason, PDF) of the `skip` rows) and the obsolete-not-included.md
lines (each PDF whose in-scope rows are all `out` for datasheet status, with
the evidence from dead-verdicts.tsv). The plan pastes them into the files.
"""
import csv
import sys
from collections import OrderedDict


def main(tsv, index, dead):
    url = {r["md5"]: r["url"] for r in csv.DictReader(open(index), delimiter="\t")}
    name = lambda h: url.get(h, "").rstrip("/").split("/")[-1] or "(none)"  # noqa: E731
    rows = [r for r in csv.DictReader(open(tsv), delimiter="\t") if r["scope"] == "in"]
    groups = OrderedDict()
    for r in sorted(rows, key=lambda r: (r["reason"], name(r["pdf"]), r["mpn"])):
        if r["verdict"] == "skip":
            groups.setdefault((r["reason"], r["pdf"]), []).append(r)
    print("## Microcontrollers (plan-36)\n")
    print("In-stock DigiKey parts in plan-36's scope (8-bit AVR, ESP32 modules, RP2040; listed 2026-11-13)")
    print("left out by plan-36-A § 3: the part number is not in its datasheet's ordering table (rule 5),")
    print("checked against the row's own PDF, every PDF of its device and every same-maker PDF naming the device.\n")
    print("| Part(s) | Device | Would have gone into | Why skipped |")
    print("|---|---|---|---|")
    for (reason, h), rs in groups.items():
        devs = ", ".join(sorted({r["device"] for r in rs}))
        fam = rs[0]["family"]
        print(f"| {', '.join(r['mpn'] for r in rs)} | {devs}, `{name(h)}` | `mcu` / Microcontroller → {fam} | {reason[0].upper() + reason[1:]} (`{name(h)}`). |")
    ev = {r["pdf"]: r["evidence"] for r in csv.DictReader(open(dead), delimiter="\t") if r["verdict"] == "out"}
    print("\n---- obsolete-not-included.md lines\n")
    by_pdf = OrderedDict()
    for r in rows:
        if r["pdf"] in ev:
            by_pdf.setdefault(r["pdf"], []).append(r)
    for h, rs in by_pdf.items():
        if all(r["verdict"] == "out" for r in rs):
            devs = ", ".join(sorted({r["device"] for r in rs}))
            print(f"| `{name(h)}` ({devs}) | mcu | Microcontroller → {rs[0]['family']} | {ev[h]}; its in-stock parts "
                  f"({', '.join(r['mpn'] for r in rs)}) are out (plan-36-A); {url.get(h, '')} |")


if __name__ == "__main__":
    main(*sys.argv[1:4])
