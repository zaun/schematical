"""plan-27-E section 4.2: move the footprints the tactile parts use into Parts/footprints.json.

usage: python3 -I append-footprints.py [<maker>...]

Appends to Parts/footprints.json (one line per footprint, the file's layout)
every footprint id that the tactile parts of Parts/switch.json name and
footprints.json does not already hold, taking the object from
planning/plan-27-data/footprints-*.json.  The maker arguments only label the
run; the source of truth is what switch.json names, so running it again
changes nothing.
"""
import glob
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
PARTS = os.path.join(ROOT, "schematical", "schematical", "Parts")


def main(makers):
    switch = json.load(open(os.path.join(PARTS, "switch.json")))
    used = []
    for d in switch["devices"]:
        if d["id"].startswith("tact_"):
            for part in d["parts"]:
                if part["footprint"] not in used:
                    used.append(part["footprint"])
    path = os.path.join(PARTS, "footprints.json")
    text = open(path).read()
    have = set(re.findall(r'"id":"([^"]+)"', text))
    ready = {}
    for f in sorted(glob.glob(os.path.join(HERE, "footprints-*.json"))):
        for o in json.load(open(f))["footprints"]:
            ready.setdefault(o["id"], o)
    new = [i for i in sorted(used) if i not in have]
    missing = [i for i in new if i not in ready]
    if missing:
        print("no ready object for:", ", ".join(missing))
        return 1
    if not new:
        print("no changes")
        return 0
    end = text.rstrip().rfind("]")
    head = text[:end].rstrip()
    lines = ",\n".join("    " + json.dumps(ready[i], ensure_ascii=False, separators=(",", ":")) for i in new)
    open(path, "w").write(head + ",\n" + lines + "\n  ]\n}\n")
    print(f"{len(new)} footprints appended ({' '.join(makers) or 'all makers'})")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
