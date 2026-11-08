"""plan-27 census, step 2: raw DigiKey pages -> candidates.tsv.

usage: python3 -I build.py <raw-dir> <out.tsv>

Reads every <raw-dir>/*.jsonl written by list.sh (one DigiKey search page per
line), merges the pages, collapses packing variants of one part (same maker +
part number) into one row, and writes one tab-separated row per part with the
DigiKey parameters the plan needs.  Nothing is dropped here except exact
duplicates: the in/out decision is the `decision` column, set by the rules in
plan-27-A section 3.
"""
import glob
import json
import os
import sys

COLS = ["maker", "mpn", "status", "qty", "series", "circuit", "function", "mounting",
        "actuator", "orientation", "termination", "outline", "rating", "illumination",
        "ingress", "height", "datasheet_url", "decision", "reason"]


def param(product, name):
    for p in product.get("Parameters", []):
        if p["ParameterText"] == name:
            return p["ValueText"]
    return ""


def main(raw_dir, out_path):
    parts = {}
    for path in sorted(glob.glob(os.path.join(raw_dir, "*.jsonl"))):
        for line in open(path):
            line = line.strip()
            if not line:
                continue
            page = json.loads(line)
            for p in page.get("Products", []):
                maker = p["Manufacturer"]["Name"]
                mpn = p["ManufacturerProductNumber"]
                key = (maker, mpn)
                qty = p.get("QuantityAvailable", 0) or 0
                row = parts.get(key)
                if row is not None:
                    row["qty"] = max(row["qty"], qty)
                    continue
                parts[key] = {
                    "maker": maker,
                    "mpn": mpn,
                    "status": p["ProductStatus"]["Status"],
                    "qty": qty,
                    "series": p.get("Series", {}).get("Name", ""),
                    "circuit": param(p, "Circuit"),
                    "function": param(p, "Switch Function"),
                    "mounting": param(p, "Mounting Type"),
                    "actuator": param(p, "Actuator Type"),
                    "orientation": param(p, "Actuator Orientation"),
                    "termination": param(p, "Termination Style"),
                    "outline": param(p, "Outline"),
                    "rating": param(p, "Contact Rating @ Voltage"),
                    "illumination": param(p, "Illumination"),
                    "ingress": param(p, "Ingress Protection"),
                    "height": param(p, "Actuator Height off PCB, Vertical"),
                    "datasheet_url": p.get("DatasheetUrl", ""),
                }
    with open(out_path, "w") as out:
        out.write("\t".join(COLS) + "\n")
        for key in sorted(parts, key=lambda k: (k[0], k[1])):
            row = parts[key]
            row["decision"] = ""
            row["reason"] = ""
            out.write("\t".join(str(row[c]).replace("\t", " ") for c in COLS) + "\n")
    print(len(parts), "parts")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
