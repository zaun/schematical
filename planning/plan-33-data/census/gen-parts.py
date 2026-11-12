"""plan-33-F / -G: turn the add rows of candidates.tsv into Parts/capacitor.json parts.

usage: python3 -I gen-parts.py --generic [--dry-run]          GENERIC-CAPACITOR and its six footprints
       python3 -I gen-parts.py --type <t>[,<t>...] [--dry-run]  the add rows of those kind types
       python3 -I gen-parts.py --ids                          the footprint id map (provisional -> final)

Kind types: ceramic, aluminum, aluminum-polymer, tantalum, tantalum-polymer,
niobium-oxide (candidates.tsv's `type` column: ceramic, aluminum,
al-polymer, tantalum, ta-polymer, niobium).

Reads candidates.tsv (decision add), fpgroups.tsv (each row's footprint,
package and pinout, by its group key, footprint-lines.py), the ready
footprint objects (footprints-generic.json, footprints-C.json,
footprints-D.json) and datasheets/capacitor/datasheets.tsv, and merges the
scope's parts into Parts/capacitor.json, their datasheets into
Parts/datasheets.json and the footprints they use into Parts/footprints.json
(existing content kept; re-running a scope gives the same files).

Footprint ids (plan-33-C section 4.3, plan-29-I's rule).  Objects whose
geometry (every key but id, description, note) is identical are one
footprint; per BASE (the provisional id less its -KEMET / -CALCHIP / -AVX
tag) the first footprint gets the bare BASE unless that id is taken, the
others keep their id.  A shipped footprint with identical geometry is reused
only when its id starts with CAP (plan-33-C section 4.6: CAP-0603 keeps its
own id although RES-0603 has its numbers); the run prints `reused <id>`.

Devices (plan-33-F section 3.1): one per (type, capacitance), id
`<type>-<value>` (`ceramic-100nF`), function the value text (section 3.3).
"""
import argparse
import collections
import csv
import importlib.util
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
PARTS = os.path.join(ROOT, "schematical", "schematical", "Parts")

_spec = importlib.util.spec_from_file_location("fpl", os.path.join(HERE, "footprint-lines.py"))
fpl = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(fpl)

KIND_TYPE = {"ceramic": "ceramic", "aluminum": "aluminum", "al-polymer": "aluminum-polymer",
             "tantalum": "tantalum", "ta-polymer": "tantalum-polymer", "niobium": "niobium-oxide"}
TYPE_NAME = {"ceramic": "Ceramic", "aluminum": "Aluminum electrolytic", "aluminum-polymer": "Aluminum polymer",
             "tantalum": "Tantalum", "tantalum-polymer": "Tantalum polymer", "niobium-oxide": "Niobium oxide"}
DRAWER = {"ceramic": "Ceramic", "aluminum": "Electrolytic", "aluminum-polymer": "Electrolytic",
          "tantalum": "Tantalum", "tantalum-polymer": "Tantalum", "niobium-oxide": "Tantalum"}
MAKER = {"KEMET": "KEMET", "Cal-Chip Electronics": "Cal-Chip Electronics", "KYOCERA AVX": "KYOCERA AVX"}
GEOMETRY_SKIP = {"id", "description", "note"}
TAGS = ("-KEMET", "-CALCHIP", "-AVX")
PINOUTS = {"two-pin": [{"number": 1, "name": "1", "role": "input"}, {"number": 2, "name": "2", "role": "input"}],
           "polar": [{"number": 1, "name": "+", "role": "input"}, {"number": 2, "name": "-", "role": "input"}]}
GENERIC_CHOICES = [("0402", "CAP-0402"), ("0603", "CAP-0603"), ("0805", "CAP-0805"), ("1206", "CAP-1206"),
                   ("Disc", "CAP-DISC-5.08"), ("Radial", "CAP-RADIAL-2.54")]


# ---- value text (section 3.3) ----------------------------------------------------

def value_text(farads):
    for scale, unit in ((1e-6, "µF"), (1e-9, "nF"), (1e-12, "pF")):
        n = farads / scale
        if n >= 1 - 1e-9 or unit == "pF":
            digits = max(0, 2 - int(f"{n:e}".split("e")[1]))
            s = f"{n:.{digits}f}"
            if "." in s:
                s = s.rstrip("0").rstrip(".")
            return f"{s} {unit}"


def device_id(kind_type, farads):
    return f"{kind_type}-" + value_text(farads).replace(" ", "").replace("µ", "u")


# ---- footprints --------------------------------------------------------------------

def ready_objects():
    out = []
    for name in ("footprints-generic.json", "footprints-C.json", "footprints-D.json"):
        p = os.path.join(DATA, name)
        if os.path.exists(p):
            out += json.load(open(p))["footprints"]
    return out


def geometry(o):
    return json.dumps({k: v for k, v in o.items() if k not in GEOMETRY_SKIP}, sort_keys=True)


def base_of(i):
    for t in TAGS:
        if i.endswith(t):
            return i[: -len(t)]
    return i


def id_map(existing):
    """provisional id -> (final id, object); object None = already shipped."""
    shipped = {}
    for i, o in existing.items():
        if i.startswith("CAP"):
            shipped.setdefault(geometry(o), i)
    by_geom, first, out = {}, {}, {}
    for o in ready_objects():
        if o["id"] in existing:
            out[o["id"]] = (o["id"], None)
            continue
        g = geometry(o)
        if g in shipped:
            out[o["id"]] = (shipped[g], None)
            continue
        by_geom.setdefault(g, o)
        first[o["id"]] = by_geom[g]
    final, taken, used_base = {}, set(existing), set()
    for o in by_geom.values():
        b = base_of(o["id"])
        if b not in used_base and b not in taken:
            final[o["id"]] = b
            used_base.add(b)
        else:
            final[o["id"]] = o["id"]
        taken.add(final[o["id"]])
    for pid, canonical in first.items():
        out[pid] = (final[canonical["id"]], canonical)
    return out


# ---- files -------------------------------------------------------------------------

def load_json(name):
    return json.load(open(os.path.join(PARTS, name)))


def write_parts(doc):
    with open(os.path.join(PARTS, "capacitor.json"), "w") as f:
        json.dump(doc, f, indent=1, ensure_ascii=False)
        f.write("\n")


def write_datasheets(ds_by_id):
    items = sorted(ds_by_id.values(), key=lambda d: d["id"])
    with open(os.path.join(PARTS, "datasheets.json"), "w") as f:
        f.write('{\n  "format": "schematical-datasheets",\n  "version": 1,\n  "datasheets": [\n')
        f.write(",\n".join("    " + json.dumps({"id": d["id"], "url": d["url"], **({"md": d["md"]} if "md" in d else {})},
                                              ensure_ascii=False) for d in items))
        f.write("\n  ]\n}\n")


def append_footprints(new_fp):
    if not new_fp:
        return
    text = open(os.path.join(PARTS, "footprints.json")).read()
    end = text.rstrip().rfind("]")
    head = text[:end].rstrip()
    lines = ",\n".join("    " + json.dumps(new_fp[k], ensure_ascii=False, separators=(",", ":")) for k in sorted(new_fp))
    with open(os.path.join(PARTS, "footprints.json"), "w") as f:
        f.write(head + ",\n" + lines + "\n  ]\n}\n")


# ---- parts -------------------------------------------------------------------------

def volts(text):
    m = re.match(r"\s*([\d.]+)\s*V", text or "")
    if not m:
        return None
    v = float(m.group(1))
    return int(v) if v == int(v) else v


def device_for(devices, kind_type, farads):
    did = device_id(kind_type, farads)
    dev = devices.get(did)
    if dev is None:
        dev = {"id": did, "function": value_text(farads),
               "description": f"{TYPE_NAME[kind_type]} capacitor, {value_text(farads)}",
               "drawer": ["Capacitors", DRAWER[kind_type]],
               "pinouts": {"two-pin" if kind_type == "ceramic" else "polar":
                           PINOUTS["two-pin" if kind_type == "ceramic" else "polar"]},
               "type": kind_type, "farads": float(f"{farads:.4g}")}
        if kind_type != "ceramic":
            dev["positive"], dev["negative"] = "+", "-"
        dev["parts"] = []
        devices[did] = dev
    return dev


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--type", default="")
    ap.add_argument("--generic", action="store_true")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--ids", action="store_true")
    a = ap.parse_args()

    fps_doc = load_json("footprints.json")
    existing_fp = {o["id"]: o for o in fps_doc["footprints"]}
    ids = id_map(existing_fp)
    if a.ids:
        for pid in sorted(ids):
            print(pid, "->", ids[pid][0])
        return 0

    doc = load_json("capacitor.json")
    devices = {d["id"]: d for d in doc["devices"]}
    ds_doc = load_json("datasheets.json")
    ds_by_id = {d["id"]: d for d in ds_doc["datasheets"]}
    ds_by_url = {d["url"]: d["id"] for d in ds_doc["datasheets"]}
    new_fp, reused = {}, set()

    def use_fp(pid):
        final, obj = ids[pid]
        if obj is None:
            if final not in reused:
                reused.add(final)
                print("reused", final)
        elif final not in existing_fp:
            o = dict(obj)
            o["id"] = final
            new_fp[final] = o
        return final

    parts = 0
    if a.generic:
        dev = devices.get("ceramic-generic") or {
            "id": "ceramic-generic", "function": "Generic", "description": "Generic capacitor",
            "drawer": ["Capacitors"], "pinouts": {"two-pin": PINOUTS["two-pin"]}, "type": "ceramic",
            "farads": 1e-7, "parts": []}
        devices["ceramic-generic"] = dev
        choices = [{"package": pkg, "footprint": use_fp(fid)} for pkg, fid in GENERIC_CHOICES]
        part = {"name": "GENERIC-CAPACITOR", "manufacturer": "Generic", "package": "Disc", "pinout": "two-pin",
                "footprint": use_fp("CAP-DISC-5.08"), "footprintChoices": choices}
        dev["parts"] = [p for p in dev["parts"] if p["name"] != part["name"]] + [part]
        parts += 1

    if a.type:
        want = set(a.type.split(","))
        bad = want - set(KIND_TYPE.values())
        if bad:
            raise SystemExit(f"unknown type {sorted(bad)}")
        paths = {p["url"]: p["path"] for p in csv.DictReader(
            open(os.path.join(ROOT, "datasheets", "capacitor", "datasheets.tsv")), delimiter="\t")}
        first_url = {}
        for url in sorted(paths):
            first_url.setdefault(paths[url], url)
        groups = {}
        for g in csv.DictReader(open(os.path.join(DATA, "fpgroups.tsv"), newline="", encoding="utf-8"),
                                delimiter="\t"):
            groups[(g["maker"], g["type"], g["package"], g["size"], g["spacing"], g["size_code"], g["pdf"])] = g
        rows = [r for r in csv.DictReader(open(os.path.join(DATA, "candidates.tsv"), newline="", encoding="utf-8"),
                                          delimiter="\t")
                if r["decision"] == "add" and KIND_TYPE[r["type"]] in want]
        for r in sorted(rows, key=lambda r: r["mpn"]):
            path = paths[r["datasheet_url"]]
            g = groups[(fpl.SHORT[r["maker"]], r["type"], r["package"], r["size"], r["spacing"], fpl.maker_case(r),
                        path)]
            kt = KIND_TYPE[r["type"]]
            dev = device_for(devices, kt, float(r["farads"]))
            url = first_url[path]
            did = ds_by_url.get(url)
            if did is None:
                did = os.path.basename(path)[:-4].lower()
                if did in ds_by_id:
                    did = f"{os.path.basename(os.path.dirname(path))}-{did}"
                ds_by_id[did] = {"id": did, "url": url}
                ds_by_url[url] = did
            part = {"name": r["mpn"], "manufacturer": MAKER[r["maker"]], "datasheet": did,
                    "package": g["part_package"], "pinout": g["pinout"], "footprint": use_fp(g["footprint"])}
            v = volts(r["voltage"])
            if v is not None:
                part["volts"] = v
            if kt == "ceramic" and r["dielectric"]:
                part["dielectric"] = r["dielectric"].split(",")[0].strip()
            if r["tolerance"] and r["tolerance"] != "-":
                part["tolerance"] = r["tolerance"]
            if r["flags"]:
                part["flags"] = r["flags"].split(",")
            dev["parts"] = [p for p in dev["parts"] if p["name"] != r["mpn"]] + [part]
            parts += 1

    print(f"{parts} parts written; {len(devices)} devices, {len(new_fp)} new footprints")
    if a.dry_run:
        return 0
    for d in devices.values():
        d["parts"].sort(key=lambda p: p["name"])
    doc["devices"] = [devices[k] for k in sorted(devices)]
    write_parts(doc)
    write_datasheets(ds_by_id)
    append_footprints(new_fp)
    return 0


if __name__ == "__main__":
    sys.exit(main())
