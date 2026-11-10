"""plan-29-I § 3: turn the add rows of candidates.tsv into Parts/diode.json parts.

usage: python3 -I gen-parts.py --maker <prefix> --category <c>[,<c>...] [--dry-run]
       python3 -I gen-parts.py --ids                print the footprint id map (provisional -> final)

Reads candidates.tsv (decision add), specs.tsv (package, pinout, pins,
footprint per (maker, category, datasheet, supplier package)),
datasheets/diode/datasheets.tsv, the footprints-[B-G].json ready objects
and work/values.tsv (ratings read from the datasheets, vrio-check.py), and
merges the scope's parts into Parts/diode.json, their datasheets into
Parts/datasheets.json and the footprints they use into Parts/footprints.json
(existing content kept; re-running a scope gives the same files).

Footprint ids (plan-29-B § 4.4).  The owners wrote provisional ids
`<BASE>-<TAG>` (TAG VISHAY / ONSEMI / TSC).  Objects whose geometry (every key
but id, description, note) is identical are one footprint.  Then, per BASE
in letter order (B C D E F G files), the first footprint gets the bare BASE
(unless Parts/footprints.json already has that id), the others keep their
tag.  Ids that already exist in Parts/footprints.json are reused as they are,
and so is a shipped footprint whose geometry a ready object has (a later
scope reuses what an earlier one shipped); the run prints `reused <id>` for
each shipped footprint the scope's parts name.

Devices (§ 3.1): one per (type, diode pairs); a second pin naming of the
same shape gets the id `<type>-<shape>-b` (then -c, …).  The pairs come from the pin
names: tokens A<i> / K<i> (anode / cathode of diode i), A / K (common to
every diode), so `K1A2` is diode 1's cathode and diode 2's anode; bridges
AC1 AC2 + -.  `NC` pins are role nc, the rest inout.

work/part-overrides.tsv (mpn footprint package pinout pins note) sets a
part's own footprint / package / pin map where its spec row's does not fit
it (a PDF that covers two pin orders or two outlines).
"""
import argparse
import collections
import csv
import glob
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
PARTS = os.path.join(ROOT, "schematical", "schematical", "Parts")
import importlib.util  # noqa: E402

_spec = importlib.util.spec_from_file_location("vrio", os.path.join(HERE, "vrio-check.py"))
vrio = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(vrio)

MAKER = {"Vishay General Semiconductor - Diodes Division": "Vishay", "onsemi": "onsemi",
         "Taiwan Semiconductor Corporation": "Taiwan Semiconductor"}
TYPE = {"Rectifier": "rectifier", "Fast recovery": "fast", "Switching": "switching",
        "Schottky": "schottky", "Zener": "zener", "Bridge": "bridge"}
CONF = {"": "single", "1 Pair Common Cathode": "cc", "1 Pair Common Anode": "ca",
        "1 Pair Series Connection": "series", "2 Independent": "ind"}
GEOMETRY_SKIP = {"id", "description", "note"}
TAGS = ("-VISHAY", "-ONSEMI", "-TSC")


# ---- footprints -------------------------------------------------------------

def ready_objects():
    out = []
    for p in sorted(glob.glob(os.path.join(DATA, "footprints-[B-G].json"))):
        for o in json.load(open(p))["footprints"]:
            out.append(o)
    return out


def geometry(o):
    return json.dumps({k: v for k, v in o.items() if k not in GEOMETRY_SKIP}, sort_keys=True)


def base_of(i):
    for t in TAGS:
        if i.endswith(t):
            return i[: -len(t)]
    return i


def id_map(existing):
    """provisional id -> (final id, object); object None = already shipped.
    `existing` is Parts/footprints.json's objects by id: a ready object with
    the geometry of a shipped footprint maps to that footprint (so a later
    scope reuses what an earlier one shipped)."""
    existing_ids = set(existing)
    shipped = {}
    for i, o in existing.items():
        shipped.setdefault(geometry(o), i)
    objs = ready_objects()
    by_geom = {}
    first = {}
    out = {}
    for o in objs:
        if o["id"] in existing_ids:
            continue
        g = geometry(o)
        if g in shipped:
            out[o["id"]] = (shipped[g], None)
            continue
        if g not in by_geom:
            by_geom[g] = o
        first.setdefault(o["id"], by_geom[g])
    final = {}
    taken = set(existing_ids)
    used_base = set()
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
    for e in existing_ids:
        out[e] = (e, None)
    return out


# ---- pins and devices -------------------------------------------------------

def parse_map(text):
    return [(int(n), name) for n, name in (x.split(":") for x in text.split(","))]


def pick_map(spec, conf):
    maps = spec["pins"].split(";")
    names = spec["pinout"].split(";")
    if len(maps) == 1:
        return names[0], maps[0].split("=", 1)[-1]
    key = CONF.get(conf, conf)
    for name, m in zip(names, maps):
        k, v = m.split("=", 1)
        if k == key:
            return name, v
    raise SystemExit(f"{spec['datasheet']}: no map for configuration {conf!r}")


def pinout_name(pins):
    """assign.py's naming: two-pin for 1:K,2:A, else p<N>-<names>."""
    if pins == "1:K,2:A":
        return "two-pin"
    names = [p.split(":")[1].lower().replace("+", "pos").replace("-", "neg") for p in pins.split(",")]
    return f"p{len(names)}-" + "-".join(names)


def tokens(name):
    return re.findall(r"([AK])(\d*)", name)


def pairs_of(pin_names):
    names = sorted({n for n in pin_names if n != "NC"})
    if set(names) & {"AC1", "AC2", "+", "-"}:
        if set(names) != {"AC1", "AC2", "+", "-"}:
            raise ValueError(f"bridge pins {names}")
        return [("AC1", "+"), ("AC2", "+"), ("-", "AC1"), ("-", "AC2")]
    toks = {n: tokens(n) for n in names}
    for n, t in toks.items():
        if not t or "".join(a + b for a, b in t) != n:
            raise ValueError(f"pin name {n!r}")
    idx = sorted({int(i) for t in toks.values() for _, i in t if i}) or [1]
    pairs = []
    for i in idx:
        s = str(i)
        an = [n for n, t in toks.items() if ("A", s) in t or ("A", "") in t]
        ca = [n for n, t in toks.items() if ("K", s) in t or ("K", "") in t]
        if len(an) != 1 or len(ca) != 1:
            raise ValueError(f"diode {i}: anodes {an}, cathodes {ca} in {names}")
        pairs.append((an[0], ca[0]))
    return pairs


def shape(pairs):
    n = len(pairs)
    if n == 1:
        return "1"
    if [p for p in pairs] == [("AC1", "+"), ("AC2", "+"), ("-", "AC1"), ("-", "AC2")]:
        return "4"
    anodes = {a for a, _ in pairs}
    cathodes = {k for _, k in pairs}
    if len(cathodes) == 1:
        return f"cc{n}"
    if len(anodes) == 1:
        return f"ca{n}"
    if anodes & cathodes:
        return f"ser{n}"
    return f"ind{n}"


DESC = {"rectifier": "rectifier", "fast": "fast recovery", "switching": "switching",
        "schottky": "Schottky", "zener": "Zener", "bridge": "bridge"}


def description(t, sh):
    word = DESC[t]
    if t == "bridge":
        return "Bridge rectifier"
    if sh == "4":
        return f"Four {word} diodes in a bridge"
    if sh == "1":
        return f"{word[0].upper() + word[1:]} diode"
    n = sh[-1]
    kind = {"cc": "common-cathode", "ca": "common-anode", "ser": "series", "ind": "independent"}[sh[:-1]]
    count = {"2": "Dual", "3": "Triple", "4": "Quad"}.get(n, n + "x")
    return f"{count} {kind} {word} diodes"


# ---- main -------------------------------------------------------------------

def load_json(name):
    return json.load(open(os.path.join(PARTS, name)))


def ds_id(path, taken_by_url, existing_ids):
    base = os.path.basename(path)[:-4].lower() if path.lower().endswith(".pdf") else os.path.basename(path).lower()
    return base


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--maker", default="")
    ap.add_argument("--category", default="")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--ids", action="store_true")
    a = ap.parse_args()

    fps_doc = load_json("footprints.json")
    existing_fp = {o["id"]: o for o in fps_doc["footprints"]}
    ids = id_map(existing_fp)
    if a.ids:
        for pid in sorted(ids):
            if ids[pid][1] is not None:
                print(pid, "->", ids[pid][0])
        return 0

    cats = set(a.category.split(",")) if a.category else None
    placed = list(csv.DictReader(open(os.path.join(ROOT, "datasheets", "diode", "datasheets.tsv")), delimiter="\t"))
    path_of = {r["url"]: r["path"] for r in placed}
    specs = {(s["maker"], s["category"], s["datasheet"], s["supplier_package"]): s
             for s in csv.DictReader(open(os.path.join(DATA, "specs.tsv")), delimiter="\t")}
    over = vrio.overrides()
    po_path = os.path.join(DATA, "work", "part-overrides.tsv")
    part_over = ({o["mpn"]: o for o in csv.DictReader(open(po_path), delimiter="\t")}
                 if os.path.exists(po_path) else {})
    rows = [r for r in csv.DictReader(open(os.path.join(DATA, "candidates.tsv")), delimiter="\t")
            if r["decision"] == "add" and r["maker"].startswith(a.maker) and (cats is None or r["category"] in cats)]

    diode_doc = load_json("diode.json")
    devices = {d["id"]: d for d in diode_doc["devices"]}
    ds_doc = load_json("datasheets.json")
    ds_by_id = {d["id"]: d for d in ds_doc["datasheets"]}
    ds_by_url = {d["url"]: d["id"] for d in ds_doc["datasheets"]}

    no_spec = unknown_fp = 0
    reused = set()
    new_fp = {}
    new_ds = 0
    parts = 0
    for r in sorted(rows, key=lambda r: r["mpn"]):
        path = path_of[r["datasheet_url"]]
        s = specs.get((r["maker"], r["category"], path, r["supplier_package"]))
        if s is None or not s["footprint"] or not s["pins"]:
            no_spec += 1
            print("no spec:", r["mpn"], path, r["supplier_package"])
            continue
        po_ = part_over.get(r["mpn"])
        if po_:
            s = dict(s)
            for k in ("footprint", "package", "pinout", "pins"):
                if po_.get(k):
                    s[k] = po_[k]
            if po_.get("pins") and not po_.get("pinout"):
                s["pinout"] = pinout_name(po_["pins"])
        fid = s["footprint"]
        if fid not in ids:
            unknown_fp += 1
            print("unknown footprint:", r["mpn"], fid)
            continue
        final_fp, obj = ids[fid]
        if obj is None and final_fp not in reused:
            reused.add(final_fp)
            print("reused", final_fp)
        if obj is not None and final_fp not in existing_fp:
            o = dict(obj)
            o["id"] = final_fp
            new_fp[final_fp] = o
        pinout, pins_text = pick_map(s, r["configuration"])
        pins = parse_map(pins_text)
        pairs = pairs_of([n for _, n in pins])
        t = TYPE[r["leaf"]]
        sh = shape(pairs)
        want = sorted(pairs)
        dev_id = f"{t}-{sh}"
        k = 0
        while dev_id in devices and sorted((d["anode"], d["cathode"]) for d in devices[dev_id]["diodes"]) != want:
            k += 1
            dev_id = f"{t}-{sh}-{'bcdefghij'[k - 1]}"
        dev = devices.get(dev_id)
        if dev is None:
            dev = {"id": dev_id, "function": r["leaf"], "description": description(t, sh),
                   "drawer": ["Discrete", "Diodes"], "pinouts": {}, "type": t,
                   "diodes": [{"anode": x, "cathode": y} for x, y in pairs], "parts": []}
            devices[dev_id] = dev
        elif sorted((d["anode"], d["cathode"]) for d in dev["diodes"]) != sorted(pairs):
            raise SystemExit(f"{r['mpn']}: device {dev_id} has other diodes than {pairs}")
        po = [{"number": n, "name": name, "role": "nc" if name == "NC" else "inout"} for n, name in pins]
        if dev["pinouts"].setdefault(pinout, po) != po:
            raise SystemExit(f"{r['mpn']}: pinout {pinout} differs in device {dev_id}")
        url = r["datasheet_url"]
        did = ds_by_url.get(url)
        if did is None:
            did = os.path.basename(path)[:-4].lower()
            if did in ds_by_id:
                did = f"{os.path.basename(os.path.dirname(path))}-{did}"
            ds_by_id[did] = {"id": did, "url": url}
            ds_by_url[url] = did
            new_ds += 1
        vals, _ = vrio.values(dict(r, _path=path), over)
        part = {"name": r["mpn"], "manufacturer": MAKER[r["maker"]], "datasheet": did,
                "package": s["package"], "pinout": pinout, "footprint": final_fp}
        if r["flags"]:
            part["flags"] = r["flags"].split(",")
        for k in ("vr", "io", "vz", "pd"):
            if k in vals:
                part[k] = float(vrio.fmt(vals[k])) if "." in vrio.fmt(vals[k]) else int(vrio.fmt(vals[k]))
        dev["parts"] = [p for p in dev["parts"] if p["name"] != r["mpn"]] + [part]
        parts += 1

    print(f"{parts} parts, {no_spec} without spec, {unknown_fp} unknown footprint; "
          f"{len(devices)} devices, {len(new_fp)} new footprints, {new_ds} new datasheets")
    if a.dry_run or no_spec or unknown_fp:
        return 1 if (no_spec or unknown_fp) else 0

    for d in devices.values():
        d["parts"].sort(key=lambda p: p["name"])
        d["pinouts"] = dict(sorted(d["pinouts"].items()))
    diode_doc["devices"] = [devices[k] for k in sorted(devices)]
    with open(os.path.join(PARTS, "diode.json"), "w") as f:
        json.dump(diode_doc, f, indent=1, ensure_ascii=False)
        f.write("\n")
    ds_doc["datasheets"] = sorted(ds_by_id.values(), key=lambda d: d["id"])
    with open(os.path.join(PARTS, "datasheets.json"), "w") as f:
        f.write('{\n  "format": "schematical-datasheets",\n  "version": 1,\n  "datasheets": [\n')
        f.write(",\n".join("    " + json.dumps({"id": d["id"], "url": d["url"], **({"md": d["md"]} if "md" in d else {})},
                                              ensure_ascii=False) for d in ds_doc["datasheets"]))
        f.write("\n  ]\n}\n")
    if new_fp:
        text = open(os.path.join(PARTS, "footprints.json")).read()
        end = text.rstrip().rfind("]")
        head = text[:end].rstrip()
        lines = ",\n".join("    " + json.dumps(new_fp[k], ensure_ascii=False, separators=(",", ":"))
                           for k in sorted(new_fp))
        with open(os.path.join(PARTS, "footprints.json"), "w") as f:
            f.write(head + ",\n" + lines + "\n  ]\n}\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
