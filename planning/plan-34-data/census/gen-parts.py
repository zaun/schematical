#!/usr/bin/env python3
"""plan-34-F § 3: write Parts/resistor.json.

usage: python3 -I gen-parts.py --generic-only
       python3 -I gen-parts.py [--dry-run]

--generic-only writes resistor.json with only GENERIC-RESISTOR's device
(§ 3.3). Without it, every `add` row of candidates.tsv becomes a part
(§ 3.1, § 3.2): the package, pinout and footprint from its specs.tsv row,
the datasheet id from datasheets/resistor/datasheets.tsv, the values from
the row. GENERIC-RESISTOR is always the file's last device.
"""
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
PARTS = os.path.join(ROOT, "schematical", "schematical", "Parts")
OUT = os.path.join(PARTS, "resistor.json")


def two_pin():
    return [{"number": 1, "name": "1", "role": "inout"},
            {"number": 2, "name": "2", "role": "inout"}]


def generic_device():
    """§ 3.3: Axial and 1 kΩ by default, the pulls' six footprint choices."""
    return {
        "id": "resistor-generic",
        "function": "Generic",
        "description": "Generic resistor",
        "drawer": ["Resistors"],
        "pinouts": {"two-pin": two_pin()},
        "resistors": [{"a": "1", "b": "2"}],
        "parts": [{
            "name": "GENERIC-RESISTOR", "manufacturer": "Generic", "package": "Axial",
            "pinout": "two-pin", "footprint": "RES-AXIAL-10.16",
            "footprintChoices": [
                {"package": "0402", "footprint": "RES-0402"},
                {"package": "0603", "footprint": "RES-0603"},
                {"package": "0805", "footprint": "RES-0805"},
                {"package": "1206", "footprint": "RES-1206"},
                {"package": "Axial", "footprint": "RES-AXIAL-10.16"},
                {"package": "Radial", "footprint": "RES-RADIAL-2.54"},
            ],
            "ohms": 1000,
        }],
    }


def write(devices):
    doc = {"format": "schematical-parts", "version": 1, "kind": "resistor", "devices": devices}
    with open(OUT, "w") as f:
        json.dump(doc, f, ensure_ascii=False, indent=1)
        f.write("\n")


def main():
    if "--generic-only" in sys.argv:
        write([generic_device()])
        print(f"gen-parts: {OUT}: 1 device, 1 part (GENERIC-RESISTOR)")
        return
    sys.exit("gen-parts: the full run is plan-34-F Phase 2 (needs specs.tsv, footprints-C/D.json)")


if __name__ == "__main__":
    main()
