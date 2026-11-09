"""plan-27-B section 4.3: every `add` row of a maker has exactly one complete spec row.

usage: python3 -I specs-check.py --maker M [--maker M2 ...] [--tsv candidates.tsv] [--specs specs.tsv]

For every `add` row of the named makers (maker name starts with M): exactly one
row of specs.tsv with the same (full DigiKey) maker, series and mounting whose
`mpn_regex` fullmatches the part number; that row has `terminals` (2 or 4),
`pins` (A=..;B=.., exactly `terminals` distinct numbers 1..terminals) and
`footprint`, an id in planning/plan-27-data/footprints-*.json or
Parts/footprints.json whose pads cover every number.  Also: the footprint's
`pins` equals `terminals`.  Prints `specs ok` or one line per problem (exit 1).
"""
import argparse
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


def footprints(extra=()):
    out = {}
    for p in list(extra) + sorted(glob.glob(os.path.join(DATA, "footprints-*.json"))) + [PARTS_FP]:
        for o in json.load(open(p))["footprints"]:
            out.setdefault(o["id"], o)
    return out


def pads(fp):
    if fp["style"] == "explicit":
        return {str(p["pin"]) for p in fp["pads"] if p["pin"] != 0}
    return {str(i) for i in range(1, fp["pins"] + 1)}


def parse_pins(text):
    m = re.fullmatch(r"A=(\d+(?:,\d+)*);B=(\d+(?:,\d+)*)", text)
    if not m:
        return None
    return m.group(1).split(","), m.group(2).split(",")


def problems_of(spec, fps):
    out = []
    t = spec["terminals"]
    if t not in ("2", "4"):
        out.append(f"terminals {t!r} not 2 or 4")
        return out
    pins = parse_pins(spec["pins"])
    if pins is None:
        out.append(f"pins {spec['pins']!r} not A=..;B=..")
        return out
    a, b = pins
    nums = a + b
    if len(nums) != int(t) or len(set(nums)) != len(nums):
        out.append(f"pins {spec['pins']!r} does not list {t} distinct numbers")
    if sorted(nums, key=int) != [str(i) for i in range(1, int(t) + 1)]:
        out.append(f"pins {spec['pins']!r} is not 1..{t}")
    if len(a) != len(b):
        out.append(f"pins {spec['pins']!r}: sides differ in size")
    fp = fps.get(spec["footprint"])
    if fp is None:
        out.append(f"footprint {spec['footprint']!r} not in any footprints file")
    else:
        if fp["pins"] != int(t):
            out.append(f"footprint {fp['id']} has {fp['pins']} pins, not {t}")
        miss = [n for n in nums if n not in pads(fp)]
        if miss:
            out.append(f"footprint {fp['id']} has no pad {','.join(miss)}")
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--maker", action="append", required=True)
    ap.add_argument("--tsv", default=os.path.join(DATA, "candidates.tsv"))
    ap.add_argument("--specs", default=os.path.join(DATA, "specs.tsv"))
    ap.add_argument("--footprints", action="append", default=[],
                    help="another footprints JSON file to search first (a maker's work file)")
    a = ap.parse_args()
    rows = list(csv.DictReader(open(a.tsv), delimiter="\t"))
    specs = list(csv.DictReader(open(a.specs), delimiter="\t")) if os.path.exists(a.specs) else []
    fps = footprints(a.footprints)
    bad = []
    checked = {}
    n = 0
    for r in rows:
        maker = next((m for m in a.maker if r["maker"].startswith(m)), None)
        if maker is None:
            continue
        if r["decision"] in ("pending", "candidate", "check"):
            bad.append(f"{r['mpn']}: still {r['decision']}")
            continue
        if r["decision"] != "add":
            continue
        n += 1
        match = [i for i, s in enumerate(specs)
                 if s["maker"] == r["maker"] and s["series"] == r["series"] and s["mounting"] == r["mounting"]
                 and re.fullmatch(s["mpn_regex"], r["mpn"])]
        if len(match) != 1:
            bad.append(f"{r['mpn']} ({r['series']}, {r['mounting']}): {len(match)} spec rows match")
            continue
        i = match[0]
        if i not in checked:
            checked[i] = problems_of(specs[i], fps)
        for p in checked[i]:
            bad.append(f"{r['mpn']}: spec row {i + 2} ({specs[i]['series']}): {p}")
    for line in bad:
        print(line)
    if bad:
        print(f"{len(bad)} problems in {n} add rows")
        return 1
    print(f"specs ok ({n} add rows)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
