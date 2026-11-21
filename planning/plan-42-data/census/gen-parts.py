#!/usr/bin/env python3
"""plan-42-H § 3: write Parts/regulator.json.

usage: python3 -I gen-parts.py --generics-only
       python3 -I gen-parts.py [<candidates.tsv>]   (default planning/plan-42-data/candidates.tsv)

--generics-only writes regulator.json with only the three generic devices
(§ 3.2). Without it, every `add` row of candidates.tsv becomes a part (§ 3.1):
the label, pinouts, regulator keys, Vref, minimum input and (where DigiKey
gives none) dropout from its specs.tsv row (plan-42-C), the footprint from
footprint-map.tsv, the datasheet id from datasheets/regulator/datasheets.tsv,
and the facts from DigiKey's columns; a fixed output of a row listed in
variant-overrides.tsv is that file's `vout`, and a multi-output part's
`vout` list comes from DigiKey's "Voltage - Output (Min/Fixed)" text in the
raw listing (raw/*.jsonl.gz). The generics are always the file's last three
devices. The full run also adds each used PDF's entry to Parts/datasheets.json
(id only if absent) and appends the ready footprint objects (footprints-D / E /
F.json) that the map uses to Parts/footprints.json (id only if absent).
"""
import collections
import csv
import glob
import gzip
import importlib.util
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.append(HERE)
import common  # noqa: E402
import specs  # noqa: E402

PARTS = os.path.join(common.ROOT, "schematical", "schematical", "Parts")
OUT = os.path.join(PARTS, "regulator.json")
LIMITS = {"vout": 100, "vref": 10, "voutMax": 1000, "vinMin": 100, "vinMax": 1000, "dropout": 10, "iout": 100,
          "iq": 1, "fsw": 1e8}
LEAF_LINEAR = {"fixed": "Fixed", "adj": "Adjustable"}
LEAF_SWITCH = {"buck": "Buck", "boost": "Boost", "buck-boost": "Buck-boost", "charge-pump": "Charge pump",
               "sepic": "SEPIC"}


def load_module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


fpcheck = load_module("fpcheck", os.path.join(HERE, "fp-check.py"))
fpmerge = load_module("fpmerge", os.path.join(HERE, "fp-merge.py"))


def kebab(text):
    return re.sub(r"[^a-z0-9]+", "-", text.lower()).strip("-")


def number(text):
    """A DigiKey fact as a JSON number (None when absent or 0)."""
    if text in ("", None):
        return None
    v = float(text)
    if v == 0:
        return None
    v = float(f"{v:.6g}")
    return int(v) if v == int(v) else v


PACKAGE_RULES = [
    (r"^TSOT", "TSOT-23"), (r"^SOT-23-THIN", "TSOT-23"), (r"^SOT-?23", "SOT-23"), (r"^SC-70", "SC-70"),
    (r"^SOT-223", "SOT-223"), (r"^TO-220", "TO-220"), (r"^TO-263", "TO-263"), (r"^TO-252", "TO-252"),
    (r"^(TO-243|SOT-89)", "SOT-89"), (r"^TO-92", "TO-92"), (r"^S-?PAK", "S-PAK"), (r"^SO$", "SOIC"),
]


def package_family(supplier):
    """The part's `package` (§ docs/parts-format.md: no pin count, size or material)."""
    s = re.sub(r"[®™]", "", supplier)
    s = re.sub(r"\s*\(.*?\)", "", s).strip()
    s = re.sub(r"^\d+-", "", s)
    s = re.sub(r"-(EP|WD|UD|WQ)$", "", s)
    for rx, fam in PACKAGE_RULES:
        if re.match(rx, s):
            return fam
    return s


def load_raw():
    """mpn -> the raw listing's parameters, for the multi-output voltage lists."""
    out = {}
    for path in sorted(glob.glob(os.path.join(common.DATA, "raw", "*.jsonl.gz"))):
        for line in gzip.open(path, "rt"):
            d = json.loads(line)
            out.setdefault(d["mpn"], d["params"])
    return out


def volts_list(text):
    """The numbers of a DigiKey voltage text: `1.8V, 3.3V` -> [1.8, 3.3]."""
    out = []
    for item in (text or "").split(","):
        m = re.match(r"\s*([0-9.]+)\s*(m?)V", item)
        if m:
            v = float(m.group(1)) / (1000 if m.group(2) else 1)
            out.append(float(f"{v:.6g}"))
    return out


def line_letters():
    out = {}
    for line in open(os.path.join(common.DATA, "footprints-needed.txt")):
        if line.startswith("#") or not line.strip():
            continue
        f = line.rstrip("\n").split("\t")
        out[(f[1], f[3], f[4])] = f[0]
    return out


def pins_json(pins):
    out = []
    for num, name, role in pins:
        pin = {"name": name, "role": role}
        pin = {"ball": num, **pin} if isinstance(num, str) else {"number": num, **pin}
        out.append(pin)
    return out


def drawer_of(count, rtype, adj):
    """§ 3.3: (drawer, function) of a device."""
    if rtype == "linear":
        return ["Power", "Regulators", "Linear"], ("Multi-output" if count > 1 else LEAF_LINEAR["adj" if adj else "fixed"])
    return ["Power", "Regulators", "Switching"], ("Multi-output" if count > 1 else LEAF_SWITCH[rtype])


def describe(rtype, adj, count):
    kind = {"linear": "Linear", "buck": "Buck", "boost": "Boost", "buck-boost": "Buck-boost",
            "charge-pump": "Charge pump", "sepic": "SEPIC"}[rtype]
    out = f"{kind} regulator, {'adjustable' if adj else 'fixed'} output"
    if count > 1:
        out = f"{kind} regulator, {count} outputs, {'adjustable' if adj else 'fixed'}"
    return out


def check_limits(r, part):
    for key, limit in LIMITS.items():
        values = part.get(key)
        for v in (values if isinstance(values, list) else [values]):
            if v is not None and not 0 < v <= limit and not (key == "dropout" and v == 0):
                sys.exit(f"gen-parts: {r['mpn']}: \"{key}\" {v} is outside (0, {limit}]")


def footprint_of(fmap, letters, r, pins):
    letter = letters[(r["maker"], r["package"], r["supplier_package"])]
    m = fpcheck.lookup(fmap, letter, r["maker"], r["package"], r["supplier_package"], pins,
                       os.path.basename(r["pdf"]))
    if m is None or not m["footprint"]:
        sys.exit(f"gen-parts: {r['mpn']} has no footprint-map row ({r['maker']} | {r['package']} | "
                 f"{r['supplier_package']} | {pins} pins)")
    return m["footprint"]


def generic_devices():
    choices = [{"package": "TO-220", "footprint": "TO-220-3"}, {"package": "DPAK", "footprint": "DPAK-3"}]
    fixed = [{"number": 1, "name": "IN", "role": "power"}, {"number": 2, "name": "GND", "role": "ground"},
             {"number": 3, "name": "OUT", "role": "output"}]
    adj = [{"number": 1, "name": "ADJ", "role": "input"}, {"number": 2, "name": "OUT", "role": "output"},
           {"number": 3, "name": "IN", "role": "power"}]
    out = []
    for dev_id, name, what, pins, element, analog, keys in [
        ("generic-reg-5v", "GENERIC-REG-5V", "Generic 5 V linear regulator", fixed,
         {"type": "linear", "in": "IN", "out": "OUT"}, [], {"vout": 5, "dropout": 2}),
        ("generic-reg-3v3", "GENERIC-REG-3V3", "Generic 3.3 V linear regulator", fixed,
         {"type": "linear", "in": "IN", "out": "OUT"}, [], {"vout": 3.3, "dropout": 2}),
        ("generic-reg-adj", "GENERIC-REG-ADJ", "Generic adjustable linear regulator (LM317 style)", adj,
         {"type": "linear", "in": "IN", "out": "OUT", "adjust": "ADJ", "adjustTo": "output"}, ["ADJ"],
         {"vref": 1.25, "dropout": 2}),
    ]:
        part = {"name": name, "manufacturer": "Generic", "package": "TO-220", "pinout": "p3",
                "footprint": "TO-220-3", "footprintChoices": choices}
        part.update(keys)
        out.append({"id": dev_id, "function": "Generic", "description": what,
                    "drawer": ["Power", "Regulators", "Linear"], "pinouts": {"p3": pins},
                    "regulators": [element], "analog": analog, "parts": [part]})
    return out


def write(devices):
    doc = {"format": "schematical-parts", "version": 1, "kind": "regulator", "devices": devices}
    with open(OUT, "w") as f:
        json.dump(doc, f, ensure_ascii=False, indent=1)
        f.write("\n")


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
        f.write('{\n  "format": "schematical-datasheets",\n  "version": 1,\n  "datasheets": [\n' + lines + "\n  ]\n}\n")
    return len(new)


def merge_footprints(used):
    target = os.path.join(PARTS, "footprints.json")
    paths = [os.path.join(common.DATA, f"footprints-{L}.json") for L in "DEF"]
    have = set(re.findall(r'"id":"([^"]+)"', open(target).read()))
    need = {i for i in used if i not in have}
    return fpmerge.merge(target, paths, only=need) if need else 0


def full(cand_path, generics_only):
    if generics_only:
        write(generic_devices())
        print(f"gen-parts: {OUT}: 3 devices, 3 parts (the generics)")
        return
    table = specs.load()
    rows = [r for r in common.load(cand_path) if r["reason"] == "add"]
    overrides = specs.variant_overrides()
    fmap = fpcheck.load_map()
    letters = line_letters()
    raw = load_raw()
    placed = {r["path"]: r for r in common.load(common.PLACED)}
    devices = {}
    pinout_names = {}
    sheets = {}
    used_fp = set()
    names = collections.Counter(r["mpn"] for r in rows)
    clash = [n for n, c in names.items() if c > 1]
    if clash:
        sys.exit(f"gen-parts: part names listed twice: {clash[:10]}")
    for r in rows:
        s = table[specs.key(r)]
        variant = specs.variant_of(r)
        mixed = bool(s["pinout_adj"])
        use_adj_cols = variant == "adj" and mixed
        pins = json.loads(s["pinout_adj"] if use_adj_cols else s["pinout"])
        regs = json.loads(s["regulators_adj"] if use_adj_cols else s["regulators"])
        analog = json.loads((s["analog_adj"] if use_adj_cols else s["analog"]) or "[]")
        adj = variant == "adj"
        rtype = regs[0]["type"]
        label = s["label"]
        dkey = (r["maker"], label, adj, json.dumps(regs, sort_keys=True), json.dumps(analog))
        if dkey not in devices:
            drawer, function = drawer_of(len(regs), rtype, adj)
            base = f"reg-{common.SLUG[r['maker']]}-{kebab(label)}" + ("-adj" if adj else "")
            used = {d["id"] for d in devices.values()}
            dev_id = base
            n = 2
            while dev_id in used:
                dev_id = f"{base}-{n}"
                n += 1
            devices[dkey] = {"id": dev_id, "function": function, "description": describe(rtype, adj, len(regs)),
                             "drawer": drawer, "pinouts": {}, "regulators": regs, "analog": analog, "parts": []}
        d = devices[dkey]
        sig = json.dumps(pins)
        pnames = pinout_names.setdefault(d["id"], {})
        if sig not in pnames:
            k = sum(1 for v in pnames.values() if v.startswith(f"p{len(pins)}"))
            pnames[sig] = f"p{len(pins)}" if k == 0 else f"p{len(pins)}-{k + 1}"
            d["pinouts"][pnames[sig]] = pins_json(pins)
        fp = footprint_of(fmap, letters, r, len(pins))
        used_fp.add(fp)
        stem = os.path.splitext(os.path.basename(r["pdf"]))[0]
        sid = f"{common.SLUG[r['maker']]}-{kebab(stem)}"
        sheets[sid] = re.sub(r"^http://", "https://", placed[r["pdf"]]["url"])
        part = {"name": r["mpn"], "manufacturer": r["maker"], "package": package_family(r["supplier_package"]),
                "pinout": pnames[sig], "footprint": fp, "datasheet": sid}
        keys = {}
        if adj:
            keys["vref"] = number(s["vref"])
            keys["voutMax"] = number(r["vout_max_v"])
        else:
            if r["mpn"] in overrides and overrides[r["mpn"]]["vout"]:
                vout = [float(overrides[r["mpn"]]["vout"])] * len(regs)
            elif len(regs) > 1:
                vout = volts_list(raw[r["mpn"]].get("Voltage - Output (Min/Fixed)", ""))
            else:
                vout = [number(r["vout_v"])]
            if len(vout) != len(regs) or any(v is None for v in vout):
                sys.exit(f"gen-parts: {r['mpn']}: {len(regs)} regulators but the output voltages are {vout}")
            keys["vout"] = vout[0] if len(regs) == 1 else vout
        keys["vinMin"] = number(s["vin_min"]) or number(r["vin_min_v"])
        keys["vinMax"] = number(r["vin_max_v"])
        if rtype == "linear":
            keys["dropout"] = number(r["dropout_v"]) or number(s["dropout"])
        keys["iout"] = number(r["iout_a"])
        keys["iq"] = number(r["iq_a"])
        if rtype != "linear":
            keys["fsw"] = number(r["fsw_hz"])
        for k, v in keys.items():
            if v is not None:
                part[k] = v
        check_limits(r, part)
        d["parts"].append(part)
    out = list(devices.values())
    for d in out:
        d["parts"].sort(key=lambda p: p["name"])
    out.sort(key=lambda d: (d["drawer"], d["function"], d["id"]))
    ids = [d["id"] for d in out]
    assert len(ids) == len(set(ids)), "duplicate device ids"
    write(out + generic_devices())
    added = add_datasheets(sheets)
    merged = merge_footprints(used_fp)
    print(f"gen-parts: {OUT}: {len(out) + 3} devices, {len(rows) + 3} parts; {len(sheets)} datasheets used, "
          f"{added} added to datasheets.json; {merged} footprints appended to footprints.json")


def main():
    args = [a for a in sys.argv[1:] if a != "--generics-only"]
    full(args[0] if args else os.path.join(common.DATA, "candidates.tsv"), "--generics-only" in sys.argv)


if __name__ == "__main__":
    main()
