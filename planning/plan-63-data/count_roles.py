#!/usr/bin/env python3 -I
"""Counts the plan-63 sub-plans cite: pin roles per kind file, and the parts whose device or part
data names an output enable ("outputEnable", "oe", "tristate", "oeLatch"). Run from the repo root:
python3 -I planning/plan-63-data/count_roles.py > planning/plan-63-data/count_roles.txt"""
import json, glob, collections, os
PARTS = "schematical/schematical/Parts"
OE = {"outputEnable", "oe", "tristate", "oeLatch"}
def has(o):
    if isinstance(o, dict):
        return any(k in OE or has(v) for k, v in o.items())
    if isinstance(o, list):
        return any(has(x) for x in o)
    return False
total = collections.Counter()
print("kind: parts | pin roles (pin entries over every pinout) | parts naming an output enable")
for f in sorted(glob.glob(PARTS + "/*.json")):
    name = os.path.basename(f)[:-5]
    if name in ("footprints", "datasheets"):
        continue
    d = json.load(open(f))
    roles = collections.Counter(); parts = 0; oe = 0
    for dev in d.get("devices", []):
        for pins in dev.get("pinouts", {}).values():
            for p in pins:
                roles[p.get("role")] += 1
        n = len(dev.get("parts", []))
        parts += n
        rest = {k: v for k, v in dev.items() if k not in ("pinouts", "parts", "drawer")}
        if has(rest) or any(has(p) for p in dev.get("parts", [])):
            oe += n
    total.update(roles)
    print(f"{name}: {parts} | {dict(roles)} | oe {oe}")
print("all kinds, pin entries by role:", dict(total))
