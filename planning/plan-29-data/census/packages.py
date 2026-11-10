"""plan-29-B–G work split: one line per (maker, supplier package, mounting) of the add/pending rows.

usage: python3 -I packages.py <candidates.tsv> <specs.tsv> > packages.tsv

Columns: owner (the first letter of B C D E F G whose scope uses the
package for that maker: it reads the drawing and draws the footprint;
later letters reuse it), letters (every letter using it), maker, supplier
package, mounting, parts, spec rows, DigiKey Package / Case values,
configurations, the three PDFs with the most parts.
"""
import collections
import csv
import sys

SHORT = {"Vishay General Semiconductor - Diodes Division": "Vishay", "onsemi": "onsemi",
         "Taiwan Semiconductor Corporation": "TSC"}


def letter(maker, cat):
    if cat == "single":
        return {"Vishay": "B", "onsemi": "C", "TSC": "D"}[maker]
    return {"zener": "E", "zener-array": "E", "array": "F", "bridge": "G"}[cat]


def main(cand, specs):
    paths = {}
    for s in csv.DictReader(open(specs), delimiter="\t"):
        pass
    placed = {r["url"]: r["path"] for r in csv.DictReader(open("../../datasheets/diode/datasheets.tsv"), delimiter="\t")}
    g = collections.defaultdict(lambda: {"letters": set(), "parts": 0, "groups": set(), "cases": set(),
                                         "conf": set(), "pdfs": collections.Counter()})
    for r in csv.DictReader(open(cand), delimiter="\t"):
        if r["decision"] not in ("add", "pending"):
            continue
        m = SHORT[r["maker"]]
        k = (m, r["supplier_package"], r["mounting"])
        e = g[k]
        e["letters"].add(letter(m, r["category"]))
        e["parts"] += 1
        p = placed[r["datasheet_url"]]
        e["groups"].add((r["category"], p))
        e["cases"].add(r["package"])
        if r["configuration"]:
            e["conf"].add(r["configuration"])
        e["pdfs"][p] += 1
    cols = ["owner", "letters", "maker", "supplier_package", "mounting", "parts", "spec_rows",
            "cases", "configurations", "top_pdfs"]
    print("\t".join(cols))
    for k in sorted(g, key=lambda k: (min(g[k]["letters"]), k)):
        e = g[k]
        print("\t".join([min(e["letters"]), "".join(sorted(e["letters"])), k[0], k[1], k[2],
                         str(e["parts"]), str(len(e["groups"])), "; ".join(sorted(e["cases"])),
                         "; ".join(sorted(e["conf"])),
                         " ".join(p for p, _ in e["pdfs"].most_common(3))]))


if __name__ == "__main__":
    main(*sys.argv[1:3])
