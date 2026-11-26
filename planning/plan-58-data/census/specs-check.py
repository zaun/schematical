"""plan-58-C Phase 2: every kept row of a maker has a spec, and the spec's footprint has its pads.

usage: python3 -I specs-check.py --maker 'C&K'
       python3 -I specs-check.py --maker 'C&K' --specs <fragment.tsv>   check one fragment: only the
           rows of the series the fragment names, against the fragment's rows
       python3 -I specs-check.py --selftest

Reads candidates.tsv (rows of the maker with decision `add`), specs.tsv (rows of
the maker) and the footprints: every planning/plan-58-data/footprints-*.json
(staged, not yet in the library) and schematical/schematical/Parts/footprints.json.
Checks (each failure is printed, exit 1):
  - every `add` row's part number fullmatches exactly one spec's `mpn_regex`;
  - that spec's maker / series / mounting / circuit are the row's;
  - `pins` is `designator=name;...`: the names are exactly those of the circuit
    (plan-58-B section 4.3: SPST A B; SPDT C A B; DPST A1 B1 A2 B2; DPDT C1 A1 B1
    C2 A2 B2), the designators are 1..n, each used once;
  - `footprint` exists, and its numbered pads (pin > 0) are exactly the designators;
  - every spec row of the maker matches at least one `add` row (no stale spec);
  - no spec row of the maker is left with an empty mpn_regex / pins / footprint / note.
Prints `specs ok` when all hold.
"""
import csv
import glob
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.abspath(os.path.join(DATA, "..", ".."))
CAND = os.path.join(DATA, "candidates.tsv")
SPECS = os.path.join(DATA, "specs.tsv")
LIBRARY = os.path.join(ROOT, "schematical", "schematical", "Parts", "footprints.json")

NAMES = {"SPST": ["A", "B"], "SPDT": ["C", "A", "B"], "DPST": ["A1", "B1", "A2", "B2"],
         "DPDT": ["C1", "A1", "B1", "C2", "A2", "B2"]}


def tsv(path):
    return list(csv.DictReader(open(path), delimiter="\t"))


def footprints():
    out = {}
    for path in sorted(glob.glob(os.path.join(DATA, "footprints-*.json"))) + [LIBRARY]:
        for fp in json.load(open(path))["footprints"]:
            out.setdefault(fp["id"], fp)
    return out


def numbered_pads(fp):
    """The pin numbers a footprint's pads carry (pin 0 pads are unnumbered)."""
    if fp["style"] == "explicit":
        return {int(p["pin"]) for p in fp["pads"] if str(p.get("pin", 0)) != "0" and "pin" in p}
    return set(range(1, int(fp["pins"]) + 1))


def parse_pins(text):
    pins = {}
    for item in text.split(";"):
        d, _, name = item.partition("=")
        if not d.strip().isdigit() or not name.strip() or int(d) in pins:
            return None
        pins[int(d)] = name.strip()
    return pins


def check(adds, specs, fps):
    problems = []
    used = set()
    for r in adds:
        hits = [i for i, s in enumerate(specs) if s["mpn_regex"] and re.fullmatch(s["mpn_regex"], r["mpn"])]
        if len(hits) != 1:
            problems.append(f"{r['mpn']}: {len(hits)} specs match (need exactly 1)")
            continue
        used.add(hits[0])
        s = specs[hits[0]]
        for k in ("maker", "series", "mounting", "circuit"):
            if s[k] != r[k]:
                problems.append(f"{r['mpn']}: spec {k} {s[k]!r} differs from the row's {r[k]!r}")
    for i, s in enumerate(specs):
        label = f"spec {s['series']} / {s['mounting']} / {s['circuit']} / {s['mpn_regex']}"
        if not (s["mpn_regex"] and s["pins"] and s["footprint"] and s["note"]):
            problems.append(f"{label}: mpn_regex, pins, footprint or note is empty")
            continue
        if i not in used:
            problems.append(f"{label}: matches no add row")
        pins = parse_pins(s["pins"])
        want = NAMES.get(s["circuit"].split()[0])
        if pins is None or want is None:
            problems.append(f"{label}: pins {s['pins']!r} is not designator=name;...")
            continue
        if sorted(pins.values()) != sorted(want) or sorted(pins) != list(range(1, len(want) + 1)):
            problems.append(f"{label}: pins {s['pins']!r} are not {want} on 1..{len(want)}")
        fp = fps.get(s["footprint"])
        if fp is None:
            problems.append(f"{label}: unknown footprint {s['footprint']}")
        elif numbered_pads(fp) != set(pins):
            problems.append(f"{label}: footprint {s['footprint']} has numbered pads {sorted(numbered_pads(fp))}, pins are {sorted(pins)}")
    return problems


def main(maker, specs_path=SPECS):
    specs = [s for s in tsv(specs_path) if s["maker"] == maker]
    series = {s["series"] for s in specs} if specs_path != SPECS else None
    adds = [r for r in tsv(CAND) if r["maker"] == maker and r["decision"] == "add"
            and (series is None or r["series"] in series)]
    problems = check(adds, specs, footprints())
    for p in problems:
        print(p)
    if problems:
        print(f"{len(problems)} problems")
        return 1
    print(f"specs ok ({len(adds)} rows, {len(specs)} specs)")
    return 0


def selftest():
    fps = {"F3": {"style": "explicit", "pins": 3, "pads": [{"pin": 1}, {"pin": 2}, {"pin": 3}, {"pin": 0}]}}
    row = {"mpn": "AB1", "maker": "M", "series": "S", "mounting": "TH", "circuit": "SPDT"}
    spec = {"maker": "M", "series": "S", "mounting": "TH", "circuit": "SPDT", "mpn_regex": "AB[12]",
            "pins": "2=C;1=A;3=B", "footprint": "F3", "note": "n"}
    assert check([row], [spec], fps) == []
    assert check([dict(row, mpn="AB3")], [spec], fps)                       # no spec matches
    assert check([row], [dict(spec, pins="1=C;2=A")], fps)                   # wrong names
    assert check([row], [dict(spec, footprint="F9")], fps)                   # unknown footprint
    assert check([row], [dict(spec, mpn_regex="AB.", pins="1=C;2=A;4=B")], fps)   # designators not 1..n
    assert check([row, row], [spec, dict(spec, mpn_regex="AB1")], fps)       # two specs match
    assert check([row], [spec, dict(spec, mpn_regex="ZZ")], fps)             # a stale spec
    assert check([row], [dict(spec, mounting="SMD")], fps)                   # mounting differs
    print("ok")
    return 0


if __name__ == "__main__":
    a = sys.argv[1:]
    if a == ["--selftest"]:
        sys.exit(selftest())
    if len(a) == 2 and a[0] == "--maker":
        sys.exit(main(a[1]))
    if len(a) == 4 and a[0] == "--maker" and a[2] == "--specs":
        sys.exit(main(a[1], a[3]))
    print(__doc__)
    sys.exit(2)
