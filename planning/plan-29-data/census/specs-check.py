"""plan-29-B section 4 (Phase 1): every `add` row has a complete spec row.

usage: python3 -I specs-check.py [--maker M] [--category C] [--mounting smd|th]
                                 [--tsv candidates.tsv] [--specs specs.tsv] [--footprints F.json ...]

For every row of the scope (maker column starts with M; category C; mounting
smd = "Surface Mount" or "Surface Mount, Wettable Flank", th = "Through Hole";
a blank mounting is in scope only with no --mounting): no row is still
pending / candidate / check, and each `add` row's spec row (same maker,
category, datasheet path, supplier package) has `package`, `pinout`, `pins`,
`footprint`; `pins` is `<number>:<name>,...` with distinct numbers 1..N
(or `<config>=<map>;...`, one map per configuration, `pinout` naming each); the
footprint is an id in planning/plan-29-data/footprints-*.json or
Parts/footprints.json with N pins whose pad designators equal those numbers;
and one `pinout` name has one `pins` map in the whole file.  Prints
`specs ok (<n> add rows)` or one line per problem (exit 1).
"""
import argparse
import collections
import csv
import glob
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
PARTS_FP = os.path.join(ROOT, "schematical", "schematical", "Parts", "footprints.json")
PLACED = os.path.join(ROOT, "datasheets", "diode", "datasheets.tsv")
MOUNT = {"smd": ("Surface Mount", "Surface Mount, Wettable Flank"), "th": ("Through Hole",)}


def footprints(extra=()):
    out = {}
    for p in list(extra) + sorted(glob.glob(os.path.join(DATA, "footprints-*.json"))) + [PARTS_FP]:
        for o in json.load(open(p))["footprints"]:
            out.setdefault(o["id"], o)
    return out


def pads(fp):
    if fp["style"] == "explicit":
        return {str(p["pin"]) for p in fp["pads"] if p.get("pin", 0) not in (0, None)}
    return {str(i) for i in range(1, fp["pins"] + 1)}


def parse_pins(text):
    if not re.fullmatch(r"\d+:[A-Za-z0-9+\-]+(,\d+:[A-Za-z0-9+\-]+)*", text or ""):
        return None
    return [tuple(x.split(":")) for x in text.split(",")]


def problems_of(spec, fps):
    out = []
    for col in ("package", "pinout", "pins", "footprint"):
        if not spec[col]:
            out.append(f"no {col}")
    if out:
        return out
    fp = fps.get(spec["footprint"])
    if fp is None:
        out.append(f"footprint {spec['footprint']!r} not in any footprints file")
    maps = [x.split("=", 1)[-1] for x in spec["pins"].split(";")]
    if len(maps) != len(spec["pinout"].split(";")):
        out.append(f"pinout {spec['pinout']!r} does not name each of {len(maps)} maps")
    for text in maps:
        pins = parse_pins(text)
        if pins is None:
            out.append(f"pins {text!r} not <number>:<name>,...")
            continue
        nums = [n for n, _ in pins]
        if sorted(nums, key=int) != [str(i) for i in range(1, len(nums) + 1)]:
            out.append(f"pins {text!r} are not 1..{len(nums)}")
        if fp is not None and pads(fp) != set(nums):
            out.append(f"footprint {fp['id']} pads {sorted(pads(fp), key=int)} != pins {nums}")
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--maker", action="append", default=[])
    ap.add_argument("--category", action="append", default=[])
    ap.add_argument("--mounting", choices=sorted(MOUNT))
    ap.add_argument("--tsv", default=os.path.join(DATA, "candidates.tsv"))
    ap.add_argument("--specs", default=os.path.join(DATA, "specs.tsv"))
    ap.add_argument("--footprints", action="append", default=[])
    a = ap.parse_args()
    rows = list(csv.DictReader(open(a.tsv), delimiter="\t"))
    specs = list(csv.DictReader(open(a.specs), delimiter="\t"))
    paths = {r["url"]: r["path"] for r in csv.DictReader(open(PLACED), delimiter="\t")}
    index = {(s["maker"], s["category"], s["datasheet"], s["supplier_package"]): i for i, s in enumerate(specs)}
    fps = footprints(a.footprints)
    bad = []
    maps = collections.defaultdict(set)
    for s in specs:
        if s["pinout"]:
            for name, text in zip(s["pinout"].split(";"), s["pins"].split(";")):
                maps[name].add(text.split("=", 1)[-1])
    for name, m in sorted(maps.items()):
        if len(m) > 1:
            bad.append(f"pinout {name} has {len(m)} pin maps: {sorted(m)}")
    checked = {}
    n = 0
    for r in rows:
        if a.maker and not any(r["maker"].startswith(m) for m in a.maker):
            continue
        if a.category and r["category"] not in a.category:
            continue
        if a.mounting and r["mounting"] not in MOUNT[a.mounting]:
            continue
        if r["decision"] in ("pending", "candidate", "check"):
            bad.append(f"{r['mpn']}: still {r['decision']}")
            continue
        if r["decision"] != "add":
            continue
        n += 1
        key = (r["maker"], r["category"], paths.get(r["datasheet_url"], r["datasheet_url"]), r["supplier_package"])
        i = index.get(key)
        if i is None:
            bad.append(f"{r['mpn']}: no spec row for {key}")
            continue
        if i not in checked:
            checked[i] = problems_of(specs[i], fps)
            for p in checked[i]:
                bad.append(f"spec row {i + 2} ({os.path.basename(specs[i]['datasheet'])}, {specs[i]['supplier_package']}): {p}")
    for line in bad:
        print(line)
    if bad:
        print(f"{len(bad)} problems in {n} add rows")
        return 1
    print(f"specs ok ({n} add rows)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
