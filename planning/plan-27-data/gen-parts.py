"""plan-27-E section 4.1: tactile switch devices and parts for Parts/switch.json.

usage: python3 -I gen-parts.py [--maker M]... [--list-footprints] [--dry-run]
       python3 -I gen-parts.py --selftest

The generator owns the text of switch.json from the first device whose id
starts `tact_` (or the end of the devices array when there is none) to the end
of the array; the text before it is never touched.  The region is rebuilt from
candidates.tsv for the named makers plus every maker whose parts the region
already holds, so a run for the next maker keeps the earlier ones, and a second
run with the same arguments changes nothing.

Device = (circuit, terminals, volts, actuator):
  id          tact_<no|nc>_<T>t_<V>v_<actuator slug>
  function    Normal Open / Normal Close; drawer ["Switch","Tactile"]
  pinouts     one per distinct specs.tsv `pins` string, p<T>_<A..>_<B..>
  contacts    [["A","B"]] or [["A1","B1"]]; joined [["A1","A2"],["B1","B2"]] (T = 4)
  momentary true, normallyClosed true (NC), ratedVolts V (DigiKey rating)
Part = {"name", "manufacturer", "datasheet", "package" (from mounting), "pinout",
"footprint" (specs.tsv)}.  Every datasheet the parts name gets an entry in
Parts/datasheets.json (id = the placed PDF's name, lower case, no .pdf; url =
the maker's URL from maker-urls.tsv, else the DigiKey URL); existing entries
are kept and new ones inserted in id order.
Refusals (exit 1): a part name already in another Parts/*.json or outside the
region; an add row with zero or two specs.tsv matches; an unknown footprint id;
an unreadable rating.
"""
import argparse
import csv
import difflib
import glob
import json
import os
import re
import shutil
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))

# plan-27-E section 4.1: DigiKey maker name prefix -> the part's "manufacturer".
MAKERS = [("C&K", "Littelfuse"), ("E-Switch", "E-Switch"), ("Aratas", "Aratas"),
          ("Panasonic", "Panasonic"), ("Alps", "Alps Alpine"), ("Same Sky", "Same Sky"),
          ("TE ", "TE Connectivity"), ("APEM", "APEM"), ("NKK", "NKK Switches"),
          ("Würth", "Würth Elektronik"), ("Test Maker", "Test Maker")]

PACKAGES = {
    "Surface Mount": "SMD",
    "Through Hole": "THT",
    "Through Hole, Right Angle": "THT right angle",
    "Surface Mount, Right Angle": "SMD right angle",
    "Edge Mount, Right Angle": "Edge mount",
}


class Refuse(Exception):
    pass


def paths(root):
    parts = os.path.join(root, "schematical", "schematical", "Parts")
    data = os.path.join(root, "planning", "plan-27-data")
    return {
        "switch": os.path.join(parts, "switch.json"),
        "parts": parts,
        "footprints": os.path.join(parts, "footprints.json"),
        "data": data,
        "candidates": os.path.join(data, "candidates.tsv"),
        "specs": os.path.join(data, "specs.tsv"),
        "places": os.path.join(root, "datasheets", "switch", "tactile", "datasheets.tsv"),
        "datasheets": os.path.join(parts, "datasheets.json"),
        "maker_urls": os.path.join(data, "maker-urls.tsv"),
    }


def read_tsv(path):
    with open(path) as f:
        return list(csv.DictReader(f, delimiter="\t"))


def footprint_ids(p):
    ids = set()
    for f in sorted(glob.glob(os.path.join(p["data"], "footprints-*.json"))) + [p["footprints"]]:
        if os.path.exists(f):
            ids.update(o["id"] for o in json.load(open(f))["footprints"])
    return ids


def volts_of(rating, mpn):
    """The volts of DigiKey's "Contact Rating @ Voltage": `0.05A @ 12VDC`,
    `25mA @ 16VDC`, `1VA @ 32VAC/DC` -> 12, 16, 32."""
    m = re.search(r"([\d.]+)\s*(mA|A|VA)\s*@\s*([\d.]+)\s*V", rating or "")
    if not m:
        raise Refuse(f"{mpn}: rating {rating!r} has no <amps>A|<va>VA @ <volts>V")
    v = float(m.group(3))
    return (int(v) if v == int(v) else v), m.group(0)


def num(v):
    return str(int(v)) if v == int(v) else repr(v)


def slug(text):
    return re.sub(r"-+", "-", re.sub(r"[^a-z0-9]+", "-", text.lower())).strip("-")


def region_bounds(text):
    """(start, end) of the generated region: start = the line of the first tact_
    device's '{' (or the position after the last DIP device), end = the
    position of the newline before the closing '  ]' of the devices array."""
    end = text.rindex("\n  ]")
    m = re.search(r'\n    \{\n      "id": "tact_', text)
    if m:
        return m.start(), end
    return end, end


def existing_names(p, region_text):
    """Part names of every other Parts/*.json and of switch.json outside the region."""
    names = {}
    for f in sorted(glob.glob(os.path.join(p["parts"], "*.json"))):
        base = os.path.basename(f)
        if base in ("footprints.json",):
            continue
        text = open(f).read()
        if os.path.abspath(f) == os.path.abspath(p["switch"]):
            text = text.replace(region_text, "")
        for n in re.findall(r'\{\s*"name"\s*:\s*"([^"]+)"', text):
            names[n] = base
    return names


def build(p, makers):
    text = open(p["switch"]).read()
    start, end = region_bounds(text)
    region = text[start:end]
    rows = read_tsv(p["candidates"])
    specs = read_tsv(p["specs"])
    in_region = set(re.findall(r'\{"name":"([^"]+)"', region))
    keep = set(makers)
    for r in rows:
        if r["mpn"] in in_region:
            keep.add(r["maker"])
    chosen = [r for r in rows if r["decision"] == "add" and any(r["maker"].startswith(m) or r["maker"] == m for m in keep)]
    places = {}
    if os.path.exists(p["places"]):
        for d in read_tsv(p["places"]):
            places[d["url"]] = d["path"]
    taken = existing_names(p, region)
    fps = footprint_ids(p)
    devices = {}
    seen = set()
    sheets, existing_sheets = sheet_ids(p, {r["datasheet_url"] for r in chosen})
    for r in sorted(chosen, key=lambda r: r["mpn"]):
        mpn = r["mpn"]
        if mpn in taken:
            raise Refuse(f"{mpn}: part name already in {taken[mpn]}")
        if mpn in seen:
            raise Refuse(f"{mpn}: two add rows")
        seen.add(mpn)
        match = [s for s in specs if s["maker"] == r["maker"] and s["series"] == r["series"]
                 and s["mounting"] == r["mounting"] and re.fullmatch(s["mpn_regex"], mpn)]
        if len(match) != 1:
            raise Refuse(f"{mpn} ({r['maker']} / {r['series']} / {r['mounting']}): {len(match)} specs.tsv rows match")
        spec = match[0]
        if spec["footprint"] not in fps:
            raise Refuse(f"{mpn}: footprint {spec['footprint']!r} is in no footprints file")
        if r["circuit"] not in ("SPST-NO", "SPST-NC"):
            raise Refuse(f"{mpn}: circuit {r['circuit']!r}")
        if r["mounting"] not in PACKAGES:
            raise Refuse(f"{mpn}: mounting {r['mounting']!r}")
        nc = r["circuit"] == "SPST-NC"
        t = int(spec["terminals"])
        v, amps = volts_of(r["rating"], mpn)
        act = r["actuator"].strip()
        if not act or act == "-":
            raise Refuse(f"{mpn}: no actuator")
        did = f"tact_{'nc' if nc else 'no'}_{t}t_{num(v)}v_{slug(act)}"
        dev = devices.setdefault(did, {"nc": nc, "t": t, "v": v, "act": act, "pinouts": {},
                                       "ratings": set(), "sheets": set(), "parts": []})
        ma = re.fullmatch(r"A=([\d,]+);B=([\d,]+)", spec["pins"])
        a, b = ma.group(1).split(","), ma.group(2).split(",")
        pname = f"p{t}_{'_'.join(a)}_{'_'.join(b)}"
        if pname not in dev["pinouts"]:
            names = {}
            if t == 2:
                names[int(a[0])], names[int(b[0])] = "A", "B"
            else:
                for i, n in enumerate(a):
                    names[int(n)] = f"A{i + 1}"
                for i, n in enumerate(b):
                    names[int(n)] = f"B{i + 1}"
            dev["pinouts"][pname] = [(n, names[n]) for n in sorted(names)]
        dev["ratings"].add(r["rating"].strip())
        sheet = places.get(r["datasheet_url"])
        if sheet:
            dev["sheets"].add(os.path.relpath(sheet, os.path.join("datasheets", "switch", "tactile")))
        dev["parts"].append({"name": mpn, "manufacturer": manufacturer(r["maker"]),
                             "datasheet": sheets[r["datasheet_url"]][0],
                             "package": PACKAGES[r["mounting"]], "pinout": pname,
                             "footprint": spec["footprint"]})
    new_sheets = {i: u for i, u in sheets.values() if i not in existing_sheets}
    return text, start, end, devices, new_sheets


def manufacturer(maker):
    for prefix, name in MAKERS:
        if maker.startswith(prefix):
            return name
    raise Refuse(f"no manufacturer name for maker {maker!r}")


def full_url(url):
    """A datasheets.json URL: `https://`, as the loader requires (DigiKey gives
    `//mm.digikey.com/…` and some `http://`), spaces escaped."""
    if url.startswith("//"):
        url = "https:" + url
    elif url.startswith("http://"):
        url = "https://" + url[len("http://"):]
    return url.replace(" ", "%20")


def sheet_ids(p, urls):
    """datasheet_url -> (id, url for datasheets.json) for the generated parts;
    also the existing datasheets.json ids -> url."""
    places = {d["url"]: d["path"] for d in read_tsv(p["places"])} if os.path.exists(p["places"]) else {}
    maker = {}
    if os.path.exists(p["maker_urls"]):
        maker = {r["dk_url"]: r["maker_url"] for r in read_tsv(p["maker_urls"]) if r["maker_url"]}
    existing = {}
    for d in json.load(open(p["datasheets"]))["datasheets"]:
        existing[d["id"]] = d["url"]
    # One entry per placed file; its URL: a maker URL found for any of the
    # file's DigiKey URLs, else the first (sorted) URL that names the file.
    by_path = {}
    for u in places:
        by_path.setdefault(places[u], []).append(u)
    for u in urls:
        if u not in places:
            raise Refuse(f"datasheet {u} is not placed (datasheets.tsv)")
    entry = {}
    for path in {places[u] for u in urls}:
        group = sorted(by_path[path])
        url = full_url(next((maker[g] for g in group if g in maker), group[0]))
        entry[path] = (os.path.basename(path)[:-4].lower(), path.split("/")[-2], url)
    ids = {}
    for path, (name, folder, url) in entry.items():
        clash = any(v[0] == name for k, v in entry.items() if k != path)
        clash = clash or (name in existing and existing[name] != url)
        did = f"{folder}-{name}" if clash else name
        if did in existing and existing[did] != url:
            raise Refuse(f"datasheet id {did} is in datasheets.json with another URL")
        ids[path] = (did, url)
    return {u: ids[places[u]] for u in urls}, existing


def sheets_text(p, new_sheets):
    """(old, new) text of datasheets.json, the new entries inserted in id order."""
    text = open(p["datasheets"]).read()
    if not new_sheets:
        return text, text
    lines = text.split("\n")
    at = [i for i, l in enumerate(lines) if l.startswith('    {"id"')]
    first, last = at[0], at[-1]
    entries = [l.rstrip(",") for l in lines[first:last + 1]]
    for i, u in new_sheets.items():
        entries.append("    " + json.dumps({"id": i, "url": u}, ensure_ascii=False))
    entries.sort(key=lambda l: re.search(r'"id": "([^"]*)"', l).group(1))
    return text, "\n".join(lines[:first] + [",\n".join(entries)] + lines[last + 1:])


def q(s):
    return json.dumps(s, ensure_ascii=False)


def render(devices):
    out = []
    for did in sorted(devices):
        d = devices[did]
        kind = "closed" if d["nc"] else "open"
        lines = [
            "    {",
            f'      "id": {q(did)},',
            f'      "function": {q("Normal Close" if d["nc"] else "Normal Open")},',
            f'      "description": {q(f"Tactile switch, SPST normally {kind}, {d["t"]} terminals, {num(d["v"])} V, {d["act"]} actuator")},',
            '      "drawer": ["Switch","Tactile"],',
            f'      "note": {q("Rating from DigiKey " + chr(34) + "Contact Rating @ Voltage" + chr(34) + " (" + ", ".join(sorted(d["ratings"])) + "; the switching rating); datasheets: " + ", ".join(sorted(d["sheets"])))},',
            '      "pinouts": {',
        ]
        pins = sorted(d["pinouts"])
        for i, pn in enumerate(pins):
            lines.append(f"        {q(pn)}: [")
            pl = d["pinouts"][pn]
            for j, (number, name) in enumerate(pl):
                comma = "," if j < len(pl) - 1 else ""
                lines.append(f'          {{"number":{number},"name":{q(name)},"role":"inout"}}{comma}')
            lines.append("        ]" + ("," if i < len(pins) - 1 else ""))
        lines.append("      },")
        lines.append(f'      "actuator": {q(d["act"])},')
        lines.append('      "contacts": [')
        lines.append('        ["A","B"]' if d["t"] == 2 else '        ["A1","B1"]')
        lines.append("      ],")
        if d["t"] == 4:
            lines.append('      "joined": [')
            lines.append('        ["A1","A2"],')
            lines.append('        ["B1","B2"]')
            lines.append("      ],")
        lines.append('      "momentary": true,')
        if d["nc"]:
            lines.append('      "normallyClosed": true,')
        lines.append(f'      "ratedVolts": {num(d["v"])},')
        lines.append('      "parts": [')
        parts = sorted(d["parts"], key=lambda x: x["name"])
        for j, part in enumerate(parts):
            comma = "," if j < len(parts) - 1 else ""
            lines.append("        " + json.dumps(part, ensure_ascii=False, separators=(",", ":")) + comma)
        lines.append("      ]")
        lines.append("    }")
        out.append("\n".join(lines))
    return out


def generate(p, makers):
    text, start, end, devices, new_sheets = build(p, makers)
    blocks = render(devices)
    head = text[:start]
    if blocks:
        if not head.rstrip().endswith(",") and not head.rstrip().endswith("["):
            head = head.rstrip() + ","
        new = head + "\n" + ",\n".join(blocks) + text[end:]
    else:
        new = head.rstrip().rstrip(",") + text[end:]
    return text, new, devices, new_sheets


def run(p, makers, dry):
    old, new, devices, new_sheets = generate(p, makers)
    old_ds, new_ds = sheets_text(p, new_sheets)
    if old == new and old_ds == new_ds:
        print("no changes")
        return 0
    if dry:
        sys.stdout.writelines(difflib.unified_diff(old.splitlines(True), new.splitlines(True),
                                                   "switch.json", "switch.json (new)", n=1))
        sys.stdout.writelines(difflib.unified_diff(old_ds.splitlines(True), new_ds.splitlines(True),
                                                   "datasheets.json", "datasheets.json (new)", n=0))
        return 0
    open(p["switch"], "w").write(new)
    open(p["datasheets"], "w").write(new_ds)
    n = sum(len(d["parts"]) for d in devices.values())
    print(f"{len(devices)} tactile devices, {n} tactile parts written; {len(new_sheets)} datasheets.json entries added")
    return 0


def selftest():
    tmp = tempfile.mkdtemp(prefix="p27-gen-")
    p = paths(tmp)
    os.makedirs(p["parts"])
    os.makedirs(p["data"])
    os.makedirs(os.path.dirname(p["places"]))
    real = paths(ROOT)
    shutil.copyfile(real["switch"], p["switch"])
    shutil.copyfile(real["datasheets"], p["datasheets"])
    with open(p["places"], "w") as f:
        f.write("path\turl\tbytes\tsha256\n"
                "datasheets/switch/tactile/test/SELFTEST-SHEET.pdf\thttps://example.com/selftest.pdf\t1\t0\n")
    open(p["footprints"], "w").write(json.dumps({"format": "schematical-footprints", "version": 1, "footprints": [
        {"id": "T2", "description": "t", "style": "dual", "pins": 2, "perSide": 1, "body": {"width": 1, "height": 1},
         "pitch": 1, "span": 2, "pad": {"width": 0.5, "height": 0.5, "shape": "rect"}},
        {"id": "T4", "description": "t", "style": "dual", "pins": 4, "body": {"width": 1, "height": 1},
         "pitch": 1, "span": 2, "pad": {"width": 0.5, "height": 0.5, "shape": "rect"}}]}))
    cols = ["maker", "mpn", "status", "qty", "series", "circuit", "function", "mounting", "actuator", "orientation",
            "termination", "outline", "rating", "illumination", "ingress", "height", "datasheet_url", "decision",
            "reason", "pdf", "hit", "dead"]

    def row(mpn, circuit, mounting):
        r = dict.fromkeys(cols, "")
        r.update(maker="Test Maker", mpn=mpn, series="S1", circuit=circuit, mounting=mounting,
                 actuator="Plunger for Cap", rating="0.05A @ 12VDC", decision="add", pdf="1", hit="1", dead="0",
                 datasheet_url="https://example.com/selftest.pdf")
        return "\t".join(r[c] for c in cols)
    with open(p["candidates"], "w") as f:
        f.write("\t".join(cols) + "\n" + row("SELFTEST-NO2", "SPST-NO", "Surface Mount") + "\n"
                + row("SELFTEST-NC4", "SPST-NC", "Through Hole") + "\n")
    with open(p["specs"], "w") as f:
        f.write("maker\tseries\tmounting\tmpn_regex\tterminals\tpins\tfootprint\tnote\n"
                "Test Maker\tS1\tSurface Mount\t.*\t2\tA=1;B=2\tT2\t\n"
                "Test Maker\tS1\tThrough Hole\t.*\t4\tA=1,2;B=3,4\tT4\t\n")
    original = open(p["switch"]).read()
    original_ds = open(p["datasheets"]).read()
    prefix_len, _ = region_bounds(original)
    run(p, ["Test Maker"], False)
    once = open(p["switch"]).read()
    once_ds = open(p["datasheets"]).read()
    run(p, [], False)
    twice = open(p["switch"]).read()
    ok = True
    if once == twice and once != original and once_ds == open(p["datasheets"]).read():
        print("idempotent ok")
    else:
        print("FAIL: second run changed a file" if once != twice else "FAIL: first run wrote nothing")
        ok = False
    if once[:prefix_len] == original[:prefix_len] and '"id": "tact_nc_4t_12v_plunger-for-cap"' in once:
        print("prefix ok")
    else:
        print("FAIL: text before the region changed")
        ok = False
    json.loads(once)
    ids = [e["id"] for e in json.loads(once_ds)["datasheets"]]
    if (once_ds != original_ds and ids == sorted(ids) and "selftest-sheet" in ids
            and '"datasheet":"selftest-sheet"' in once and '"manufacturer":"Test Maker"' in once
            and len(ids) == len(json.loads(original_ds)["datasheets"]) + 1):
        print("datasheets ok")
    else:
        print("FAIL: datasheets.json entry or the part keys")
        ok = False
    shutil.rmtree(tmp)
    return 0 if ok else 1


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--maker", action="append", default=[])
    ap.add_argument("--list-footprints", action="store_true")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--selftest", action="store_true")
    a = ap.parse_args()
    try:
        if a.selftest:
            return selftest()
        p = paths(ROOT)
        if a.list_footprints:
            rows = {r["mpn"]: r["maker"] for r in read_tsv(p["candidates"])}
            _, _, _, devices, _ = build(p, a.maker)
            ids = set()
            for d in devices.values():
                for x in d["parts"]:
                    if not a.maker or any(rows[x["name"]].startswith(m) for m in a.maker):
                        ids.add(x["footprint"])
            for i in sorted(ids):
                print(i)
            return 0
        return run(p, a.maker, a.dry_run)
    except Refuse as e:
        print("refused:", e)
        return 1


if __name__ == "__main__":
    sys.exit(main())
