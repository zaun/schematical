#!/usr/bin/env python3
"""plan-40-G § 3: write Parts/transistor.json.

usage: python3 -I gen-parts.py --generics-only
       python3 -I gen-parts.py [<candidates.tsv>]   (default planning/plan-40-data/candidates.tsv)

--generics-only writes transistor.json with only the four generic devices
(§ 3.3). Without it, every `add` row of candidates.tsv becomes a part
(§ 3.1, § 3.2): the package label, pinout and elements from its specs.tsv
row, the footprint from footprint-map.tsv, the datasheet id from
datasheets/transistor/datasheets.tsv, the ratings from the row's DigiKey
facts. The generics are always the file's last four devices. The full run
also adds each used PDF's entry to Parts/datasheets.json (id only if
absent). Both runs take the generics' footprint choices from the `add`
rows (the footprint of the most-stocked add part of each label).
"""
import collections
import csv
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.append(HERE)
import common  # noqa: E402
import specs  # noqa: E402

PARTS = os.path.join(common.ROOT, "schematical", "schematical", "Parts")
OUT = os.path.join(PARTS, "transistor.json")
MAP = os.path.join(common.DATA, "footprint-map.tsv")
CHOICES = ["TO-92", "SOT-23", "SOT-223", "TO-220", "DPAK"]
NAMED_GENERICS = [("npn", "GENERIC-NPN", "NPN"), ("pnp", "GENERIC-PNP", "PNP"),
                  ("nmos", "GENERIC-NMOS", "N-channel MOSFET"), ("pmos", "GENERIC-PMOS", "P-channel MOSFET")]


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


def load_map():
    out = {}
    for r in csv.DictReader(open(MAP), delimiter="\t"):
        out[(r["maker"], r["package"], r["supplier"])] = r
    return out


def footprint_of(fmap, r):
    m = fmap.get((r["maker"], r["package"], r["supplier_package"])) or fmap.get((r["maker"], r["package"], "*"))
    if m is None or not m["footprint"]:
        sys.exit(f"gen-parts: {r['mpn']} has no footprint-map row ({r['maker']} | {r['package']} | "
                 f"{r['supplier_package']})")
    return m["footprint"]


def pinout_key(names, ball):
    if ball:
        return f"p{len(names)}-" + "-".join(f"{b.lower()}{n.lower()}" for b, n in ball)
    return f"p{len(names)}-" + "".join(n.lower() for n in names)


def parse_pinout(text):
    """(names in pin order, [(ball, name)] or None) of a specs.tsv pinout."""
    items = [i.strip() for i in text.split(",")]
    if all("=" in i for i in items):
        ball = [tuple(i.split("=", 1)) for i in items]
        return [n for _, n in ball], ball
    return items, None


def pins_json(names, ball, controls):
    out = []
    for k, name in enumerate(names):
        role = "nc" if name == "NC" else "input" if name in controls else "inout"
        pin = {"name": name, "role": role}
        if ball:
            pin = {"ball": ball[k][0], **pin}
        else:
            pin = {"number": k + 1, **pin}
        out.append(pin)
    return out


def elements_json(text, darlington):
    out = []
    for t, control, a, b in specs.elements(text):
        if t in ("npn", "pnp"):
            e = {"type": t, "base": control, "collector": a, "emitter": b}
            if darlington:
                e["darlington"] = True
        else:
            e = {"type": t, "gate": control, "drain": a, "source": b}
        out.append(e)
    return out


def function_of(r, els):
    """§ 3.1: the drawer leaf and the description."""
    fam = r["family"]
    if fam.endswith("array") or len(els) > 1:
        kinds = sorted({e[0] for e in els})
        mosfet = kinds[0] in ("nmos", "pmos")
        names = {"npn": "NPN", "pnp": "PNP", "nmos": "N-channel", "pmos": "P-channel"}
        what = " / ".join(names[k] for k in kinds)
        n = len(els)
        lead = {2: "Dual", 3: "Triple", 4: "Quad"}.get(n, f"{n}-element")
        pre = "pre-biased " if fam.startswith("bjt-prebiased") else ""
        desc = f"{lead} {what} {pre}{'MOSFET array' if mosfet else 'transistor array'}"
        return ("mosfet" if mosfet else "bjt"), "Array", desc
    t = els[0][0]
    if t in ("npn", "pnp"):
        up = t.upper()
        if "Darlington" in r["type"]:
            return "bjt", f"{up} Darlington", f"{up} Darlington transistor"
        if fam == "bjt-prebiased":
            return "bjt", f"{up} pre-biased", f"Pre-biased {up} transistor"
        return "bjt", up, f"{up} bipolar transistor"
    ch = "N-channel" if t == "nmos" else "P-channel"
    logic = r["drive_v"] not in ("", None) and float(r["drive_v"]) <= 4.5
    if logic:
        return "mosfet", f"{ch} logic-level", f"{ch} logic-level MOSFET"
    return "mosfet", ch, f"{ch} MOSFET"


def facts(r, group, logic):
    out = {}
    if group == "bjt":
        keys = [("vceo", "vmax_v"), ("ic", "imax_a"), ("pd", "power_w"), ("hfe", "hfe_min")]
        if r["family"].startswith("bjt-prebiased"):
            keys += [("r1", "r1_ohm"), ("r2", "r2_ohm")]
    else:
        keys = [("vds", "vmax_v"), ("id", "imax_a"), ("pd", "power_w"), ("rdson", "rdson_ohm"),
                ("vgsth", "vgsth_v"), ("vgs", "vgs_v")]
    for k, c in keys:
        v = number(r[c])
        if v is not None:
            out[k] = v
    if group == "mosfet" and logic:
        out["logicLevel"] = True
    return out


def add_rows(cand_path):
    return [r for r in csv.DictReader(open(cand_path), delimiter="\t") if r["reason"] == "add"]


def generic_choices(rows, table, fmap):
    """§ 3.3: per label of CHOICES, the footprint of the most-stocked add part."""
    best = {}
    for r in rows:
        s = table[specs.key(r)]
        lab = s["label"]
        if lab in CHOICES and (lab not in best or int(r["stock"]) > best[lab][0]):
            best[lab] = (int(r["stock"]), footprint_of(fmap, r))
    missing = [c for c in CHOICES if c not in best]
    if missing:
        sys.exit(f"gen-parts: no add part with the label {missing} for the generics' choices")
    return [{"package": c, "footprint": best[c][1]} for c in CHOICES]


def generic_devices(choices):
    out = []
    for t, name, what in NAMED_GENERICS:
        bjt = t in ("npn", "pnp")
        c, a, b = ("B", "C", "E") if bjt else ("G", "D", "S")
        el = ({"type": t, "base": c, "collector": a, "emitter": b} if bjt
              else {"type": t, "gate": c, "drain": a, "source": b})
        out.append({
            "id": f"generic-{t}",
            "function": "Generic",
            "description": f"Generic {what}" + (" transistor" if bjt else ""),
            "drawer": ["Discrete", "Transistors"],
            "pinouts": {"p3": [{"number": 1, "name": c, "role": "input"},
                               {"number": 2, "name": a, "role": "inout"},
                               {"number": 3, "name": b, "role": "inout"}]},
            "elements": [el],
            "parts": [{"name": name, "manufacturer": "Generic", "package": "TO-92", "pinout": "p3",
                       "footprint": choices[0]["footprint"], "footprintChoices": choices}],
        })
    return out


def full(cand_path, generics_only):
    table = specs.load()
    fmap = load_map()
    rows = add_rows(cand_path)
    choices = generic_choices(rows, table, fmap)
    if generics_only:
        write(generic_devices(choices))
        print(f"gen-parts: {OUT}: 4 devices, 4 parts (the generics); choices",
              ", ".join(f"{c['package']}={c['footprint']}" for c in choices))
        return
    placed = {r["path"]: r for r in csv.DictReader(open(common.PLACED), delimiter="\t")}
    devices = {}
    sheets = {}
    names = collections.Counter(r["mpn"] for r in rows)
    clash = [n for n, c in names.items() if c > 1]
    if clash:
        sys.exit(f"gen-parts: part names listed twice: {clash[:10]}")
    for r in rows:
        s = table[specs.key(r)]
        pin_names, ball = parse_pinout(s["pinout"])
        els = specs.elements(s["elements"])
        group, function, desc = function_of(r, els)
        pkey = pinout_key(pin_names, ball)
        dkey = (group, function, pkey)
        if dkey not in devices:
            controls = {e[1] for e in els}
            devices[dkey] = {
                "id": f"{group}-{kebab(function)}-{pkey}",
                "function": function,
                "description": desc,
                "drawer": ["Discrete", "Transistors", "BJT" if group == "bjt" else "MOSFET"],
                "pinouts": {pkey: pins_json(pin_names, ball, controls)},
                "elements": elements_json(s["elements"], "Darlington" in r["type"]),
                "parts": [],
            }
        elif "Darlington" in r["type"] and group == "bjt":
            pass  # a Darlington's function names it, so its device holds only Darlingtons
        sid = f"{common.SLUG[r['maker']]}-{kebab(os.path.splitext(os.path.basename(r['pdf']))[0])}"
        sheets[sid] = placed[r["pdf"]]["url"]
        part = {"name": r["mpn"], "manufacturer": r["maker"], "package": s["label"], "pinout": pkey,
                "footprint": footprint_of(fmap, r), "datasheet": sid}
        part.update(facts(r, group, function.endswith("logic-level")))
        devices[dkey]["parts"].append(part)
    out = []
    for key in sorted(devices):
        d = devices[key]
        d["parts"].sort(key=lambda p: p["name"])
        out.append(d)
    ids = [d["id"] for d in out]
    assert len(ids) == len(set(ids)), "duplicate device ids"
    write(out + generic_devices(choices))
    added = add_datasheets(sheets)
    print(f"gen-parts: {OUT}: {len(out) + 4} devices, {len(rows) + 4} parts; "
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
    doc = {"format": "schematical-parts", "version": 1, "kind": "transistor", "devices": devices}
    with open(OUT, "w") as f:
        json.dump(doc, f, ensure_ascii=False, indent=1)
        f.write("\n")


def main():
    args = [a for a in sys.argv[1:] if a != "--generics-only"]
    full(args[0] if args else os.path.join(common.DATA, "candidates.tsv"), "--generics-only" in sys.argv)


if __name__ == "__main__":
    main()
