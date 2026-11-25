"""plan-58-C section 4.4: slide switch devices and parts for Parts/switch.json.

usage: python3 -I gen-parts.py [--maker M]... [--dry-run]
       python3 -I gen-parts.py --selftest

The generator owns the contiguous run of devices whose id starts `slide_`, which
sits immediately before the first `tact_` device of switch.json (plan-27's
generator owns the text from that `tact_` device to the end and never touches it;
the hand-written DIP devices before the run stay).  The region is rebuilt from
candidates.tsv (rows `add`) and specs.tsv for the named makers plus every maker
whose parts the region already holds, so a run for the next maker keeps the
earlier ones, and a second run with the same arguments changes nothing.

Device = (circuit, action, rated volts, actuator):
  id          slide_<circuit>_<onon|onoffon|onoff>_<V>v_<actuator slug>
  function    "<circuit> <action>" (SPDT On-On); drawer ["Switches & Relays","Slide"]
  ratedVolts  DigiKey's DC voltage rating, else its AC rating, in volts
  contacts, states, rest   plan-58-B section 4.3 (by circuit and action)
  pinouts     one per distinct specs.tsv `pins` string, named p<N>_<the designators of the pin names
              in the order of plan-58-B section 4.3> (SPDT `2=C;1=A;3=B` -> p3_213), so adding another
              part's numbering never renames a pinout an earlier part uses
Part = {"name", "manufacturer", "datasheet", "package" (from mounting), "pinout",
"footprint" (specs.tsv)}.  Every datasheet the parts name gets an entry in
Parts/datasheets.json (id = the placed PDF's name, lower case, no .pdf; url =
the maker's URL from maker-urls.tsv, else the DigiKey URL); existing entries are
kept and new ones inserted in id order.  Every footprint the written parts name
moves from planning/plan-58-data/footprints-*.json into Parts/footprints.json
(appended, in id order, one line each), so a footprint and its first part land
together.
Refusals (exit 1): a part name already in another Parts/*.json or outside the
region; an add row with zero or two specs.tsv matches; a spec whose circuit is not
the row's; an unknown footprint id; an unreadable rating; a circuit / action with
no states.
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

# DigiKey maker name -> the part's "manufacturer".
MAKERS = [("C&K", "Littelfuse"), ("NKK", "NKK Switches"), ("CIT", "CIT Relay & Switch"),
          ("E-Switch", "E-Switch"), ("Test Maker", "Test Maker")]

PACKAGES = {
    "Through Hole": "THT",
    "Through Hole, Right Angle": "THT right angle",
    "Through Hole, Right Angle, Vertical": "THT right angle",
    "Surface Mount": "SMD",
    "Surface Mount, Right Angle": "SMD right angle",
}

ACTIONS = {"On-On": "onon", "On-Off-On": "onoffon", "On-Off": "onoff"}

# circuit -> (pin names in pinout order, contacts)
CIRCUITS = {
    "SPST": (["A", "B"], [["A", "B"]]),
    "SPDT": (["C", "A", "B"], [["C", "A"], ["C", "B"]]),
    "DPST": (["A1", "B1", "A2", "B2"], [["A1", "B1"], ["A2", "B2"]]),
    "DPDT": (["C1", "A1", "B1", "C2", "A2", "B2"],
             [["C1", "A1"], ["C1", "B1"], ["C2", "A2"], ["C2", "B2"]]),
}


class Refuse(Exception):
    pass


def states_of(circuit, action):
    """(states, rest) of plan-58-B section 4.3."""
    two = circuit in ("SPDT", "DPDT")
    if action == "On-Off" and not two:
        return ([[1], []] if circuit == "SPST" else [[1, 2], []]), 2
    if action == "On-On" and two:
        return ([[1], [2]] if circuit == "SPDT" else [[1, 3], [2, 4]]), 1
    if action == "On-Off-On" and two:
        return ([[1], [], [2]] if circuit == "SPDT" else [[1, 3], [], [2, 4]]), 2
    raise Refuse(f"{circuit} {action}: no states (plan-58-B section 4.3)")


def paths(root):
    parts = os.path.join(root, "schematical", "schematical", "Parts")
    data = os.path.join(root, "planning", "plan-58-data")
    return {
        "switch": os.path.join(parts, "switch.json"),
        "parts": parts,
        "footprints": os.path.join(parts, "footprints.json"),
        "data": data,
        "candidates": os.path.join(data, "candidates.tsv"),
        "specs": os.path.join(data, "specs.tsv"),
        "places": os.path.join(root, "datasheets", "switch", "slide", "datasheets.tsv"),
        "datasheets": os.path.join(parts, "datasheets.json"),
        "maker_urls": os.path.join(data, "maker-urls.tsv"),
    }


def read_tsv(path):
    with open(path) as f:
        return list(csv.DictReader(f, delimiter="\t"))


def num(v):
    return str(int(v)) if v == int(v) else repr(v)


def slug(text):
    return re.sub(r"-+", "-", re.sub(r"[^a-z0-9]+", "-", text.lower())).strip("-")


def volts_of(r):
    """(volts, "DC"|"AC"): DigiKey's DC voltage rating, else its AC rating."""
    for key, kind in (("vdc", "DC"), ("vac", "AC")):
        m = re.fullmatch(r"\s*([\d.]+)\s*V\s*", r.get(key, ""))
        if m:
            v = float(m.group(1))
            if v > 0:
                return (int(v) if v == int(v) else v), kind
    raise Refuse(f"{r['mpn']}: no DC and no AC voltage rating (vdc {r.get('vdc')!r}, vac {r.get('vac')!r})")


def manufacturer(maker):
    for prefix, name in MAKERS:
        if maker.startswith(prefix):
            return name
    raise Refuse(f"no manufacturer name for maker {maker!r}")


def staged_footprints(p):
    """footprint id -> (object, staged file) for every planning/plan-58-data/footprints-*.json."""
    out = {}
    for f in sorted(glob.glob(os.path.join(p["data"], "footprints-*.json"))):
        for o in json.load(open(f))["footprints"]:
            out[o["id"]] = (o, f)
    return out


def library_footprints(p):
    return {o["id"] for o in json.load(open(p["footprints"]))["footprints"]}


def region_bounds(text):
    """(start, end) of the generated region: start = the "\\n" before the first
    slide_ device's '{' (or, with none, the first tact_ device's), end = the
    "\\n" before the first tact_ device's '{'."""
    t = re.search(r'\n    \{\n      "id": "tact_', text)
    if not t:
        raise Refuse("switch.json has no tact_ device to place the slide region before")
    s = re.search(r'\n    \{\n      "id": "slide_', text)
    return (s.start() if s else t.start()), t.start()


def existing_names(p, region_text):
    """Part names of every other Parts/*.json and of switch.json outside the region."""
    names = {}
    for f in sorted(glob.glob(os.path.join(p["parts"], "*.json"))):
        base = os.path.basename(f)
        if base == "footprints.json":
            continue
        text = open(f).read()
        if os.path.abspath(f) == os.path.abspath(p["switch"]):
            text = text.replace(region_text, "")
        for n in re.findall(r'\{\s*"name"\s*:\s*"([^"]+)"', text):
            names[n] = base
    return names


def full_url(url):
    """A datasheets.json URL: `https://` (DigiKey gives `//mm.digikey.com/...` and some
    `http://`), spaces escaped."""
    if url.startswith("//"):
        url = "https:" + url
    elif url.startswith("http://"):
        url = "https://" + url[len("http://"):]
    return url.replace(" ", "%20")


def sheet_ids(p, urls):
    """datasheet url (of candidates.tsv) -> (id, url for datasheets.json); also the
    existing datasheets.json ids -> url."""
    places = {d["url"]: d["file"] for d in read_tsv(p["places"])} if os.path.exists(p["places"]) else {}
    maker = {}
    if os.path.exists(p["maker_urls"]):
        maker = {r["url"]: r["maker_url"] for r in read_tsv(p["maker_urls"]) if r["maker_url"]}
    existing = {d["id"]: d["url"] for d in json.load(open(p["datasheets"]))["datasheets"]}
    by_path = {}
    for u in places:
        by_path.setdefault(places[u], []).append(u)
    for u in urls:
        if u not in places:
            raise Refuse(f"datasheet {u} is not placed (datasheets.tsv)")
    entry = {}
    for path in {places[u] for u in urls}:
        group = sorted(u for u in by_path[path] if u in urls)
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
    chosen = [r for r in rows if r["decision"] == "add" and any(r["maker"].startswith(m) for m in keep)]
    taken = existing_names(p, region)
    staged = staged_footprints(p)
    have = library_footprints(p)
    sheets, existing_sheets = sheet_ids(p, {r["datasheet"] for r in chosen})
    devices, seen, used_fps = {}, set(), set()
    for r in sorted(chosen, key=lambda r: r["mpn"]):
        mpn = r["mpn"]
        if mpn in taken:
            raise Refuse(f"{mpn}: part name already in {taken[mpn]}")
        if mpn in seen:
            raise Refuse(f"{mpn}: two add rows")
        seen.add(mpn)
        match = [s for s in specs if s["maker"] == r["maker"] and s["series"] == r["series"]
                 and s["mounting"] == r["mounting"] and s["mpn_regex"] and re.fullmatch(s["mpn_regex"], mpn)]
        if len(match) != 1:
            raise Refuse(f"{mpn} ({r['maker']} / {r['series']} / {r['mounting']}): {len(match)} specs.tsv rows match")
        spec = match[0]
        if spec["circuit"] != r["circuit"]:
            raise Refuse(f"{mpn}: its spec is for {spec['circuit']}, the row is {r['circuit']}")
        if spec["footprint"] not in have and spec["footprint"] not in staged:
            raise Refuse(f"{mpn}: footprint {spec['footprint']!r} is in no footprints file")
        used_fps.add(spec["footprint"])
        if r["circuit"] not in CIRCUITS:
            raise Refuse(f"{mpn}: circuit {r['circuit']!r}")
        if r["function"] not in ACTIONS:
            raise Refuse(f"{mpn}: function {r['function']!r}")
        if r["mounting"] not in PACKAGES:
            raise Refuse(f"{mpn}: mounting {r['mounting']!r}")
        states_of(r["circuit"], r["function"])
        v, kind = volts_of(r)
        act = r["actuator"].strip()
        if not act or act == "-":
            raise Refuse(f"{mpn}: no actuator")
        did = f"slide_{r['circuit'].lower()}_{ACTIONS[r['function']]}_{num(v)}v_{slug(act)}"
        dev = devices.setdefault(did, {"circuit": r["circuit"], "action": r["function"], "v": v, "act": act,
                                       "kinds": set(), "pinouts": {}, "parts": []})
        dev["kinds"].add(kind)
        dev["pinouts"].setdefault(spec["pins"], None)
        dev["parts"].append({"name": mpn, "manufacturer": manufacturer(r["maker"]),
                             "datasheet": sheets[r["datasheet"]][0], "package": PACKAGES[r["mounting"]],
                             "pins": spec["pins"], "footprint": spec["footprint"]})
    for dev in devices.values():
        names, final = {}, {}
        for pins in sorted(dev["pinouts"]):
            order = CIRCUITS[dev["circuit"]][0]
            pl = {}
            for item in pins.split(";"):
                d, _, name = item.partition("=")
                pl[int(d)] = name
            by_name = {name: d for d, name in pl.items()}
            # p<N>_<designators in pin-name order>: stable when another part's numbering is added
            pname = f"p{len(order)}_" + "".join(str(by_name[n]) for n in order)
            names[pins] = pname
            final[pname] = (pname, [(d, pl[d]) for d in sorted(pl)])   # two spellings of one numbering share it
        dev["pinouts"] = final
        dev["parts"] = [{"name": x["name"], "manufacturer": x["manufacturer"], "datasheet": x["datasheet"],
                         "package": x["package"], "pinout": names[x["pins"]], "footprint": x["footprint"]}
                        for x in dev["parts"]]
    new_sheets = {i: u for i, u in sheets.values() if i not in existing_sheets}
    return text, start, end, devices, new_sheets, sorted(used_fps - have)


def q(s):
    return json.dumps(s, ensure_ascii=False)


def render(devices):
    out = []
    for did in sorted(devices):
        d = devices[did]
        pins, contacts = CIRCUITS[d["circuit"]]
        states, rest = states_of(d["circuit"], d["action"])
        note = f"Rating {num(d['v'])} V: DigiKey Voltage Rating - {' and '.join(sorted(d['kinds']))}"
        lines = [
            "    {",
            f'      "id": {q(did)},',
            f'      "function": {q(d["circuit"] + " " + d["action"])},',
            f'      "description": {q(d["circuit"] + " slide switch, " + d["action"])},',
            '      "drawer": ["Switches & Relays","Slide"],',
            f'      "note": {q(note)},',
            '      "pinouts": {',
        ]
        ps = sorted(d["pinouts"].values())
        for i, (pn, pl) in enumerate(ps):
            lines.append(f"        {q(pn)}: [")
            for j, (number, name) in enumerate(pl):
                comma = "," if j < len(pl) - 1 else ""
                lines.append(f'          {{"number":{number},"name":{q(name)},"role":"inout"}}{comma}')
            lines.append("        ]" + ("," if i < len(ps) - 1 else ""))
        lines.append("      },")
        lines.append(f'      "actuator": {q(d["act"])},')
        lines.append('      "contacts": [')
        for j, c in enumerate(contacts):
            lines.append("        " + json.dumps(c, separators=(",", ":")) + ("," if j < len(contacts) - 1 else ""))
        lines.append("      ],")
        lines.append(f'      "states": {json.dumps(states, separators=(",", ":"))},')
        lines.append(f'      "rest": {rest},')
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
    text, start, end, devices, new_sheets, move = build(p, makers)
    blocks = render(devices)
    head, tail = text[:start], text[end:]
    if blocks:
        new = head + "\n" + ",\n".join(blocks) + "," + tail
    else:
        new = head + tail
    return text, new, devices, new_sheets, move


def sheets_text(p, new_sheets):
    """(old, new) text of datasheets.json. Each new entry goes in front of the first existing
    entry whose id is greater (else after the last): the file is nearly, not exactly,
    sorted (other plans append blocks), so the existing lines are never reordered."""
    text = open(p["datasheets"]).read()
    if not new_sheets:
        return text, text
    lines = text.split("\n")
    at = [i for i, l in enumerate(lines) if l.startswith('    {"id"')]
    first, last = at[0], at[-1]
    entries = [l.rstrip(",") for l in lines[first:last + 1]]
    key = lambda l: re.search(r'"id": "([^"]*)"', l).group(1)
    for i, u in sorted(new_sheets.items()):
        line = "    " + json.dumps({"id": i, "url": u}, ensure_ascii=False)
        pos = next((n for n, e in enumerate(entries) if key(e) > i), len(entries))
        entries.insert(pos, line)
    return text, "\n".join(lines[:first] + [",\n".join(entries)] + lines[last + 1:])


def footprints_text(p, move):
    """(old, new) text of footprints.json with the staged footprints in `move` (ids,
    in id order) appended, one line each."""
    text = open(p["footprints"]).read()
    if not move:
        return text, text
    staged = staged_footprints(p)
    end = text.rstrip().rfind("]")
    head = text[:end].rstrip()
    lines = ",\n".join("    " + json.dumps(staged[i][0], ensure_ascii=False, separators=(",", ":")) for i in move)
    return text, head + ",\n" + lines + "\n  ]\n}\n"


def unstage(p, move):
    """Removes the moved footprints from their staged files (a file left empty is deleted)."""
    staged = staged_footprints(p)
    by_file = {}
    for i in move:
        by_file.setdefault(staged[i][1], set()).add(i)
    for f, ids in by_file.items():
        doc = json.load(open(f))
        doc["footprints"] = [o for o in doc["footprints"] if o["id"] not in ids]
        if not doc["footprints"]:
            os.remove(f)
            continue
        with open(f, "w") as out:
            out.write('{\n  "format": "schematical-footprints",\n  "version": 1,\n  "footprints": [\n')
            out.write(",\n".join("    " + json.dumps(o, ensure_ascii=False, separators=(",", ":")) for o in doc["footprints"]))
            out.write("\n  ]\n}\n")


def run(p, makers, dry):
    old, new, devices, new_sheets, move = generate(p, makers)
    old_ds, new_ds = sheets_text(p, new_sheets)
    old_fp, new_fp = footprints_text(p, move)
    if old == new and old_ds == new_ds and old_fp == new_fp:
        print("no changes")
        return 0
    if dry:
        for a, b, name in ((old, new, "switch.json"), (old_ds, new_ds, "datasheets.json"), (old_fp, new_fp, "footprints.json")):
            sys.stdout.writelines(difflib.unified_diff(a.splitlines(True), b.splitlines(True), name,
                                                       name + " (new)", n=0 if name != "switch.json" else 1))
        return 0
    open(p["switch"], "w").write(new)
    open(p["datasheets"], "w").write(new_ds)
    open(p["footprints"], "w").write(new_fp)
    unstage(p, move)
    n = sum(len(d["parts"]) for d in devices.values())
    print(f"{len(devices)} slide devices, {n} slide parts written; {len(new_sheets)} datasheets.json entries and "
          f"{len(move)} footprints added")
    return 0


def selftest():
    tmp = tempfile.mkdtemp(prefix="p58-gen-")
    p = paths(tmp)
    os.makedirs(p["parts"])
    os.makedirs(p["data"])
    os.makedirs(os.path.dirname(p["places"]))
    real = paths(ROOT)
    shutil.copyfile(real["switch"], p["switch"])
    shutil.copyfile(real["datasheets"], p["datasheets"])
    with open(p["places"], "w") as f:
        f.write("file\turl\tbytes\tsha256\n"
                "datasheets/switch/slide/test/SELFTEST-SHEET.pdf\thttps://example.com/selftest.pdf\t1\t0\n")
    pad = {"x": 0, "y": 0, "width": 1, "height": 1, "shape": "round", "drill": 0.5}

    def fp(i, n):
        return {"id": i, "description": "t", "style": "explicit", "pins": n, "body": {"width": 5, "height": 5},
                "pads": [dict(pad, pin=k + 1, x=k * 2.0) for k in range(n)]}
    open(p["footprints"], "w").write('{\n  "format": "schematical-footprints",\n  "version": 1,\n  "footprints": [\n'
                                     + json.dumps(fp("LIB-3", 3), separators=(",", ":")).join(["    ", ""]) + "\n  ]\n}\n")
    with open(os.path.join(p["data"], "footprints-X.json"), "w") as f:
        json.dump({"format": "schematical-footprints", "version": 1, "footprints": [fp("SW-T3", 3), fp("SW-T6", 6), fp("SW-UNUSED", 2)]}, f)
    cols = ["maker", "mpn", "series", "circuit", "function", "timing", "current", "vac", "vdc", "actuator", "mounting",
            "termination", "features", "stock", "packaging", "datasheet", "detail", "decision", "reason", "pdf", "hit", "dead"]

    def row(mpn, circuit, function, mounting, vac="", vdc=""):
        r = dict.fromkeys(cols, "")
        r.update(maker="Test Maker", mpn=mpn, series="S1", circuit=circuit, function=function, mounting=mounting,
                 actuator="Standard", vac=vac, vdc=vdc, decision="add", pdf="1", hit="1", dead="0",
                 datasheet="https://example.com/selftest.pdf")
        return "\t".join(r[c] for c in cols)
    rows = [row("SELF-SPDT-1", "SPDT", "On-On", "Through Hole", "125 V", "28 V"),
            row("SELF-SPDT-2", "SPDT", "On-On", "Through Hole", "125 V", "28 V"),
            row("SELF-DPDT-3", "DPDT", "On-Off-On", "Through Hole, Right Angle", "30 V", "-")]
    with open(p["candidates"], "w") as f:
        f.write("\t".join(cols) + "\n" + "\n".join(rows) + "\n")
    scols = "maker\tseries\tmounting\tcircuit\tmpn_regex\tterminals\tpins\tfootprint\tnote\n"
    with open(p["specs"], "w") as f:
        f.write(scols + "Test Maker\tS1\tThrough Hole\tSPDT\tSELF-SPDT-[12]\t\t2=C;1=A;3=B\tSW-T3\tn\n"
                "Test Maker\tS1\tThrough Hole, Right Angle\tDPDT\tSELF-DPDT-3\t\t2=C1;1=A1;3=B1;5=C2;4=A2;6=B2\tSW-T6\tn\n")
    original = open(p["switch"]).read()
    original_ds = open(p["datasheets"]).read()
    prefix_len, _ = region_bounds(original)
    ok = True

    def check(cond, what):
        nonlocal ok
        print(("ok   " if cond else "FAIL ") + what)
        ok = ok and cond
    run(p, ["Test Maker"], False)
    once, once_ds, once_fp = (open(p[k]).read() for k in ("switch", "datasheets", "footprints"))
    run(p, [], False)
    check(once == open(p["switch"]).read() and once_ds == open(p["datasheets"]).read()
          and once_fp == open(p["footprints"]).read() and once != original, "a second run changes nothing; the first wrote")
    doc = json.loads(once)
    devs = [d for d in doc["devices"] if d["id"].startswith("slide_")]
    ids = [d["id"] for d in doc["devices"]]
    check(sorted(d["id"] for d in devs) == ["slide_dpdt_onoffon_30v_standard", "slide_spdt_onon_28v_standard"],
          "one device per (circuit, action, volts, actuator); DC rating before AC")
    check(once[:prefix_len] == original[:prefix_len] and ids.index(devs[-1]["id"]) + 1 == ids.index(
        next(i for i in ids if i.startswith("tact_"))), "text before the region is unchanged; slide_ devices sit just before tact_")
    spdt = next(d for d in devs if d["id"].startswith("slide_spdt"))
    check(spdt["contacts"] == [["C", "A"], ["C", "B"]] and spdt["states"] == [[1], [2]] and spdt["rest"] == 1
          and [x["name"] for x in spdt["pinouts"]["p3_213"]] == ["A", "C", "B"] and [x["number"] for x in spdt["pinouts"]["p3_213"]] == [1, 2, 3],
          "contacts, states, rest and pin names of an SPDT On-On")
    dpdt = next(d for d in devs if d["id"].startswith("slide_dpdt"))
    check(dpdt["states"] == [[1, 3], [], [2, 4]] and dpdt["rest"] == 2 and len(dpdt["pinouts"]["p6_213546"]) == 6,
          "an On-Off-On DPDT starts in its middle position")
    check(spdt["parts"][0]["package"] == "THT" and dpdt["parts"][0]["package"] == "THT right angle"
          and spdt["parts"][0]["manufacturer"] == "Test Maker" and spdt["parts"][0]["datasheet"] == "selftest-sheet"
          and spdt["parts"][0]["footprint"] == "SW-T3", "part keys")
    new_ids = [o["id"] for o in json.loads(once_fp)["footprints"]]
    left = [o["id"] for o in json.load(open(os.path.join(p["data"], "footprints-X.json")))["footprints"]]
    check(new_ids == ["LIB-3", "SW-T3", "SW-T6"] and left == ["SW-UNUSED"], "used footprints move to the library, the unused one stays staged")
    dsids = [e["id"] for e in json.loads(once_ds)["datasheets"]]
    old_ids = [e["id"] for e in json.loads(original_ds)["datasheets"]]
    check([i for i in dsids if i != "selftest-sheet"] == old_ids and "selftest-sheet" in dsids,
          "one datasheets.json entry added; the existing entries keep their order")
    # refusals
    def refused(what, mutate):
        shutil.copyfile(real["switch"], p["switch"])
        shutil.copyfile(real["datasheets"], p["datasheets"])
        with open(p["candidates"], "w") as f:
            f.write("\t".join(cols) + "\n" + "\n".join(rows) + "\n")
        with open(p["specs"], "w") as f:
            f.write(scols + "Test Maker\tS1\tThrough Hole\tSPDT\tSELF-SPDT-[12]\t\t2=C;1=A;3=B\tSW-T3\tn\n"
                    "Test Maker\tS1\tThrough Hole, Right Angle\tDPDT\tSELF-DPDT-3\t\t2=C1;1=A1;3=B1;5=C2;4=A2;6=B2\tSW-T6\tn\n")
        mutate()
        try:
            build(p, ["Test Maker"])
        except Refuse:
            check(True, "refused: " + what)
        else:
            check(False, "refused: " + what)

    def edit(path, old, new):
        def go():
            t = open(path).read()
            assert old in t, old
            open(path, "w").write(t.replace(old, new))
        return go
    refused("a part name already in the library", edit(p["candidates"], "SELF-SPDT-1", "SN74HC00N"))
    refused("no spec matches", edit(p["specs"], "SELF-SPDT-[12]", "NOPE"))
    def duplicate():
        with open(p["specs"], "a") as f:
            f.write("Test Maker\tS1\tThrough Hole\tSPDT\tSELF-SPDT-1\t\t2=C;1=A;3=B\tSW-T3\tn\n")
    refused("two specs match", duplicate)
    refused("a spec for another circuit", edit(p["specs"], "Through Hole\tSPDT", "Through Hole\tDPDT"))
    refused("an unknown footprint", edit(p["specs"], "SW-T3", "SW-NOPE"))
    refused("no rating", edit(p["candidates"], "125 V\t28 V", "-\t-"))
    refused("an unknown action", edit(p["candidates"], "On-Off-On", "Mom-On"))
    shutil.rmtree(tmp)
    return 0 if ok else 1


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--maker", action="append", default=[])
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--selftest", action="store_true")
    a = ap.parse_args()
    try:
        if a.selftest:
            return selftest()
        return run(paths(ROOT), a.maker, a.dry_run)
    except Refuse as e:
        print("refused:", e)
        return 1


if __name__ == "__main__":
    sys.exit(main())
