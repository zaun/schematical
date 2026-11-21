#!/usr/bin/env python3
"""plan-42-D/E/F: split the land-pattern lines of one letter into drawing packets.

usage: python3 -I fp-packets.py <letter> <out dir> [<lines per packet>]

Writes <out dir>/<letter>-<nn>.txt: per line (maker, DigiKey Package / Case,
Supplier Device Package) the pin counts its `add` rows' pinouts have, the
placed datasheets (the PDFs to take the package drawing from), the part
numbers, the existing library footprints with the same outline (footprints-
needed.txt's 10th column) and one example pinout per pin count; and
<out dir>/<letter>-<nn>.only (maker TAB package TAB supplier per line, for
fp-check.py --only). Lines are sorted by package so a packet holds the
variants of one outline.
"""
import collections
import json
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402
import specs  # noqa: E402


def main(letter, out, size):
    rows = common.load(specs.CANDIDATES)
    table = specs.load()
    info = collections.defaultdict(lambda: {"pdfs": collections.Counter(), "mpns": [], "pins": {}})
    for r in rows:
        if r["reason"] != "add":
            continue
        s = table[specs.key(r)]
        mixed = bool(s["pinout_adj"])
        pin = s["pinout_adj"] if (specs.variant_of(r) == "adj" and mixed) else s["pinout"]
        names = json.loads(pin)
        d = info[(r["maker"], r["package"], r["supplier_package"])]
        d["pdfs"][r["pdf"]] += 1
        d["mpns"].append(r["mpn"])
        d["pins"].setdefault(len(names), " ".join(f"{n}:{nm}" for n, nm, _ in names))
    lines = []
    for line in open(os.path.join(common.DATA, "footprints-needed.txt")):
        if line.startswith("#") or not line.strip():
            continue
        f = line.rstrip("\n").split("\t")
        if f[0] == letter:
            lines.append(f)
    lines.sort(key=lambda f: (f[3], f[4], f[1]))
    os.makedirs(out, exist_ok=True)
    for n in range(0, len(lines), size):
        name = f"{letter}-{n // size + 1:02d}"
        with open(os.path.join(out, name + ".txt"), "w") as t, open(os.path.join(out, name + ".only"), "w") as o:
            for f in lines[n:n + size]:
                d = info[(f[1], f[3], f[4])]
                t.write(f"### LINE maker={f[1]} | package={f[3]} | supplier={f[4]} | mounting={f[2]} | parts={f[6]}\n")
                t.write(f"  pin counts: {sorted(d['pins'])}\n")
                for pc in sorted(d["pins"]):
                    t.write(f"  example pinout ({pc} pins): {d['pins'][pc]}\n")
                t.write("  datasheets: " + "; ".join(f"{p} ({c} parts)" for p, c in d["pdfs"].most_common(4)) + "\n")
                t.write("  parts: " + ", ".join(sorted(d["mpns"])[:5]) + "\n")
                t.write(f"  existing footprints with the same outline: {f[9] or '(none)'}\n\n")
                o.write(f"{f[1]}\t{f[3]}\t{f[4]}\n")
        print(name, len(lines[n:n + size]), "lines")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2], int(sys.argv[3]) if len(sys.argv) > 3 else 14)
