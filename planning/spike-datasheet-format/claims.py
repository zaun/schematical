#!/usr/bin/env python3
"""Print what a datasheet file must claim: the catalog devices and parts
whose "datasheet" is the given id, with their pinouts and footprints.

Usage: python3 -I claims.py <datasheet id>...
"""
import glob
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
PARTS = os.path.join(HERE, "..", "..", "schematical", "schematical", "Parts")


def main(ids):
    files = [f for f in sorted(glob.glob(os.path.join(PARTS, "*.json")))
             if os.path.basename(f) not in ("datasheets.json", "footprints.json")]
    for want in ids:
        devices, parts, pinouts, footprints = [], [], set(), set()
        for f in files:
            with open(f, encoding="utf-8") as fh:
                data = json.load(fh)
            for dev in data.get("devices", []):
                mine = [p for p in dev["parts"] if p.get("datasheet") == want]
                if mine:
                    devices.append("%s:%s" % (data["kind"], dev["id"]))
                    parts += [p["name"] for p in mine]
                    pinouts.update("%s:%s" % (dev["id"], p["pinout"]) for p in mine)
                    footprints.update(p.get("footprint", "") for p in mine)
        print("%s:" % want)
        print('  "devices": %s,' % json.dumps(devices))
        print('  "parts": %s,' % json.dumps(parts))
        print("  pinouts used: %s" % ", ".join(sorted(pinouts)))
        print("  footprints used: %s" % ", ".join(sorted(f for f in footprints if f)))


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    main(sys.argv[1:])
