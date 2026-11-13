#!/usr/bin/env python3
"""plan-34-F § 3: write Parts/resistor.json.

usage: python3 -I gen-parts.py --generic-only
       python3 -I gen-parts.py [<candidates.tsv>]   (default planning/plan-34-data/candidates.tsv)

--generic-only writes resistor.json with only GENERIC-RESISTOR's device
(§ 3.3). Without it, every `add` row of candidates.tsv becomes a part
(§ 3.1, § 3.2): the package, pinout and footprint from its specs.tsv row,
the datasheet id from datasheets/resistor/datasheets.tsv, the values from
the row. GENERIC-RESISTOR is always the file's last device. The full run
also adds each used PDF's entry to Parts/datasheets.json (id only if
absent). The footprints go in with fp-merge.py (F Phase 2's first task).

Pinouts (footprint-map.tsv `pinout`): `two-pin`, or an array's
`iso-8:1-8,2-7,…` / `bus-10:5-1,…;common=5,10`. Pins are 1..N, `inout`,
named by number, except an array's common pins, which share the name `C`
(resistor.c ties same-named pins into one node). Each distinct pin map
gets a name: its prefix (`iso-8`), with `-b`, `-c`, … for further maps of
the same prefix, in sorted order of the map strings.
"""
import csv
import json
import os
import re
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


GROUPS = [("< 1 kΩ", "lt-1k"), ("1 kΩ – 5 kΩ", "1k-5k"), ("5 kΩ – 10 kΩ", "5k-10k"),
          ("10 kΩ – 50 kΩ", "10k-50k"), ("50 kΩ – 100 kΩ", "50k-100k"),
          ("100 kΩ – 500 kΩ", "100k-500k"), ("500 kΩ – 1 MΩ", "500k-1m"), ("≥ 1 MΩ", "ge-1m")]
GROUP_SLUG = dict(GROUPS)
GROUP_ORDER = {g: i for i, (g, _) in enumerate(GROUPS)}
CATEGORY = {"chip": ("chip", "Chip"), "through-hole": ("axial", "Through-hole"), "array": ("array", "Array")}
CATEGORY_ORDER = {"chip": 0, "through-hole": 1, "array": 2}
SLUG = {"YAGEO": "yageo", "Vishay Dale": "vishay-dale", "Viking Tech": "viking"}


def kebab(text):
    return re.sub(r"[^a-z0-9]+", "-", text.lower()).strip("-")


def number(text):
    """'4700' -> 4700, '4.7' -> 4.7 (ints stay ints in the JSON)."""
    v = float(text)
    return int(v) if v == int(v) else v


def power_watts(text):
    """The first number of DigiKey's power column: '0.1W, 1/10W' -> 0.1, '62.5mW' -> 0.0625."""
    m = re.match(r"\s*([0-9.]+)\s*(m?)W", text or "")
    if not m:
        return None
    v = float(m.group(1)) / (1000 if m.group(2) else 1)
    return round(v, 6)


def parse_pinout(text):
    """('iso-8', n pins, [(a, b)...], {common pins}) from a map pinout string."""
    if text == "two-pin":
        return "two-pin", 2, [(1, 2)], set()
    prefix, _, rest = text.partition(":")
    pairs_text, _, common_text = rest.partition(";common=")
    n = int(prefix.split("-")[1])
    pairs = [tuple(int(x) for x in p.split("-")) for p in pairs_text.split(",")]
    common = {int(x) for x in common_text.split(",")} if common_text else set()
    return prefix, n, pairs, common


def pinout_json(n, common):
    return [{"number": k, "name": "C" if k in common else str(k), "role": "inout"} for k in range(1, n + 1)]


def pairs_json(pairs, common):
    name = lambda k: "C" if k in common else str(k)
    out = []
    for a, b in pairs:
        pair = {"a": name(a), "b": name(b)}
        if pair not in out:
            out.append(pair)
    return out


def full(cand_path):
    sys.path.append(HERE)
    import importlib.util
    spec = importlib.util.spec_from_file_location("specs_fill", os.path.join(HERE, "specs-fill.py"))
    fill = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(fill)
    fmap = fill.load_map()
    placed = {r["path"]: r for r in csv.DictReader(open(os.path.join(ROOT, "datasheets", "resistor",
                                                                     "datasheets.tsv")), delimiter="\t")}
    rows = [r for r in csv.DictReader(open(cand_path), delimiter="\t") if r["decision"] == "add"]
    # Pinout names: one per distinct map string, per prefix.
    maps = {}
    for r in rows:
        m = fill.entry(fmap, r)
        if m is None:
            sys.exit(f"gen-parts: {r['mpn']} has no footprint-map entry {fill.line_key(r)}")
        r["_map"] = m
        maps.setdefault(parse_pinout(m["pinout"])[0], set()).add(m["pinout"])
    pin_name = {}
    for prefix, texts in maps.items():
        for i, t in enumerate(sorted(texts)):
            pin_name[t] = prefix if i == 0 else f"{prefix}-{chr(ord('b') + i - 1)}"
    devices = {}
    sheets = {}
    for r in rows:
        m = r["_map"]
        prefix, n, pairs, common = parse_pinout(m["pinout"])
        pname = pin_name[m["pinout"]]
        cat_slug, leaf = CATEGORY[r["category"]]
        jumper = r["tolerance"] == "Jumper"
        comp = "jumper" if jumper else kebab(r["composition"])
        key = (CATEGORY_ORDER[r["category"]], comp, pname, GROUP_ORDER[r["bucket"]])
        if key not in devices:
            parts_id = [cat_slug] + ([comp] if comp else []) + [pname, GROUP_SLUG[r["bucket"]]]
            if jumper:
                desc = "Zero-ohm jumper"
            elif r["category"] == "array":
                desc = f"Resistor array, {len(pairs_json(pairs, common))} {r['circuit'].lower() or 'resistors'}"
            else:
                kind = "chip resistor" if r["category"] == "chip" else "axial resistor"
                desc = f"{r['composition']} {kind}"
            devices[key] = {
                "id": "-".join(parts_id),
                "function": r["bucket"],
                "description": desc,
                "drawer": ["Resistors", leaf],
                "pinouts": {pname: pinout_json(n, common)},
                "resistors": pairs_json(pairs, common),
                "parts": [],
            }
        pdf = r["pdf"]
        sid = f"{SLUG[r['maker']]}-{kebab(os.path.splitext(os.path.basename(pdf))[0])}"
        sheets[sid] = placed[pdf]["url"]
        part = {"name": r["mpn"], "manufacturer": r["maker"], "package": m["part_package"],
                "pinout": pname, "footprint": m["footprint"], "datasheet": sid, "ohms": number(r["ohms"])}
        if not jumper:
            part["tolerance"] = number(r["tolerance"].strip("±%"))
        w = power_watts(r["power"])
        if w:
            part["power"] = w
        if r["military"]:
            part["flags"] = ["military"]
        devices[key]["parts"].append(part)
    out = []
    for key in sorted(devices):
        d = devices[key]
        d["parts"].sort(key=lambda p: (p["ohms"], p["name"]))
        out.append(d)
    ids = [d["id"] for d in out]
    assert len(ids) == len(set(ids)), "duplicate device ids"
    write(out + [generic_device()])
    added = add_datasheets(sheets)
    print(f"gen-parts: {OUT}: {len(out) + 1} devices, {len(rows) + 1} parts; "
          f"{len(sheets)} datasheets used, {added} added to datasheets.json")


def add_datasheets(sheets):
    path = os.path.join(PARTS, "datasheets.json")
    text = open(path).read()
    have = set(re.findall(r'"id": "([^"]+)"', text))
    new = [(i, u) for i, u in sorted(sheets.items()) if i not in have]
    if not new:
        return 0
    doc = json.loads(text)
    doc["datasheets"].extend({"id": i, "url": u} for i, u in new)
    doc["datasheets"].sort(key=lambda d: d["id"])
    lines = ",\n".join("    " + json.dumps(d, ensure_ascii=False) for d in doc["datasheets"])
    with open(path, "w") as f:
        f.write('{\n  "format": "schematical-datasheets",\n  "version": 1,\n  "datasheets": [\n'
                + lines + "\n  ]\n}\n")
    return len(new)


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
    full(sys.argv[1] if len(sys.argv) > 1 else os.path.join(DATA, "candidates.tsv"))


if __name__ == "__main__":
    main()
