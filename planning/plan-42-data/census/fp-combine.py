#!/usr/bin/env python3
"""plan-42-D/E/F: merge the packets' ready footprint objects and map rows.

usage: python3 -I fp-combine.py [--write]

Reads every footprints-<L>-<packet>.json and footprint-map-<L>-<packet>.tsv
(L = D, E, F), reports an id defined twice with different content, ids no map
row uses, and map rows naming an id that no file defines (nor Parts/
footprints.json). With --write: writes footprints-<L>.json (the letter's
objects in packet order; an id defined by several packets once, the first)
and footprint-map.tsv (every map row, with the `pdf` column), and removes the
packet files they replace (the .evidence.tsv files stay).
"""
import csv
import glob
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
LIB = os.path.join(ROOT, "schematical", "schematical", "Parts", "footprints.json")


def main(write):
    lib = set(re.findall(r'"id":"([^"]+)"', open(LIB).read()))
    defined = {}
    per_letter = {}
    bad = 0
    for L in "DEF":
        objs = []
        for path in sorted(glob.glob(os.path.join(DATA, f"footprints-{L}-*.json"))):
            if path.endswith(".evidence.json"):
                continue
            for o in json.load(open(path))["footprints"]:
                if o["id"] in defined:
                    if defined[o["id"]] != o:
                        print("id defined twice with different content:", o["id"], path)
                        bad += 1
                    continue
                defined[o["id"]] = o
                objs.append(o)
        per_letter[L] = objs
    rows = []
    cols = ["letter", "maker", "package", "supplier", "pins", "footprint", "label", "note", "pdf"]
    for path in sorted(glob.glob(os.path.join(DATA, "footprint-map-[DEF]-*.tsv"))):
        for r in csv.DictReader(open(path), delimiter="\t"):
            r.setdefault("pdf", "")
            rows.append(r)
    used = {r["footprint"] for r in rows}
    for r in rows:
        if r["footprint"] not in defined and r["footprint"] not in lib:
            print("map row names an unknown id:", r["footprint"], r["maker"], r["package"])
            bad += 1
    unused = sorted(set(defined) - used)
    for i in unused:
        print("defined but no map row uses it:", i)
    print({L: len(o) for L, o in per_letter.items()}, "objects;", len(rows), "map rows;", len(unused), "unused;",
          "reused library ids:", len({r["footprint"] for r in rows} & lib))
    if write and not bad:
        for L, objs in per_letter.items():
            with open(os.path.join(DATA, f"footprints-{L}.json"), "w") as f:
                f.write('{\n  "format": "schematical-footprints",\n  "version": 1,\n  "footprints": [\n')
                f.write(",\n".join("    " + json.dumps(o, ensure_ascii=False, separators=(",", ":")) for o in objs))
                f.write("\n  ]\n}\n")
        with open(os.path.join(DATA, "footprint-map.tsv"), "w") as f:
            w = csv.writer(f, delimiter="\t", lineterminator="\n")
            w.writerow(cols)
            for r in sorted(rows, key=lambda r: (r["letter"], r["maker"], r["package"], r["supplier"], r["pins"], r.get("pdf", ""))):
                w.writerow([r.get(c, "") for c in cols])
        for path in glob.glob(os.path.join(DATA, "footprints-[DEF]-*.json")) + glob.glob(os.path.join(DATA, "footprint-map-[DEF]-*.tsv")):
            os.remove(path)
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main("--write" in sys.argv))
