"""plan-36-B Phase 2: transcribed.tsv = the auto, hand and module-table transcriptions.

usage: python3 -I transcribed-merge.py <out transcribed.tsv> <in.tsv>...

All inputs have the transcribed.tsv columns (`pdf page figure devices pinout
designator name role`). A (pdf, devices, pinout) unit the hand file also has
is taken from the hand file (a hand reading overrides the extractor).
"""
import csv
import sys


def main(out_path, *paths):
    units = {}
    order = []
    for path in paths:
        hand = "hand" in path
        got = {}
        for r in csv.DictReader(open(path), delimiter="\t"):
            got.setdefault((r["pdf"], r["devices"], r["pinout"]), []).append(r)
        for k, rs in got.items():
            if k in units and not hand:
                continue
            if k not in units:
                order.append(k)
            units[k] = rs
    cols = ["pdf", "page", "figure", "devices", "pinout", "designator", "name", "role"]
    with open(out_path, "w") as out:
        out.write("\t".join(cols) + "\n")
        for k in order:
            for r in units[k]:
                out.write("\t".join(r[c] for c in cols) + "\n")
    print(f"transcribed.tsv: {len(order)} units, {sum(len(v) for v in units.values())} pins")


if __name__ == "__main__":
    main(*sys.argv[1:])
