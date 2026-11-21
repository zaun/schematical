#!/usr/bin/env python3
"""plan-42-D/E/F Phase 2: replace objects of footprints-<L>.json by the corrected
objects of footprints-fix-*.json (hand checks), by id. Prints each replaced id
and removes the fix files."""
import glob
import json
import os

DATA = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
fixes = {}
paths = sorted(glob.glob(os.path.join(DATA, "footprints-fix-*.json")))
for p in paths:
    for o in json.load(open(p))["footprints"]:
        fixes[o["id"]] = o
done = set()
for L in "DEF":
    path = os.path.join(DATA, f"footprints-{L}.json")
    doc = json.load(open(path))
    out = []
    for o in doc["footprints"]:
        if o["id"] in fixes:
            out.append(fixes[o["id"]])
            done.add(o["id"])
            print("replaced", L, o["id"])
        else:
            out.append(o)
    with open(path, "w") as f:
        f.write('{\n  "format": "schematical-footprints",\n  "version": 1,\n  "footprints": [\n')
        f.write(",\n".join("    " + json.dumps(o, ensure_ascii=False, separators=(",", ":")) for o in out))
        f.write("\n  ]\n}\n")
for i in sorted(set(fixes) - done):
    print("fix id not found in any letter:", i)
for p in paths:
    os.remove(p)
