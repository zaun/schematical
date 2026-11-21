#!/usr/bin/env python3
"""plan-42-D/E/F: does every land-pattern line have a footprint?

usage: python3 -I fp-check.py <letter> [--list] [--only <file of maker TAB package TAB supplier>]

Joins the letter's lines of footprints-needed.txt (letter, maker, mounting,
DigiKey Package / Case, supplier package, families, parts, example part,
example datasheet, existing footprints with the same outline) with the
footprint maps (planning/plan-42-data/footprint-map.tsv and every
footprint-map-<letter>*.tsv: columns letter, maker, package, supplier, pins,
footprint, label, note and optionally `pdf` (the datasheet file name an `add`
row uses, or `*`: a row for one datasheet's drawing wins over a `*` row);
one row per land pattern; `supplier` is the line's Supplier Device Package or
`*`; `pins` is the pinout's pin count or `*`). A line needs a map row for
every (datasheet, pin count) its `add` rows' pinouts have (the count of the
variant's `pinout`), the mapped id must be in
Parts/footprints.json or a footprints-<letter>*.json of this directory, and
the footprint's `pins` must equal that pin count. Prints `<n> lines, <m>
without a footprint` and each missing line, unknown id or pin-count mismatch;
with --list also the footprint chosen per line.
"""
import collections
import csv
import glob
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.append(HERE)
import common  # noqa: E402
import specs  # noqa: E402

DATA = common.DATA
ROOT = common.ROOT


def footprint_pins():
    """{id: pins} of Parts/footprints.json and every footprints-*.json here."""
    out = {}
    paths = [os.path.join(ROOT, "schematical", "schematical", "Parts", "footprints.json")]
    paths += sorted(glob.glob(os.path.join(DATA, "footprints-[DEF]*.json")))
    for p in paths:
        for o in json.load(open(p))["footprints"]:
            out.setdefault(o["id"], o["pins"])
    return out


def load_map():
    fmap = {}
    paths = [os.path.join(DATA, "footprint-map.tsv")] + sorted(glob.glob(os.path.join(DATA, "footprint-map-*.tsv")))
    for path in paths:
        if os.path.exists(path):
            for r in csv.DictReader(open(path), delimiter="\t"):
                fmap[(r["letter"], r["maker"], r["package"], r["supplier"], r["pins"], r.get("pdf") or "*")] = r
    return fmap


def lookup(fmap, letter, maker, package, supplier, pins, pdf="*"):
    for d in (pdf, "*"):
        for sup in (supplier, "*"):
            for p in (str(pins), "*"):
                m = fmap.get((letter, maker, package, sup, p, d))
                if m:
                    return m
    return None


def line_pins():
    """{(maker, package, supplier): set of (pdf file name, pin count)} over the `add` rows."""
    rows = common.load(specs.CANDIDATES)
    table = specs.load()
    out = collections.defaultdict(set)
    for r in rows:
        if r["reason"] != "add":
            continue
        s = table[specs.key(r)]
        mixed = any(s[c] for c in ("pinout_adj",))
        pin = s["pinout_adj"] if (specs.variant_of(r) == "adj" and mixed) else s["pinout"]
        out[(r["maker"], r["package"], r["supplier_package"])].add((os.path.basename(r["pdf"]), len(json.loads(pin))))
    return out


def main():
    letter = sys.argv[1]
    listing = "--list" in sys.argv
    only = None
    if "--only" in sys.argv:
        only = {tuple(l.rstrip("\n").split("\t")[:3]) for l in open(sys.argv[sys.argv.index("--only") + 1]) if l.strip()}
    pins_of = footprint_pins()
    fmap = load_map()
    counts = line_pins()
    n = missing = 0
    for line in open(os.path.join(DATA, "footprints-needed.txt")):
        if line.startswith("#") or not line.strip():
            continue
        f = line.rstrip("\n").split("\t")
        if f[0] != letter or (only is not None and (f[1], f[3], f[4]) not in only):
            continue
        n += 1
        bad = []
        for pdf, pc in sorted(counts[(f[1], f[3], f[4])]):
            m = lookup(fmap, f[0], f[1], f[3], f[4], pc, pdf)
            if not m or not m["footprint"]:
                bad.append(f"no footprint for {pc} pins ({pdf})")
            elif m["footprint"] not in pins_of:
                bad.append(f"unknown id {m['footprint']} ({pdf})")
            elif pins_of[m["footprint"]] != pc:
                bad.append(f"{m['footprint']} has {pins_of[m['footprint']]} pins, the pinout {pc} ({pdf})")
            elif listing:
                print("ok", f[1][:3], "|", f[3], "|", f[4], "|", pc, "|", pdf, "->", m["footprint"])
        if bad:
            missing += 1
            print("; ".join(bad) + ":", f[1], "|", f[3], "|", f[4])
    print(f"{n} lines, {missing} without a footprint")
    return 1 if missing else 0


if __name__ == "__main__":
    sys.exit(main())
