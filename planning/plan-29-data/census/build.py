"""plan-29 census, step 2: raw DigiKey pages -> all.tsv.

usage: python3 -I build.py <raw-dir> <out.tsv>

Reads every <raw-dir>/<category>-<maker>.jsonl written by list.sh (one
DigiKey search page per line), merges the pages, collapses packing variants
of one part (same maker + part number) into one row, and writes one
tab-separated row per part with the DigiKey parameters the plan needs.
Nothing is dropped here except exact duplicates: the in/out decision is the
`decision` column, set by select.py and then plan-29-A.
"""
import glob
import json
import os
import sys

CATEGORY = {"280": "single", "286": "array", "299": "bridge", "287": "zener", "295": "zener-array"}

COLS = ["category", "maker", "mpn", "status", "qty", "series", "technology", "speed",
        "configuration", "diode_type", "vr", "io", "vf", "trr", "vz", "tolerance", "power",
        "grade", "qualification", "mounting", "package", "supplier_package",
        "datasheet_url", "decision", "reason"]

# DigiKey parameter names per column; the first one present wins.
PARAMS = {
    "technology": ["Technology"],
    "speed": ["Speed"],
    "configuration": ["Diode Configuration", "Configuration"],
    "diode_type": ["Diode Type"],
    "vr": ["Voltage - DC Reverse (Vr) (Max)", "Voltage - Peak Reverse (Max)"],
    "io": ["Current - Average Rectified (Io)", "Current - Average Rectified (Io) (per Diode)"],
    "vf": ["Voltage - Forward (Vf) (Max) @ If"],
    "trr": ["Reverse Recovery Time (trr)"],
    "vz": ["Voltage - Zener (Nom) (Vz)"],
    "tolerance": ["Tolerance"],
    "power": ["Power - Max"],
    "grade": ["Grade"],
    "qualification": ["Qualification"],
    "mounting": ["Mounting Type"],
    "package": ["Package / Case"],
    "supplier_package": ["Supplier Device Package"],
}


def param(product, names):
    by = {p["ParameterText"]: p["ValueText"] for p in product.get("Parameters", [])}
    for n in names:
        if n in by:
            return by[n]
    return ""


def main(raw_dir, out_path):
    parts = {}
    for path in sorted(glob.glob(os.path.join(raw_dir, "*.jsonl"))):
        category = CATEGORY[os.path.basename(path).split("-")[0]]
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
                row = {"category": category, "maker": maker, "mpn": mpn,
                       "status": p["ProductStatus"]["Status"], "qty": qty,
                       "series": (p.get("Series") or {}).get("Name", ""),
                       "datasheet_url": p.get("DatasheetUrl", "") or ""}
                for col, names in PARAMS.items():
                    row[col] = param(p, names)
                parts[key] = row
    with open(out_path, "w") as out:
        out.write("\t".join(COLS) + "\n")
        for key in sorted(parts):
            row = parts[key]
            row["decision"] = ""
            row["reason"] = ""
            out.write("\t".join(str(row[c]).replace("\t", " ") for c in COLS) + "\n")
    print(len(parts), "parts")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
